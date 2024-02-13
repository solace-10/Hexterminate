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

#include <configuration.h>
#include <gui/gui.h>
#include <imgui/imgui.h>

#include "hexterminate.h"
#include "ui/button.h"
#include "ui/panel.h"
#include "ui/text.h"
#include "ui/window.h"

namespace Hexterminate::UI
{

static const char* sWindowPropertyCenterH = "center_h";
static const char* sWindowPropertyCenterV = "center_v";
static const int sTitleHeight = 32;

Window::Window( const std::string& name, bool canBeClosed /* = false */ )
    : Element( name )
    , m_CenterHorizontally( false )
    , m_CenterVertically( false )
    , m_CanBeClosed( canBeClosed )
{
    using namespace Genesis;

    m_pTitlePanel = std::make_shared<Panel>( "Title" );
    m_pTitlePanel->SetBorders( false, false, true, false );
    m_pTitlePanel->SetBorderColour( 0.309f, 0.639f, 0.690f, 1.0f );
    m_pTitlePanel->SetSize( 128, sTitleHeight );
    Add( m_pTitlePanel );

    m_pTitleText = std::make_shared<Text>( "Title text" );
    m_pTitleText->SetFont( "kimberley22.fnt" );
    m_pTitleText->SetColour( 0.309f, 0.639f, 0.690f, 1.0f );
    m_pTitleText->SetPosition( 8, 6 );
    m_pTitlePanel->Add( m_pTitleText );

    if ( canBeClosed )
    {
        m_pCloseButton = std::make_shared<UI::Button>( "Close button",
            [ this ]( std::any userData ) {
                Show( false );
            } );
        m_pTitlePanel->Add( m_pCloseButton );
    }

    m_pContentPanel = std::make_shared<Panel>( "Content" );
    m_pContentPanel->SetBorders( false, false, true, true );
    Add( m_pContentPanel );
}

void Window::SaveProperties( json& properties )
{
    Element::SaveProperties( properties );

    properties[ sWindowPropertyCenterH ] = m_CenterHorizontally;
    properties[ sWindowPropertyCenterV ] = m_CenterVertically;
}

void Window::LoadProperties( const json& properties )
{
    Element::LoadProperties( properties );

    if ( properties.contains( sWindowPropertyCenterH ) )
    {
        m_CenterHorizontally = properties[ sWindowPropertyCenterH ].get<bool>();
        if ( m_CenterHorizontally )
        {
            CenterWindowHorizontally();
        }
    }

    if ( properties.contains( sWindowPropertyCenterV ) )
    {
        m_CenterVertically = properties[ sWindowPropertyCenterV ].get<bool>();
        if ( m_CenterVertically )
        {
            CenterWindowVertically();
        }
    }
}

void Window::RenderProperties()
{
    Element::RenderProperties();

    if ( ImGui::CollapsingHeader( "Window", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        if ( ImGui::Checkbox( "Center H", &m_CenterHorizontally ) && m_CenterHorizontally )
        {
            CenterWindowHorizontally();
        }

        if ( ImGui::Checkbox( "Center V", &m_CenterVertically ) && m_CenterVertically )
        {
            CenterWindowVertically();
        }
    }
}

void Window::Show( bool state )
{
    Element::Show( state );

    // This should be set if this window can be closed. Otherwise it will trigger when displaying permanent windows (such as the galaxy view)
    // and block input forever.
    if ( m_CanBeClosed )
    {
        g_pGame->SetInputBlocked( state );
    }
}

void Window::CenterWindowHorizontally()
{
    int x, y;
    GetPosition( x, y );
    x = ( Genesis::Configuration::GetScreenWidth() - GetWidth() ) / 2;
    SetPosition( x, y );
}

void Window::CenterWindowVertically()
{
    int x, y;
    GetPosition( x, y );
    y = ( Genesis::Configuration::GetScreenHeight() - GetHeight() ) / 2;
    SetPosition( x, y );
}

void Window::SetSize( int width, int height )
{
    Element::SetSize( width, height );
    m_pTitlePanel->SetSize( width, sTitleHeight );
    m_pTitleText->SetSize( width, sTitleHeight );

    m_pContentPanel->SetSize( width, height - sTitleHeight );
    m_pContentPanel->SetPosition( 0, sTitleHeight );
}

void Window::SetTitle( const std::string& title )
{
    m_pTitleText->SetText( title );
}

} // namespace Hexterminate::UI
