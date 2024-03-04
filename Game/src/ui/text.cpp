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

#include <filesystem>

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <gui/gui.h>

#include "ui/fonts.h"
#include "ui/text.h"

namespace Hexterminate::UI
{

static const char* sTextPropertyMultiline = "multiline";
static const char* sTextPropertyLabel = "label";
static const char* sTextPropertyFont = "font";
static const char* sTextPropertyLineSpacing = "line_spacing";
static const char* sTextPropertyColor = "color";
static const char* sTextPropertyAlignment = "alignment";

Text::Text( const std::string& name )
    : Element( name )
    , m_Multiline( false )
    , m_pText( nullptr )
    , m_Label( "???" )
    , m_Color( { 1.0f, 1.0f, 1.0f, 1.0f } )
    , m_Alignment( Alignment::Left )
{
    m_pText = new Genesis::Gui::Text();
    m_pText->SetSize( 256.0f, 16.0f );
    m_pText->SetColor( 1.0f, 1.0f, 1.0f, 1.0f );
    m_pText->SetMultiLine( false );
    m_pText->SetFont( Fonts::Get( "kimberley18light.fnt" ) );
    m_pText->SetLineSpacing( 1.4f );
    m_pText->SetText( m_Label );
    GetPanel()->AddElement( m_pText );
}

Text::~Text()
{
}

void Text::SaveProperties( json& properties )
{
    Element::SaveProperties( properties );

    properties[ sTextPropertyMultiline ] = m_Multiline;
    properties[ sTextPropertyLabel ] = m_Label;
    properties[ sTextPropertyFont ] = m_pText->GetFont()->GetFilename().GetName();
    properties[ sTextPropertyLineSpacing ] = m_pText->GetLineSpacing();
    properties[ sTextPropertyColor ] = {
        { "r", m_Color[ 0 ] },
        { "g", m_Color[ 1 ] },
        { "b", m_Color[ 2 ] },
        { "a", m_Color[ 3 ] }
    };

    if ( m_Alignment == Alignment::Right )
    {
        properties[ sTextPropertyAlignment ] = "right";
    }
    else if ( m_Alignment == Alignment::Center )
    {
        properties[ sTextPropertyAlignment ] = "center";
    }
}

void Text::LoadProperties( const json& properties )
{
    Element::LoadProperties( properties );

    if ( properties.contains( sTextPropertyMultiline ) )
    {
        m_Multiline = properties[ sTextPropertyMultiline ].get<bool>();
        m_pText->SetMultiLine( m_Multiline );
    }

    if ( properties.contains( sTextPropertyLabel ) )
    {
        m_Label = properties[ sTextPropertyLabel ].get<std::string>();
        m_pText->SetText( m_Label );
    }

    if ( properties.contains( sTextPropertyFont ) )
    {
        std::string fontName = properties[ sTextPropertyFont ].get<std::string>();
        Genesis::ResourceFont* pFont = Fonts::Get( fontName );
        if ( pFont == nullptr )
        {
            Genesis::FrameWork::GetLogger()->LogWarning( "Couldn't find font '%s' for UI element '%s'.", fontName.c_str(), GetName().c_str() );
        }
        else
        {
            m_pText->SetFont( pFont );
        }
    }

    if ( properties.contains( sTextPropertyLineSpacing ) )
    {
        m_pText->SetLineSpacing( properties[ sTextPropertyLineSpacing ].get<float>() );
    }

    if ( properties.contains( sTextPropertyColor ) )
    {
        const json& color = properties[ sTextPropertyColor ];
        m_Color[ 0 ] = color[ "r" ].get<float>();
        m_Color[ 1 ] = color[ "g" ].get<float>();
        m_Color[ 2 ] = color[ "b" ].get<float>();
        m_Color[ 3 ] = color[ "a" ].get<float>();
        m_pText->SetColor( m_Color[ 0 ], m_Color[ 1 ], m_Color[ 2 ], m_Color[ 3 ] );
    }

    if ( properties.contains( sTextPropertyAlignment ) )
    {
        const std::string alignment = properties[ sTextPropertyAlignment ].get<std::string>();
        if ( alignment == "right" )
        {
            SetAlignment( Alignment::Right );
        }
        else if ( alignment == "center" )
        {
            SetAlignment( Alignment::Center );
        }
        else
        {
            SetAlignment( Alignment::Left );
        }
    }
}

void Text::RenderProperties()
{
    Element::RenderProperties();

    if ( ImGui::CollapsingHeader( "Text", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        if ( ImGui::BeginListBox( "Font" ) )
        {
            for ( auto& pFont : Fonts::Get() )
            {
                bool isSelected = ( pFont == m_pText->GetFont() );
                if ( ImGui::Selectable( pFont->GetFilename().GetName().c_str(), &isSelected ) )
                {
                    m_pText->SetFont( pFont );
                }
            }
            ImGui::EndListBox();
        }

        if ( ImGui::Checkbox( "Multiline", &m_Multiline ) )
        {
            m_pText->SetMultiLine( m_Multiline );
        }

        bool textChanged = false;
        if ( m_Multiline )
        {
            textChanged = ImGui::InputTextMultiline( "Text", &m_Label, ImVec2( 0, ImGui::GetTextLineHeight() * 8 ) );
        }
        else
        {
            textChanged = ImGui::InputText( "Text", &m_Label );
        }

        if ( textChanged )
        {
            SetText( m_Label );
        }

        int alignment = static_cast<int>( m_Alignment );
        if ( ImGui::Combo( "Alignment", &alignment, "Left\0Right\0Center\0\0" ) )
        {
            SetAlignment( static_cast<Alignment>( alignment ) );
        }

        float lineSpacing = m_pText->GetLineSpacing();
        ImGui::InputFloat( "Line spacing", &lineSpacing );
        m_pText->SetLineSpacing( lineSpacing );

        if ( ImGui::ColorEdit4( "Color", m_Color.data() ) )
        {
            m_pText->SetColor( m_Color[ 0 ], m_Color[ 1 ], m_Color[ 2 ], m_Color[ 3 ] );
        }
    }
}

void Text::SetColor( const Genesis::Color& color )
{
    m_pText->SetColor( color );
    m_Color[ 0 ] = color.r;
    m_Color[ 1 ] = color.g;
    m_Color[ 2 ] = color.b;
    m_Color[ 3 ] = color.a;
}

void Text::SetColor( float r, float g, float b, float a )
{
    SetColor( Genesis::Color( r, g, b, a ) );
}

void Text::SetFont( const std::string& fontName )
{
    Genesis::ResourceFont* pFont = Fonts::Get( fontName );
    if ( pFont == nullptr )
    {
        Genesis::FrameWork::GetLogger()->LogWarning( "Couldn't find font '%s'.", fontName.c_str() );
    }
    else
    {
        m_pText->SetFont( pFont );
        RealignText();
    }
}

void Text::SetText( const std::string& text )
{
    m_pText->SetText( text );
    RealignText();
}

void Text::SetSize( int width, int height )
{
    Element::SetSize( width, height );
    m_pText->SetSize( width, height );
}

void Text::SetAlignment( Alignment alignment )
{
    m_Alignment = alignment;
    RealignText();
}

void Text::RealignText()
{
    if ( m_Alignment == Alignment::Left )
    {
        m_pText->SetPosition( 0, 0 );
    }
    else
    {
        const int textWidth = static_cast<int>( m_pText->GetFont()->GetTextLength( m_pText->GetText() ) );

        if ( m_Alignment == Alignment::Right )
        {
            m_pText->SetPosition( GetWidth() - textWidth, 0 );
        }
        else if ( m_Alignment == Alignment::Center )
        {
            m_pText->SetPosition( ( GetWidth() - textWidth ) / 2, 0 );
        }
    }
}

} // namespace Hexterminate::UI
