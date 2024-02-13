// Copyright 2014 Pedro Nunes
//
// This file is part of Genesis.
//
// Genesis is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Genesis is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Genesis. If not, see <http://www.gnu.org/licenses/>.

#include <SDL_image.h>

#include "resourceimage.h"
#include "../genesis.h"
#include "../logger.h"
#include "../rendersystem.h"

namespace Genesis
{

ResourceImage::ResourceImage( const Filename& filename )
    : ResourceGeneric( filename )
    , m_TextureSlot( 0u )
    , m_Width( 0u )
    , m_Height( 0u )
    , m_MipMapped( true )
    , m_pTemporarySurface( nullptr )
{
}

void ResourceImage::Preload()
{
    SDL_assert( m_pTemporarySurface == nullptr );
    m_pTemporarySurface = CreateSurface( GetFilename().GetFullPath() );
}

bool ResourceImage::Load()
{
    if ( m_pTemporarySurface == nullptr )
    {
        m_State = ResourceState::Unloaded;
        return false;
    }
    else
    {
        const std::string& extension = GetFilename().GetExtension();
        if ( extension == "jpg" )
        {
            CreateTexture( GL_RGB, GL_RGB );
        }
        else if ( extension == "png" )
        {
            GLenum format = ( m_pTemporarySurface->format->BitsPerPixel == 24 ) ? GL_RGB : GL_RGBA;
            CreateTexture( format, format );
        }
        else if ( extension == "tga" )
        {
            CreateTexture( GL_RGBA, GL_BGRA );
        }
        else if ( extension == "bmp" )
        {
            CreateTexture( GL_RGB, GL_BGR );
        }
        else
        {
            Genesis::FrameWork::GetLogger()->LogError( "Don't know how to create texture for extension '%s'", extension.c_str() );
            SDL_FreeSurface( m_pTemporarySurface );
            m_State = ResourceState::Unloaded;
            return false;
        }

        m_State = ResourceState::Loaded;
        return true;
    }
}

SDL_Surface* ResourceImage::CreateSurface( const std::string& filename )
{
    SDL_Surface* image = IMG_Load( filename.c_str() );
    if ( image == nullptr )
    {
		FrameWork::GetLogger()->LogWarning( "%s", IMG_GetError() );
    }

    return image;
}

void ResourceImage::CreateTexture( uint32_t internalFormat, uint32_t format )
{
    SDL_assert( m_pTemporarySurface != nullptr );

    GLuint texture;
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, m_pTemporarySurface->w, m_pTemporarySurface->h, 0, format, GL_UNSIGNED_BYTE, m_pTemporarySurface->pixels );
    glGenerateMipmap( GL_TEXTURE_2D );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    m_Width = m_pTemporarySurface->w;
    m_Height = m_pTemporarySurface->h;
    m_TextureSlot = texture;

    SDL_FreeSurface( m_pTemporarySurface );
}

void ResourceImage::EnableMipMapping( bool state )
{
    glBindTexture( GL_TEXTURE_2D, GetTexture() );

    if ( state )
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    }
    else
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    }
}
}