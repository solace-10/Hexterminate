// Copyright 2021 Pedro Nunes
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

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <genesis.h>
#include <gui/gui.h>
#include <logger.h>
#include <shadercache.h>

#include "ui/image.h"

namespace Hexterminate::UI
{

static const char* sImagePropertyFile = "file";
static const char* sImagePropertyAutoSize = "auto_size";

Image::Image( const std::string& name, const std::string& path /* = "" */ )
    : Element( name )
    , m_pImage( nullptr )
    , m_pResource( nullptr )
    , m_Path( path )
    , m_AutoSize( true )
{
    using namespace Genesis;

    GetPanel()->SetColour( 0.0f, 0.0f, 0.0f, 0.0f );
    GetPanel()->SetBorderColour( 0.0f, 0.0f, 0.0f, 0.0f );
    m_pImage = new Genesis::Gui::Image();
    GetPanel()->AddElement( m_pImage );

    LoadResources();
}

Image::~Image()
{
}

void Image::SetBlendMode( BlendMode blendMode )
{
    m_pImage->SetBlendMode( static_cast<Genesis::BlendMode>( blendMode ) );
}

void Image::SetColour( float r, float g, float b, float a )
{
    m_pImage->SetColour( r, g, b, a );
}

void Image::SetPath( const std::string& filename )
{
    m_Path = filename;
    LoadResources();
}

void Image::SetShader( const std::string& shaderName )
{
    m_pImage->SetShader( Genesis::FrameWork::GetRenderSystem()->GetShaderCache()->Load( shaderName ) );
}

void Image::LoadResources()
{
    using namespace Genesis;

    if ( m_Path.empty() == false )
    {
        m_pResource = static_cast<ResourceImage*>( FrameWork::GetResourceManager()->GetResource( m_Path ) );
        if ( m_pResource == nullptr )
        {
            FrameWork::GetLogger()->LogWarning( "Couldn't load '%s'.", m_Path.c_str() );
        }
        else
        {
            m_pImage->SetTexture( m_pResource );

            if ( m_AutoSize )
            {
                SetSize( static_cast<int>( m_pResource->GetWidth() ), static_cast<int>( m_pResource->GetHeight() ) );
            }
        }
    }
}

void Image::SaveProperties( json& properties )
{
    Element::SaveProperties( properties );
    properties[ sImagePropertyFile ] = m_Path;
}

void Image::LoadProperties( const json& properties )
{
    Element::LoadProperties( properties );

    if ( properties.contains( sImagePropertyAutoSize ) )
    {
        m_AutoSize = properties[ sImagePropertyAutoSize ].get<bool>();
    }

    if ( properties.contains( sImagePropertyFile ) )
    {
        m_Path = properties[ sImagePropertyFile ].get<std::string>();
        LoadResources();
    }
}

void Image::RenderProperties()
{
    Element::RenderProperties();

    if ( ImGui::CollapsingHeader( "Image", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        ImGui::Checkbox( "Auto size", &m_AutoSize );
        ImGui::InputText( "Image", &m_Path );
        if ( ImGui::Button( "Reload images" ) )
        {
            LoadResources();
        }
    }
}

void Image::SetSize( int width, int height )
{
    Element::SetSize( width, height );
    m_pImage->SetSize( width, height );
}

} // namespace Hexterminate::UI
