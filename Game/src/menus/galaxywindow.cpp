// Copyright 2015 Pedro Nunes
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

#include "menus/galaxywindow.h"

#include <sstream>

#include <configuration.h>
#include <genesis.h>

#include "hexterminate.h"
#include "menus/endgamewindow.h"
#include "menus/eva.h"
#include "menus/fleetwindow.h"
#include "menus/perks/perkswindow.h"
#include "player.h"
#include "ui/button.h"
#include "ui/image.h"
#include "ui/panel.h"
#include "ui/rootelement.h"
#include "ui/text.h"

namespace Hexterminate
{

/////////////////////////////////////////////////////////////////////
// GalaxyWindow
/////////////////////////////////////////////////////////////////////

GalaxyWindow::GalaxyWindow()
    : UI::Window( "Galaxy window" )
    , m_pPerksWindow( nullptr )
    , m_pEndGameWindow( nullptr )
{
    m_pStardateText = std::make_shared<UI::Text>( "Stardate text" );
    GetContentPanel()->Add( m_pStardateText );
    m_pInfluenceText = std::make_shared<UI::Text>( "Influence text" );
    GetContentPanel()->Add( m_pInfluenceText );
    m_pPerksText = std::make_shared<UI::Text>( "Perks text" );
    GetContentPanel()->Add( m_pPerksText );

    GetContentPanel()->Add( std::make_shared<UI::Image>( "Stardate icon" ) );
    GetContentPanel()->Add( std::make_shared<UI::Image>( "Influence icon" ) );
    GetContentPanel()->Add( std::make_shared<UI::Image>( "Perks icon" ) );

    m_pFleetsButton = std::make_shared<UI::Button>( "Fleets button", [ this ]( const std::any& userData ) { ShowFleetWindow( !IsFleetWindowVisible() ); } );
    g_pGame->GetUIRoot()->Add( m_pFleetsButton );
    m_pPerksButton = std::make_shared<UI::Button>( "Perks button", [ this ]( const std::any& userData ) { ShowPerksWindow( !IsPerksWindowVisible() ); } );
    g_pGame->GetUIRoot()->Add( m_pPerksButton );

    m_pFleetWindow = std::make_shared<FleetWindow>();
    m_pFleetWindow->Show( false );
    g_pGame->GetUIRoot()->Add( m_pFleetWindow );

    m_pPerksWindow = new PerksWindow();
    m_pPerksWindow->Init();
    m_pPerksWindow->Show( false );

    m_pEndGameWindow = new EndGameWindow();
    m_pEndGameWindow->Init();
    m_pEndGameWindow->Show( false );
}

GalaxyWindow::~GalaxyWindow()
{
    delete m_pPerksWindow;
    delete m_pEndGameWindow;
}

void GalaxyWindow::Show( bool state )
{
    UI::Window::Show( state );
    m_pFleetsButton->Show( state );
    m_pPerksButton->Show( state );

    if ( m_pFleetWindow != nullptr && m_pFleetWindow->IsVisible() )
    {
        m_pFleetWindow->Show( false );
    }

    if ( m_pPerksWindow != nullptr && m_pPerksWindow->IsVisible() )
    {
        m_pPerksWindow->Show( false );
    }
}

void GalaxyWindow::ShowFleetWindow( bool state )
{
    if ( m_pPerksWindow != nullptr && state == true )
    {
        m_pPerksWindow->Show( false );
    }

    if ( m_pEndGameWindow != nullptr && state == true )
    {
        m_pEndGameWindow->Show( false );
    }

    if ( m_pFleetWindow != nullptr && m_pFleetWindow->IsVisible() == false )
    {
        m_pFleetWindow->Show( state );
    }
}

void GalaxyWindow::ShowPerksWindow( bool state )
{
    if ( m_pFleetWindow != nullptr && state == true )
    {
        m_pFleetWindow->Show( false );
    }

    if ( m_pEndGameWindow != nullptr && state == true )
    {
        m_pEndGameWindow->Show( false );
    }

    if ( m_pPerksWindow != nullptr && m_pPerksWindow->IsVisible() == false )
    {
        m_pPerksWindow->Show( state );
    }
}

void GalaxyWindow::ShowEndGameWindow()
{
    ShowFleetWindow( false );
    ShowPerksWindow( false );
    m_pEndGameWindow->Show( true );
}

bool GalaxyWindow::IsFleetWindowVisible() const
{
    return m_pFleetWindow != nullptr ? m_pFleetWindow->IsVisible() : false;
}

bool GalaxyWindow::IsPerksWindowVisible() const
{
    return m_pPerksWindow != nullptr ? m_pPerksWindow->IsVisible() : false;
}

bool GalaxyWindow::IsEndGameWindowVisible() const
{
    return m_pEndGameWindow != nullptr ? m_pEndGameWindow->IsVisible() : false;
}

bool GalaxyWindow::HasSubWindowsVisible() const
{
    return IsFleetWindowVisible() || IsPerksWindowVisible() || IsEndGameWindowVisible();
}

void GalaxyWindow::Update()
{
    UI::Element::Update();

    if ( g_pGame->GetPlayer() )
    {
        UpdateStardateText();
        UpdateInfluenceText();
        UpdatePerksText();
    }
}

void GalaxyWindow::UpdateInfluenceText()
{
    Player* pPlayer = g_pGame->GetPlayer();
    const std::string influence = std::to_string( pPlayer->GetInfluence() );
    m_pInfluenceText->SetText( "Influence:  " + influence );
}

void GalaxyWindow::UpdatePerksText()
{
    const std::string perkPoints = std::to_string( g_pGame->GetPlayer()->GetPerkPoints() );
    m_pPerksText->SetText( "Perk points:  " + perkPoints );
}

void GalaxyWindow::UpdateStardateText()
{
    const int stardateMajor = 9524 + (int)( g_pGame->GetPlayedTime() / 10.0f ) / 100;
    const int stardateMinor = 583 + (int)( g_pGame->GetPlayedTime() / 10.0f ) % 100;

    std::stringstream ss;
    ss << "Stardate:  " << stardateMajor << "." << stardateMinor;

    m_pStardateText->SetText( ss.str() );
}

void GalaxyWindow::HandleGameEvent( GameEvent* pEvent )
{
    if ( pEvent->GetType() == GameEventType::PerkAcquired )
    {
        UpdatePerksText();
    }
}

} // namespace Hexterminate
