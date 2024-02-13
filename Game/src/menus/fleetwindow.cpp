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

#include "menus/fleetwindow.h"

#include <sstream>

#include <configuration.h>
#include <genesis.h>

#include "fleet/fleet.h"
#include "hexterminate.h"
#include "player.h"
#include "ship/shipinfo.h"
#include "stringaux.h"
#include "ui/button.h"
#include "ui/image.h"
#include "ui/panel.h"
#include "ui/rootelement.h"
#include "ui/scrollingelement.h"
#include "ui/text.h"

namespace Hexterminate
{

/////////////////////////////////////////////////////////////////////
// FleetWindow
/////////////////////////////////////////////////////////////////////

FleetWindow::FleetWindow()
    : UI::Window( "Fleet window", true )
    , m_RequisitionTextInitialized( false )
{
    m_pFleetPanel = std::make_shared<UI::Panel>( "Fleet panel" );
    GetContentPanel()->Add( m_pFleetPanel );
    m_pFleetSubtitle = std::make_shared<UI::Text>( "Fleet subtitle" );
    m_pFleetPanel->Add( m_pFleetSubtitle );
    m_pRequisitionShipsPanel = std::make_shared<UI::Panel>( "Requisition ships panel " );
    GetContentPanel()->Add( m_pRequisitionShipsPanel );
    m_pRequisitionShipsSubtitle = std::make_shared<UI::Text>( "Requisition ships subtitle" );
    m_pRequisitionShipsPanel->Add( m_pRequisitionShipsSubtitle );
    m_pRequisitionShipsArea = std::make_shared<UI::ScrollingElement>( "Requisition ships area" );
    m_pRequisitionShipsPanel->Add( m_pRequisitionShipsArea );

    PopulateFleetShips();
    PopulateRequisitionShips();
}

FleetWindow::~FleetWindow()
{
}

void FleetWindow::Show( bool state )
{
    UI::Window::Show( state );

    if ( state )
    {
        // Setting the initial text from the ShipInfo can't be done on window construction,
        // as it gets overwritten by the design.
        if ( m_RequisitionTextInitialized == false )
        {
            for ( auto& rsi : m_RequisitionShipInfos )
            {
                rsi.pTitle->SetText( rsi.pShipInfo->GetLongDisplayName() );
                rsi.pBackground->SetPath( rsi.pShipInfo->GetDisplayImage() );

                std::stringstream weaponsText;
                weaponsText << "Weapons: " << rsi.pShipInfo->GetWeaponsText();
                rsi.pWeaponsText->SetText( weaponsText.str() );

                std::stringstream defenseText;
                defenseText << "Defense: " << rsi.pShipInfo->GetDefenseText();
                rsi.pDefenseText->SetText( defenseText.str() );

                std::stringstream influenceText;
                influenceText << "Influence: " << rsi.pShipInfo->GetCost();
                rsi.pInfluenceText->SetText( influenceText.str() );
                if ( HasSufficientInfluence( rsi.pShipInfo ) )
                {
                    rsi.pInfluenceIcon->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
                    rsi.pInfluenceText->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
                }
                else
                {
                    rsi.pInfluenceIcon->SetColour( 1.0f, 0.0f, 0.0f, 1.0f );
                    rsi.pInfluenceText->SetColour( 1.0f, 0.0f, 0.0f, 1.0f );
                }

                std::optional<Perk> requiredPerk = rsi.pShipInfo->GetRequiredPerk();
                if ( requiredPerk.has_value() == false )
                {
                    rsi.pPerkIcon->Show( false );
                    rsi.pPerkText->Show( false );
                }
                else
                {
                    rsi.pPerkIcon->Show( true );
                    rsi.pPerkText->Show( true );

                    std::stringstream perkText;
                    perkText << "Perk: " << ToString( requiredPerk.value() );
                    rsi.pPerkText->SetText( perkText.str() );

                    if ( g_pGame->GetPlayer()->GetPerks()->IsEnabled( requiredPerk.value() ) )
                    {
                        rsi.pPerkIcon->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
                        rsi.pPerkText->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
                    }
                    else
                    {
                        rsi.pPerkIcon->SetColour( 1.0f, 0.0f, 0.0f, 1.0f );
                        rsi.pPerkText->SetColour( 1.0f, 0.0f, 0.0f, 1.0f );
                    }
                }

                rsi.pRequisitionButton->Enable( CanRequisitionShip( rsi.pShipInfo ) );
            }

            m_RequisitionTextInitialized = true;
        }

        RefreshFleetShips();
        RefreshRequisitionShips();

        g_pGame->Pause();
    }
    else
    {
        g_pGame->Unpause();
    }
}

void FleetWindow::PopulateFleetShips()
{
    for ( int i = 0; i < 5; ++i )
    {
        std::stringstream panelId;
        panelId << "Ship " << i + 1;

        FleetShipInfo fsi;
        fsi.isPlayerShip = ( i == 0 );
        fsi.pBackground = std::make_shared<UI::Image>( panelId.str() );
        m_pFleetPanel->Add( fsi.pBackground );
        fsi.pNameText = std::make_shared<UI::Text>( "Name text" );
        fsi.pBackground->Add( fsi.pNameText );
        fsi.pCategoryText = std::make_shared<UI::Text>( "Category text" );
        fsi.pBackground->Add( fsi.pCategoryText );
        fsi.pReturnButton = std::make_shared<UI::Button>( "Return button", []( std::any userData ) {} );
        fsi.pBackground->Add( fsi.pReturnButton );
        fsi.pUnavailableIcon = std::make_shared<UI::Image>( "Unavailable icon" );
        fsi.pUnavailableIcon->SetShader( "gui_textured" );
        fsi.pUnavailableIcon->SetColour( 1.0f, 0.0f, 0.0f, 1.0f );
        fsi.pBackground->Add( fsi.pUnavailableIcon );
        fsi.pUnavailableText = std::make_shared<UI::Text>( "Unavailable text" );
        fsi.pBackground->Add( fsi.pUnavailableText );

        m_FleetShipInfos.push_back( fsi );
    }
}

void FleetWindow::PopulateRequisitionShips()
{
    const std::string shipNames[] = {
        "lancer",
        "phalanx",
        "ironclad",
        "t1_battlecruiser_luna",
        "t1_capital_dominion",
        "special_t2_gunship_lancer_mk2",
        "special_t2_battlecruiser_wyvern",
        "special_t2_battlecruiser_glory",
        "special_t2_battlecruiser_keeper",
        "special_t3_battlecruiser_dies_irae",
        "special_t3_battlecruiser_tyrant",
        "special_t3_capital_arbellatris"
    };

    Faction* pImperialFaction = g_pGame->GetFaction( FactionId::Empire );
    for ( auto& shipName : shipNames )
    {
        const ShipInfo* pShipInfo = g_pGame->GetShipInfoManager()->Get( pImperialFaction, shipName );
        if ( pShipInfo != nullptr )
        {
            RequisitionShipInfo rsi;
            rsi.pShipInfo = pShipInfo;
            rsi.pPanel = std::make_shared<UI::Panel>( shipName );
            rsi.pBackground = std::make_shared<UI::Image>( "Background" );
            rsi.pTitle = std::make_shared<UI::Text>( "Title" );

            m_pRequisitionShipsArea->GetScrollingArea()->Add( rsi.pPanel );
            rsi.pPanel->Add( rsi.pBackground );
            rsi.pBackground->Add( rsi.pTitle );

            rsi.pTextPanel = std::make_shared<UI::Panel>( "Text panel" );
            rsi.pBackground->Add( rsi.pTextPanel );

            rsi.pWeaponsIcon = std::make_shared<UI::Image>( "Weapons icon" );
            rsi.pTextPanel->Add( rsi.pWeaponsIcon );
            rsi.pWeaponsText = std::make_shared<UI::Text>( "Weapons text" );
            rsi.pTextPanel->Add( rsi.pWeaponsText );
            rsi.pDefenseIcon = std::make_shared<UI::Image>( "Defense icon" );
            rsi.pTextPanel->Add( rsi.pDefenseIcon );
            rsi.pDefenseText = std::make_shared<UI::Text>( "Defense text" );
            rsi.pTextPanel->Add( rsi.pDefenseText );
            rsi.pInfluenceIcon = std::make_shared<UI::Image>( "Influence icon" );
            rsi.pInfluenceIcon->SetShader( "gui_textured" );
            rsi.pTextPanel->Add( rsi.pInfluenceIcon );
            rsi.pInfluenceText = std::make_shared<UI::Text>( "Influence text" );
            rsi.pTextPanel->Add( rsi.pInfluenceText );
            rsi.pPerkIcon = std::make_shared<UI::Image>( "Perk icon" );
            rsi.pPerkIcon->SetShader( "gui_textured" );
            rsi.pTextPanel->Add( rsi.pPerkIcon );
            rsi.pPerkText = std::make_shared<UI::Text>( "Perk text" );
            rsi.pTextPanel->Add( rsi.pPerkText );

            rsi.pRequisitionButton = std::make_shared<UI::Button>( "Requisition button", [ this, pShipInfo ]( std::any userData ) { RequisitionShip( pShipInfo ); } );
            rsi.pRequisitionButton->Enable( CanRequisitionShip( pShipInfo ) );
            rsi.pBackground->Add( rsi.pRequisitionButton );

            m_RequisitionShipInfos.push_back( rsi );
        }
    }
}

void FleetWindow::RefreshFleetShips()
{
    const size_t maxFleetShips = m_FleetShipInfos.size();
    FleetSharedPtr pPlayerFleet = g_pGame->GetPlayerFleet().lock();
    const ShipInfoList& fleetShips = pPlayerFleet->GetShips();

    FleetShipInfo& playerFsi = m_FleetShipInfos[ 0 ];
    playerFsi.pNameText->SetText( g_pGame->GetPlayer()->GetShipCustomisationData().m_ShipName );
    playerFsi.pCategoryText->SetText( "Flagship" );
    playerFsi.pReturnButton->Show( false );
    playerFsi.pUnavailableIcon->Show( false );
    playerFsi.pUnavailableText->Show( false );

    size_t usedSlots = 1;
    for ( const ShipInfo* pShipInfo : fleetShips )
    {
        FleetShipInfo& fsi = m_FleetShipInfos[ usedSlots++ ];
        fsi.pNameText->Show( true );
        fsi.pNameText->SetText( pShipInfo->GetDisplayName() );
        fsi.pCategoryText->Show( true );
        fsi.pCategoryText->SetText( ToString( pShipInfo->GetShipType() ) );
        fsi.pReturnButton->Show( true );
        fsi.pReturnButton->SetOnPressed(
            [ pShipInfo, this ]( const std::any& userData ) {
                g_pGame->ReturnShip( pShipInfo );
                RefreshFleetShips();
                RefreshRequisitionShips();
            } );
        fsi.pUnavailableIcon->Show( false );
        fsi.pUnavailableText->Show( false );
    }

    Perks* pPerks = g_pGame->GetPlayer()->GetPerks();
    for ( size_t i = usedSlots; i < maxFleetShips; ++i )
    {
        FleetShipInfo& fsi = m_FleetShipInfos[ i ];
        fsi.pNameText->Show( false );
        fsi.pCategoryText->Show( false );
        fsi.pReturnButton->Show( false );

        if ( i == 3 && pPerks->IsEnabled( Perk::SupportRequest ) == false )
        {
            fsi.pUnavailableIcon->Show( true );
            fsi.pUnavailableText->Show( true );
        }
        else if ( i == 4 && pPerks->IsEnabled( Perk::PrimaryFleet ) == false )
        {
            fsi.pUnavailableIcon->Show( true );
            fsi.pUnavailableText->Show( true );
        }
        else
        {
            fsi.pUnavailableIcon->Show( false );
            fsi.pUnavailableText->Show( false );
        }
    }
}

void FleetWindow::RefreshRequisitionShips()
{
    for ( auto& rsi : m_RequisitionShipInfos )
    {
        if ( CanRequisitionShip( rsi.pShipInfo ) )
        {
            rsi.pBackground->SetShader( "gui_textured" );
            rsi.pRequisitionButton->Enable( true );
        }
        else
        {
            rsi.pBackground->SetShader( "gui_textured_greyscale" );
            rsi.pRequisitionButton->Enable( false );
        }
    }
}

bool FleetWindow::CanRequisitionShip( const ShipInfo* pShipInfo ) const
{
    if ( g_pGame->GetPlayer() == nullptr || g_pGame->GetPlayerFleet().expired() )
    {
        return false;
    }
    else
    {
        const bool fleetHasSpace = g_pGame->GetPlayerFleet().lock()->GetShips().size() + 1 < g_pGame->GetPlayer()->GetFleetMaxShips();
        return fleetHasSpace && HasSufficientInfluence( pShipInfo ) && HasNecessaryPerk( pShipInfo );
    }
}

bool FleetWindow::HasSufficientInfluence( const ShipInfo* pShipInfo ) const
{
    return g_pGame->GetPlayer()->GetInfluence() >= pShipInfo->GetCost();
}

bool FleetWindow::HasNecessaryPerk( const ShipInfo* pShipInfo ) const
{
    std::optional<Perk> requiredPerk = pShipInfo->GetRequiredPerk();
    if ( requiredPerk.has_value() == false )
    {
        return true;
    }
    else
    {
        return g_pGame->GetPlayer()->GetPerks()->IsEnabled( requiredPerk.value() );
    }
}

void FleetWindow::RequisitionShip( const ShipInfo* pShipInfo )
{
    if ( g_pGame->RequisitionShip( pShipInfo ) )
    {
        RefreshFleetShips();
        RefreshRequisitionShips();
    }
}

} // namespace Hexterminate
