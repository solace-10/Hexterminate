// Copyright 2014 Pedro Nunes
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

#include "menus/deathmenu.h"
#include "fleet/fleet.h"
#include "hexterminate.h"
#include "player.h"
#include "sector/sector.h"
#include "ship/inventory.h"
#include "ship/module.h"
#include "ship/ship.h"
#include <configuration.h>
#include <genesis.h>
#include <sstream>

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// DeathMenu
///////////////////////////////////////////////////////////////////////////////

DeathMenu::DeathMenu()
    : m_pWindow( nullptr )
    , m_pText( nullptr )
    , m_pButtonExit( nullptr )
    , m_pButtonRespawn( nullptr )
    , m_DisplayTimer( 0.0f )
    , m_pRespawnSector( nullptr )
{
    unsigned int menuWidth = Genesis::Configuration::GetScreenWidth();
    unsigned int menuHeight = 72;
    unsigned int menuX = 0;
    unsigned int menuY = Genesis::Configuration::GetScreenHeight() / 2 - menuHeight / 2;
    unsigned int centreX = Genesis::Configuration::GetScreenWidth() / 2;

    m_pWindow = new EvaWindow( menuX, menuY, menuWidth, menuHeight, true );
    m_pWindow->GetMainPanel()->Show( false );
    m_pWindow->GetMainPanel()->SetBorderMode( Genesis::Gui::PANEL_BORDER_BOTTOM | Genesis::Gui::PANEL_BORDER_TOP );

    m_pText = new Genesis::Gui::Text;
    m_pText->SetFont( EVA_FONT );
    m_pText->SetColour( EVA_TEXT_COLOUR );
    m_pText->SetSize( Genesis::Configuration::GetScreenWidth(), 64 );
    m_pWindow->GetMainPanel()->AddElement( m_pText );

    m_pButtonExit = new ButtonDeathExit();
    m_pButtonExit->SetSize( 128.0f, 32.0f );
    m_pButtonExit->SetColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 0.05f ) );
    m_pButtonExit->SetHoverColour( EVA_BUTTON_COLOUR_HOVER );
    m_pButtonExit->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
    m_pButtonExit->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
    m_pButtonExit->SetFont( EVA_FONT );
    m_pButtonExit->SetText( "Main menu" );
    m_pWindow->GetMainPanel()->AddElement( m_pButtonExit );

    m_pButtonRespawn = new ButtonDeathRespawn( this );
    m_pButtonRespawn->SetPosition( centreX + 4.0f, 32.0f );
    m_pButtonRespawn->SetSize( 128.0f, 32.0f );
    m_pButtonRespawn->SetColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 0.05f ) );
    m_pButtonRespawn->SetHoverColour( EVA_BUTTON_COLOUR_HOVER );
    m_pButtonRespawn->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
    m_pButtonRespawn->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
    m_pButtonRespawn->SetFont( EVA_FONT );
    m_pButtonRespawn->SetText( "Respawn" );
    m_pButtonRespawn->Show( false );
    m_pWindow->GetMainPanel()->AddElement( m_pButtonRespawn );

    Show( false );
}

DeathMenu::~DeathMenu()
{
    delete m_pWindow;
}

void DeathMenu::Update( float delta )
{
    if ( m_pWindow->GetMainPanel()->IsVisible() == false )
    {
        Ship* pPlayerShip = g_pGame->GetPlayer()->GetShip();
        if ( pPlayerShip != nullptr && pPlayerShip->GetTowerModule() != nullptr && pPlayerShip->GetTowerModule()->GetHealth() <= 0.0f )
        {
            if ( m_DisplayTimer >= 5.0f )
            {
                UpdateButtons();
                Show( true );
            }
            else
            {
                m_DisplayTimer += delta;
            }
        }
    }
}

void DeathMenu::UpdateButtons()
{
    const bool hardcore = ( g_pGame->GetDifficulty() == Difficulty::Hardcore );
    m_pRespawnSector = hardcore ? nullptr : g_pGame->FindSpawnSector();

    if ( hardcore )
    {
        m_pText->SetText( "Your ship has been destroyed, but your name will be inscribed in the Halls of Glory." );
    }
    else
    {
        if ( m_pRespawnSector == nullptr )
        {
            m_pText->SetText( "Your ship has been destroyed and the Empire has lost all its shipyards. The war has been lost." );
        }
        else
        {
            m_pText->SetText( "Your ship has been destroyed." );
        }
    }

    const bool respawnAllowed = ( !hardcore && m_pRespawnSector != nullptr );

    m_pText->SetPosition( 0.0f, 8.0f );
    AlignToCentre( m_pText );

    unsigned int centreX = Genesis::Configuration::GetScreenWidth() / 2;
    m_pButtonExit->SetPosition( !respawnAllowed ? ( centreX - 64.0f ) : ( centreX - 128.0f - 4.0f ), 32.0f );

    m_pButtonRespawn->Show( respawnAllowed );
}

void DeathMenu::Show( bool state )
{
    m_pWindow->GetMainPanel()->Show( state );
}

void DeathMenu::AlignToCentre( Genesis::Gui::Text* pText )
{
    unsigned int centreX = Genesis::Configuration::GetScreenWidth() / 2;
    pText->SetPosition( floorf( centreX - pText->GetFont()->GetTextLength( pText->GetText() ) / 2.0f ), pText->GetPosition().y );
}

///////////////////////////////////////////////////////////////////////////////
// ButtonDeathExit
///////////////////////////////////////////////////////////////////////////////

ButtonDeathExit::ButtonDeathExit()
{
}

void ButtonDeathExit::OnPress()
{
    g_pGame->EndGame();
}

///////////////////////////////////////////////////////////////////////////////
// ButtonDeathRespawn
///////////////////////////////////////////////////////////////////////////////

ButtonDeathRespawn::ButtonDeathRespawn( DeathMenu* pOwner )
    : m_pOwner( pOwner )
{
}

void ButtonDeathRespawn::OnPress()
{
    SectorInfo* respawnSector = m_pOwner->GetRespawnSector();
    SDL_assert( respawnSector != nullptr );
    g_pGame->ExitSector();

    if ( g_pGame->GetDifficulty() == Difficulty::Normal )
    {
        g_pGame->GetPlayer()->GetInventory()->DeductCachedModules();
    }

    FleetSharedPtr pPlayerFleet = g_pGame->GetPlayerFleet().lock();
    if ( pPlayerFleet != nullptr )
    {
        int sectorX, sectorY;
        respawnSector->GetCoordinates( sectorX, sectorY );
        float x = ( (float)sectorX + 0.5f ) / NumSectorsX;
        float y = ( (float)sectorY + 0.5f ) / NumSectorsY;
        pPlayerFleet->SetPosition( x, y );
        pPlayerFleet->SetDestination( x, y );
        pPlayerFleet->SetEngaged( false );
    }
}

} // namespace Hexterminate