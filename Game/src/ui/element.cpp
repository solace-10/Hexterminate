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
#include <sstream>

#include <configuration.h>
#include <genesis.h>
#include <gui/gui.h>
#include <imgui/imgui.h>
#include <logger.h>

#include "hexterminate.h"
#include "menus/popup.h"
#include "ui/design.h"
#include "ui/element.h"
#include "ui/rootelement.h"

namespace Hexterminate::UI
{

static const char* sElementPropertyPosition = "position";
static const char* sElementPropertySize = "size";
static const char* sElementPropertyAnchor = "anchor";

Element::Element( const std::string& name )
    : m_Name( name )
    , m_IsPopupElement( false )
    , m_IsEditable( true )
    , m_IsDynamic( false )
    , m_AnchorTop( true )
    , m_AnchorLeft( true )
    , m_AnchorBottom( false )
    , m_AnchorRight( false )
    , m_PaddingRight( false )
    , m_PaddingBottom( false )
{
    m_pPanel = new Genesis::Gui::Panel();
    m_pPanel->SetSize( 128.0f, 128.0f );
    m_pPanel->SetPosition( 0.0f, 0.0f );
    m_pPanel->SetColour( 0.0f, 0.0f, 0.0f, 0.0f );
    m_pPanel->SetBorderColour( 0.0f, 0.0f, 0.0f, 0.0f );
}

Element::~Element()
{
}

void Element::Update()
{
    for ( auto& pChild : m_Children )
    {
        pChild->Update();
    }
}

void Element::Add( ElementSharedPtr pElement )
{
#ifdef _DEBUG
    for ( auto& pChild : m_Children )
    {
        if ( pChild->GetName() == pElement->GetName() )
        {
            Genesis::FrameWork::GetLogger()->LogWarning( "UI Element '%s' already contains a child called '%s'.", pElement->GetName().c_str() );
            return;
        }
    }
#endif

    m_Children.push_back( pElement );
    m_pPanel->AddElement( pElement->GetPanel() );
    pElement->ResolvePath( this );
    pElement->LoadFromDesign();
}

bool Element::IsHovered() const
{
    return GetPanel()->IsMouseInside();
}

void Element::Show( bool state )
{
    GetPanel()->Show( state );
}

void Element::LoadFromDesign()
{
    if ( IsPathResolved() == false || IsDynamic() )
    {
        return;
    }

    Design* pDesign = g_pGame->GetUIRoot()->GetDesign();
    const json& data = pDesign->Get( GetPath() );
    if ( data.is_object() )
    {
        if ( data.contains( "properties" ) )
        {
            const json& properties = data[ "properties" ];
            if ( properties.is_object() )
            {
                LoadProperties( properties );
            }
        }

        for ( auto& pChildElement : m_Children )
        {
            pChildElement->LoadFromDesign();
        }
    }
}

void Element::ResolvePath( Element* pParentElement )
{
    std::stringstream path;

    if ( pParentElement == g_pGame->GetUIRoot() )
    {
        path << "/" << GetName();
    }
    else if ( pParentElement->IsPathResolved() )
    {
        path << pParentElement->GetPath() << "/" << GetName();
    }
    else
    {
        return;
    }

    bool pathChanged = ( m_Path != path.str() );
    if ( pathChanged )
    {
        m_Path = path.str();

        for ( auto& pChildElement : m_Children )
        {
            pChildElement->ResolvePath( this );
        }
    }
}

void Element::SaveInternal( json& data, bool saveProperties /* = true */ )
{
    if ( IsDynamic() )
    {
        return;
    }

    if ( saveProperties )
    {
        SaveProperties( data[ "properties" ] );
    }

    if ( GetChildren().size() > 0 )
    {
        for ( auto& pChild : GetChildren() )
        {
            json& childData = data[ pChild->GetName() ];
            pChild->SaveInternal( childData );
        }
    }
}

void Element::SaveProperties( json& properties )
{
    glm::vec2 pos = m_pPanel->GetPosition();
    properties[ sElementPropertyPosition ] = {
        { "x", pos.x },
        { "y", pos.y }
    };

    glm::vec2 size = m_pPanel->GetSize();
    properties[ sElementPropertySize ] = {
        { "w", size.x },
        { "h", size.y }
    };

    properties[ sElementPropertyAnchor ] = {
        { "t", m_AnchorTop },
        { "l", m_AnchorLeft },
        { "b", m_AnchorBottom },
        { "r", m_AnchorRight }
    };
}

void Element::LoadProperties( const json& properties )
{
    const json& position = properties[ sElementPropertyPosition ];
    SetPosition( position[ "x" ].get<int>(), position[ "y" ].get<int>() );

    const json& anchor = properties[ sElementPropertyAnchor ];
    m_AnchorTop = anchor[ "t" ].get<bool>();
    m_AnchorLeft = anchor[ "l" ].get<bool>();
    m_AnchorBottom = anchor[ "b" ].get<bool>();
    m_AnchorRight = anchor[ "r" ].get<bool>();

    const json& size = properties[ sElementPropertySize ];
    const int w = size[ "w" ].get<int>();
    const int h = size[ "h" ].get<int>();
    SetSize( w, h );
}

void Element::InputIntExt( const char* label, int* v, bool isReadOnly /* = false */ )
{
    if ( isReadOnly )
    {
        ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.2f, 0.2f, 0.2f, 0.5f ) );
        ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.2f, 0.2f, 0.2f, 0.5f ) );
        ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.7f, 0.7f, 0.7f, 1.0f ) );
        ImGui::InputInt( label, v, 1, 100, ImGuiInputTextFlags_ReadOnly );
        ImGui::PopStyleColor( 3 );
    }
    else
    {
        ImGui::InputInt( label, v );
    }
}

void Element::RenderProperties()
{
    if ( ImGui::CollapsingHeader( "Element", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        bool visible = m_pPanel->IsVisible();
        if ( ImGui::Checkbox( "Visible", &visible ) )
        {
            m_pPanel->Show( visible );
        }

        ImGui::Text( "%s", "Anchor" );
        ImGui::PushStyleVar( ImGuiStyleVar_SelectableTextAlign, ImVec2( 0.5f, 0.5f ) );
        ImVec2 s( 25, 25 );
        ImGui::InvisibleButton( "TL", s );
        ImGui::SameLine();
        if ( ImGui::Selectable( "T", m_AnchorTop, 0, s ) )
        {
            m_AnchorTop = !m_AnchorTop;
        }

        if ( ImGui::Selectable( "L", m_AnchorLeft, 0, s ) )
        {
            m_AnchorLeft = !m_AnchorLeft;
        }
        ImGui::SameLine();
        ImGui::InvisibleButton( "C", s );
        ImGui::SameLine();
        if ( ImGui::Selectable( "R", m_AnchorRight, 0, s ) )
        {
            m_AnchorRight = !m_AnchorRight;
        }

        ImGui::InvisibleButton( "BL", s );
        ImGui::SameLine();
        if ( ImGui::Selectable( "B", m_AnchorBottom, 0, s ) )
        {
            m_AnchorBottom = !m_AnchorBottom;
        }
        ImGui::PopStyleVar();

        glm::vec2 gpos = m_pPanel->GetPosition();
        int ipos[ 2 ] = { static_cast<int>( gpos.x ), static_cast<int>( gpos.y ) };
        InputIntExt( "X", &ipos[ 0 ], !m_AnchorLeft );
        InputIntExt( "Y", &ipos[ 1 ], !m_AnchorTop );
        SetPosition( ipos[ 0 ], ipos[ 1 ] );

        InputIntExt( "Padding right", &m_PaddingRight, !m_AnchorRight );
        InputIntExt( "Padding bottom", &m_PaddingBottom, !m_AnchorBottom );

        glm::vec2 gsize = m_pPanel->GetSize();
        int isize[ 2 ] = { static_cast<int>( gsize.x ), static_cast<int>( gsize.y ) };
        const bool isWidthReadOnly = !IsResizeable() || ( m_AnchorLeft && m_AnchorRight );
        InputIntExt( "Width", &isize[ 0 ], isWidthReadOnly );
        const bool isHeightReadOnly = !IsResizeable() || ( m_AnchorTop && m_AnchorBottom );
        InputIntExt( "Height", &isize[ 1 ], isHeightReadOnly );

        SetSize( isize[ 0 ], isize[ 1 ] );
    }
}

void Element::SetSize( int width, int height )
{
    const glm::vec2& pos = m_pPanel->GetPosition();

    if ( m_AnchorLeft && m_AnchorRight )
    {
        width = static_cast<int>( Genesis::Configuration::GetScreenWidth() ) - static_cast<int>( pos.x ) - m_PaddingRight;
    }

    if ( m_AnchorTop && m_AnchorBottom )
    {
        height = static_cast<int>( Genesis::Configuration::GetScreenHeight() ) - static_cast<int>( pos.y ) - m_PaddingBottom;
    }

    m_pPanel->SetSize( width, height );
}

void Element::SetPosition( int x, int y )
{
    m_pPanel->SetPosition( x, y );
}

int Element::GetWidth() const
{
    return m_pPanel->GetWidth();
}

int Element::GetHeight() const
{
    return m_pPanel->GetHeight();
}

void Element::GetPosition( int& x, int& y )
{
    const glm::vec2 pos = m_pPanel->GetPosition();
    x = static_cast<int>( pos.x );
    y = static_cast<int>( pos.y );
}

bool Element::IsAcceptingInput() const
{
    if ( IsPopupElement() )
    {
        return true;
    }
    else
    {
        return g_pGame->GetPopup()->IsActive() == false && GetPanel()->IsVisible();
    }
}

bool Element::IsVisible() const
{
    return m_pPanel->IsVisible();
}

} // namespace Hexterminate::UI
