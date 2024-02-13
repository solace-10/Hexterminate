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

#include "menus/hyperspacemenu.h"
#include "fleet/fleet.h"
#include "hexterminate.h"
#include "player.h"
#include "sector/sector.h"
#include "ship/hyperspacecore.h"
#include "ship/module.h"
#include "ship/ship.h"
#include <configuration.h>
#include <genesis.h>
#include <sstream>

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// HyperspaceMenu
///////////////////////////////////////////////////////////////////////////////

HyperspaceMenu::HyperspaceMenu()
    : m_pWindow( nullptr )
    , m_pText( nullptr )
    , m_pText2( nullptr )
    , m_pButtonJump( nullptr )
    , m_pButtonCancel( nullptr )
    , m_ShowProgress( false )
{
    unsigned int menuWidth = Genesis::Configuration::GetScreenWidth();
    unsigned int menuHeight = 72;
    unsigned int menuX = 0;
    unsigned int menuY = Genesis::Configuration::GetScreenHeight() / 2 - menuHeight / 2;
    unsigned int centreX = Genesis::Configuration::GetScreenWidth() / 2;

    m_pWindow = new EvaWindow( menuX, menuY, menuWidth, menuHeight, true );
    m_pWindow->GetMainPanel()->Show( false );
    m_pWindow->GetMainPanel()->SetBorderMode( Genesis::Gui::PANEL_BORDER_BOTTOM | Genesis::Gui::PANEL_BORDER_TOP );

    m_pText = new Genesis::Gui::Text();
    m_pText->SetFont( EVA_FONT );
    m_pText->SetColour( EVA_TEXT_COLOUR );
    m_pText->SetSize( 256.0f, 64.0f );
    m_pText->SetText( "Jump into hyperspace?" );
    m_pText->SetPosition( 0.0f, 8.0f );
    AlignToCentre( m_pText );
    m_pWindow->GetMainPanel()->AddElement( m_pText );

    m_pText2 = new Genesis::Gui::Text();
    m_pText2->SetFont( EVA_FONT );
    m_pText2->SetColour( EVA_TEXT_COLOUR );
    m_pText2->SetSize( 256.0f, 64.0f );
    m_pText2->SetPosition( 0.0f, 24.0f );
    m_pWindow->GetMainPanel()->AddElement( m_pText2 );

    m_pButtonJump = new ButtonHyperspaceJump( this );
    m_pButtonJump->SetPosition( centreX - 128.0f - 4.0f, 32.0f );
    m_pButtonJump->SetSize( 128.0f, 32.0f );
    m_pButtonJump->SetColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 0.05f ) );
    m_pButtonJump->SetHoverColour( EVA_BUTTON_COLOUR_HOVER );
    m_pButtonJump->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
    m_pButtonJump->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
    m_pButtonJump->SetFont( EVA_FONT );
    m_pButtonJump->SetText( "Jump" );
    m_pWindow->GetMainPanel()->AddElement( m_pButtonJump );

    m_pButtonCancel = new ButtonHyperspaceCancel( this );
    m_pButtonCancel->SetPosition( centreX + 4.0f, 32.0f );
    m_pButtonCancel->SetSize( 128.0f, 32.0f );
    m_pButtonCancel->SetColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 0.05f ) );
    m_pButtonCancel->SetHoverColour( EVA_BUTTON_COLOUR_HOVER );
    m_pButtonCancel->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
    m_pButtonCancel->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
    m_pButtonCancel->SetFont( EVA_FONT );
    m_pButtonCancel->SetText( "Cancel" );
    m_pWindow->GetMainPanel()->AddElement( m_pButtonCancel );
}

HyperspaceMenu::~HyperspaceMenu()
{
    delete m_pWindow;
}

void HyperspaceMenu::Update( float delta )
{
    // Only show if the player's ship is still alive
    Ship* pPlayerShip = g_pGame->GetPlayer()->GetShip();
    if ( pPlayerShip != nullptr && pPlayerShip->GetTowerModule() != nullptr && pPlayerShip->GetTowerModule()->GetHealth() <= 0.0f )
    {
        Show( false );
    }
    else if ( pPlayerShip != nullptr && m_ShowProgress )
    {
        HyperspaceCore* pCore = pPlayerShip->GetHyperspaceCore();
        SDL_assert( pCore != nullptr );
        if ( pCore->IsCharging() )
        {
            std::stringstream ss;
            int chargePercentage = (int)( pCore->GetChargePercentage() * 100.0f );
            ss << "[ " << chargePercentage << "% ]";
            m_pText2->SetText( ss.str() );
            AlignToCentre( m_pText2 );
        }
        else if ( pCore->IsJumping() )
        {
            m_pText->SetText( "Jumping" );
            m_pText2->SetText( "[ 100% ]" );
            AlignToCentre( m_pText );
            AlignToCentre( m_pText2 );
        }
    }
}

void HyperspaceMenu::Show( bool state )
{
    Ship* pPlayerShip = g_pGame->GetPlayer()->GetShip();
    if ( state == true && pPlayerShip != nullptr && pPlayerShip->GetTowerModule() != nullptr && pPlayerShip->GetTowerModule()->GetHealth() <= 0.0f )
        return;

    m_pWindow->GetMainPanel()->Show( state );
    g_pGame->SetInputBlocked( state );
    g_pGame->ShowCursor( true );

    if ( state )
    {
        g_pGame->Pause();
    }
    else
    {
        g_pGame->Unpause();
    }
}

void HyperspaceMenu::ShowProgress()
{
    m_ShowProgress = true;
    m_pButtonJump->Show( false );
    m_pButtonCancel->Show( false );
    m_pWindow->GetMainPanel()->SetPosition( 0.0f, 32.0f );
    m_pWindow->GetMainPanel()->SetSize( (float)Genesis::Configuration::GetScreenWidth(), 48.0f );
    m_pText->SetText( "Hyperspace core charging" );
    AlignToCentre( m_pText );
    g_pGame->SetInputBlocked( false );
    g_pGame->Unpause();
}

void HyperspaceMenu::AlignToCentre( Genesis::Gui::Text* pText )
{
    unsigned int centreX = Genesis::Configuration::GetScreenWidth() / 2;
    pText->SetPosition( floorf( centreX - pText->GetFont()->GetTextLength( pText->GetText() ) / 2.0f ), pText->GetPosition().y );
}

///////////////////////////////////////////////////////////////////////////////
// ButtonHyperspaceJump
///////////////////////////////////////////////////////////////////////////////

ButtonHyperspaceJump::ButtonHyperspaceJump( HyperspaceMenu* pOwner )
    : m_pOwner( pOwner )
{
}

void ButtonHyperspaceJump::OnPress()
{
    bool immediateJump = g_pGame->GetCurrentSector()->IsPlayerVictorious();
    const ShipList& shipList = g_pGame->GetCurrentSector()->GetShipList();
    for ( Ship* pShip : shipList )
    {
        if ( pShip->GetFaction() == g_pGame->GetPlayerFaction() )
        {
            pShip->GetHyperspaceCore()->EnterHyperspace( immediateJump );
        }
    }

    if ( immediateJump )
    {
        m_pOwner->Show( false );
    }
    else
    {
        m_pOwner->ShowProgress();
    }
}

///////////////////////////////////////////////////////////////////////////////
// ButtonHyperspaceCancel
///////////////////////////////////////////////////////////////////////////////

ButtonHyperspaceCancel::ButtonHyperspaceCancel( HyperspaceMenu* pOwner )
    : m_pOwner( pOwner )
{
}

void ButtonHyperspaceCancel::OnPress()
{
    m_pOwner->Show( false );
}

} // namespace Hexterminate