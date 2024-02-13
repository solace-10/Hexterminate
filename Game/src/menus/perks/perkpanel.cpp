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

#include <genesis.h>
#include <resourcemanager.h>
#include <resources/resourceimage.h>

#include "menus/perks/perkpanel.h"

#include "gameevents.h"
#include "hexterminate.h"
#include "menus/perks/perkcategorypanel.h"
#include "menus/perks/perktooltip.h"
#include "menus/popup.h"
#include "misc/gui.h"
#include "player.h"

namespace Hexterminate
{

PerkPanel::PerkPanel()
    : m_pTitle( nullptr )
    , m_pIcon( nullptr )
    , m_pCost( nullptr )
    , m_Cost( 0 )
    , m_Perk( Perk::Count )
    , m_pTooltip( nullptr )
    , m_PendingPurchase( false )
{
}

PerkPanel::~PerkPanel()
{
    delete m_pTooltip;
}

void PerkPanel::Init( Perk perk, const std::string& name, const std::string& description, const std::string& icon, int cost, PerkState state )
{
    m_Perk = perk;
    m_Cost = cost;

    m_pTitle = GuiExtended::CreateText( 8, 80, 256, 32, name, this );
    AlignToCentre( m_pTitle );

    m_pIcon = new PerkButton( this, perk, cost );
    m_pIcon->SetSize( 64, 64 );
    m_pIcon->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
    m_pIcon->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
    m_pIcon->SetPosition( GetWidth() / 2 - 32, 8 );
    m_pIcon->SetTexture( (Genesis::ResourceImage*)Genesis::FrameWork::GetResourceManager()->GetResource( icon ) );

    AddElement( m_pIcon );

    if ( state != PerkState::Enabled )
    {
        m_pCost = GuiExtended::CreateText( 8, 32, 256, 32, std::to_string( cost ), this, true );
        m_pCost->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
        AlignToCentre( m_pCost );
    }

    m_pTooltip = new PerkTooltip();
    m_pTooltip->Init( name, description, cost );
    m_pTooltip->Show( false );

    SetState( state );
}

void PerkPanel::Show( bool state )
{
    Genesis::Gui::Panel::Show( state );

    if ( state == false )
    {
        m_pTooltip->Show( false );
    }
}

void PerkPanel::Update( float delta )
{
    Genesis::Gui::Panel::Update( delta );

    m_pTooltip->Show( IsMouseInside() && g_pGame->GetPopup()->IsActive() == false );

    if ( m_PendingPurchase )
    {
        PopupState state = g_pGame->GetPopup()->GetState();
        if ( state == PopupState::Yes )
        {
            Player* pPlayer = g_pGame->GetPlayer();
            pPlayer->SetPerkPoints( pPlayer->GetPerkPoints() - m_Cost );
            pPlayer->GetPerks()->Enable( m_Perk );
            GameEventManager::Broadcast( new GameEvent( GameEventType::PerkAcquired ) );
            m_PendingPurchase = false;
        }
        else if ( state == PopupState::No )
        {
            m_PendingPurchase = false;
        }
    }
}

void PerkPanel::AlignToCentre( Genesis::Gui::Text* pText )
{
    unsigned int centreX = GetWidth() / 2;
    pText->SetPosition( floorf( centreX - pText->GetFont()->GetTextLength( pText->GetText() ) / 2.0f ), pText->GetPosition().y );
}

void PerkPanel::SetState( PerkState state )
{
    bool showCost = true;

    if ( state == PerkState::Disabled )
    {
        m_pIcon->SetColour( 1.0f, 0.0f, 0.0f, 0.6f );
        m_pIcon->SetEnabled( false );
        m_pTitle->SetColour( 1.0f, 1.0f, 1.0f, 0.6f );
    }
    else if ( state == PerkState::Locked )
    {
        m_pIcon->SetColour( 0.5f, 0.5f, 0.5f, 0.6f );
        m_pIcon->SetEnabled( true );
        m_pTitle->SetColour( 1.0f, 1.0f, 1.0f, 0.6f );
    }
    else
    {
        m_pIcon->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
        m_pIcon->SetEnabled( false );
        m_pTitle->SetColour( EVA_TEXT_COLOUR );
        showCost = false;
    }

    m_pTooltip->SetState( state );

    if ( m_pCost != nullptr )
    {
        m_pCost->Show( showCost );
    }
}

void PerkPanel::SetPendingPurchase()
{
    m_PendingPurchase = true;
}

/////////////////////////////////////////////////////////////////////
// PerkButton
/////////////////////////////////////////////////////////////////////

PerkButton::PerkButton( PerkPanel* pOwner, Perk perk, int cost )
    : m_pOwner( pOwner )
    , m_Cost( cost )
    , m_Enabled( false )
{
}

void PerkButton::OnPress()
{
    Genesis::Gui::ButtonImage::OnPress();

    if ( m_Enabled )
    {
        Player* pPlayer = g_pGame->GetPlayer();
        const int playerPerkPoints = pPlayer->GetPerkPoints();
        if ( playerPerkPoints >= m_Cost )
        {
            g_pGame->GetPopup()->Show( PopupMode::YesNo, "Do you want to purchase this perk?" );
            m_pOwner->SetPendingPurchase();
        }
    }
}

} // namespace Hexterminate
