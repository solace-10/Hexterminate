// Copyright 2016 Pedro Nunes
//
// This file is part of Hexterminate.
//
// Hexterminate is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Hexterminate is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hexterminate. If not, see <http://www.gnu.org/licenses/>.

#include <rendersystem.h>
#include <shader.h>
#include <shadercache.h>
#include <shaderuniform.h>

#include "sprite/spritemanager.h"

#include "hexterminate.h"

namespace Hexterminate
{

SpriteManager::SpriteManager()
    : m_pTexture( nullptr )
    , m_pShader( nullptr )
    , m_pVertexBuffer( nullptr )
{
    using namespace Genesis;

    m_Sprites.reserve( 512 );

    m_pTexture = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/images/sprites.png" );

    m_pShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "sprite" );
    Genesis::ShaderUniform* pSampler = m_pShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    pSampler->Set( m_pTexture, GL_TEXTURE0 );

    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );
};

SpriteManager::~SpriteManager()
{
    delete m_pVertexBuffer;
}

void SpriteManager::Update( float delta )
{
    using namespace Genesis;

    if ( m_Sprites.empty() )
    {
        return;
    }

    PositionData posData;
    UVData uvData;
    ColourData colourData;

    size_t numVertices = m_Sprites.size() * 6;
    posData.reserve( numVertices );
    uvData.reserve( numVertices );
    colourData.reserve( numVertices );

    for ( auto& sprite : m_Sprites )
    {
        const float halfWidth = sprite.GetWidth() / 2.0f;
        const glm::vec3& src = sprite.GetSource();
        const glm::vec3& dst = sprite.GetDestination();
        glm::vec3 dir = glm::normalize( dst - src );
        glm::vec3 perp( -dir.y * halfWidth, dir.x * halfWidth, 0.0f );

        posData.emplace_back( src.x + perp.x, src.y + perp.y, src.z ); // 0
        posData.emplace_back( src.x - perp.x, src.y - perp.y, src.z ); // 1
        posData.emplace_back( dst.x - perp.x, dst.y - perp.y, src.z ); // 2
        posData.emplace_back( src.x + perp.x, src.y + perp.y, src.z ); // 0
        posData.emplace_back( dst.x - perp.x, dst.y - perp.y, src.z ); // 2
        posData.emplace_back( dst.x + perp.x, dst.y + perp.y, src.z ); // 3

        uvData.emplace_back( 1.0f, 0.0f ); // 0
        uvData.emplace_back( 0.0f, 0.0f ); // 1
        uvData.emplace_back( 0.0f, 1.0f ); // 2
        uvData.emplace_back( 1.0f, 0.0f ); // 0
        uvData.emplace_back( 0.0f, 1.0f ); // 2
        uvData.emplace_back( 1.0f, 1.0f ); // 3

        const glm::vec4 colour = sprite.GetColour().glm();
        for ( int i = 0; i < 6; ++i )
        {
            colourData.push_back( colour );
        }
    }

    m_pVertexBuffer->CopyPositions( posData );
    m_pVertexBuffer->CopyUVs( uvData );
    m_pVertexBuffer->CopyColours( colourData );
}

void SpriteManager::Render()
{
    using namespace Genesis;

    if ( m_Sprites.empty() )
        return;

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Add );

    m_pShader->Use();
    m_pVertexBuffer->Draw( static_cast<uint32_t>( m_Sprites.size() * 6 ) );

    if ( g_pGame->IsPaused() == false )
    {
        m_Sprites.clear();
    }

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Disabled );
}

void SpriteManager::AddSprite( const Sprite& Sprite )
{
    m_Sprites.push_back( Sprite );
}

} // namespace Hexterminate