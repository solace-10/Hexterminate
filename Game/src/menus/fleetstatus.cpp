// Copyright 2020 Pedro Nunes
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

#include <iomanip>
#include <sstream>

#include "hexterminate.h"
#include "menus/eva.h"
#include "menus/fleetstatus.h"
#include "menus/table.h"
#include "menus/tablerow.h"
#include "player.h"
#include "sector/sector.h"
#include "ship/module.h"
#include "ship/moduleinfo.h"
#include "ship/shield.h"
#include "ship/shipinfo.h"
#include "stringaux.h"
#include <configuration.h>
#include <genesis.h>

namespace Hexterminate
{

FleetStatus::FleetStatus()
    : m_pMainPanel( nullptr )
    , m_pTable( nullptr )
    , m_pShipLostSFX( nullptr )
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();

    const float panelWidth = 256.0f;
    const float screenHeight = static_cast<float>( Genesis::Configuration::GetScreenHeight() );
    const float outerBorder = 8.0f;
    const float innerBorder = 8.0f;

    m_pMainPanel = new Genesis::Gui::Panel();
    m_pMainPanel->SetPosition( outerBorder, outerBorder );
    m_pMainPanel->SetColour( 0.0f, 0.0f, 0.0f, 0.40f );
    m_pMainPanel->SetBorderColour( 1.0f, 1.0f, 1.0f, 0.25f );
    m_pMainPanel->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
    pGuiManager->AddElement( m_pMainPanel );

    m_pTable = new Table();
    m_pTable->SetPosition( innerBorder, innerBorder );

    AddFleetRows();

    // Resize window to fit all the rows we need
    const float tableHeight = m_pTable->GetRowCount() * m_pTable->GetRowHeight();
    m_pTable->SetSize( panelWidth - innerBorder * 2.0f, tableHeight );
    m_pMainPanel->SetSize( panelWidth, tableHeight + innerBorder * 2.0f );
    m_pMainPanel->SetPosition( innerBorder, screenHeight - tableHeight - innerBorder * 2.0f - outerBorder );
    m_pMainPanel->AddElement( m_pTable );

    m_pShipLostSFX = Genesis::Gui::LoadSFX( "data/sfx/ship_lost.wav" );
}

FleetStatus::~FleetStatus()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();
    if ( pGuiManager != nullptr )
    {
        pGuiManager->RemoveElement( m_pMainPanel );
    }
}

void FleetStatus::AddFleetRows()
{
    for ( int i = 0; i < 5; ++i )
    {
        Entry entry;
        entry.m_pRow = new TableRow();
        entry.m_pRow->Add( "" );
        entry.m_pRow->Add( "" );
        entry.m_pShip = nullptr;
        entry.m_Destroyed = true;
        m_Entries.push_back( entry );
        m_pTable->AddRow( entry.m_pRow );
    }
}

void FleetStatus::AddShip( Ship* pShip )
{
    for ( Entry& entry : m_Entries )
    {
        if ( entry.m_pShip == nullptr )
        {
            entry.m_pShip = pShip;
            entry.m_Destroyed = false;
            break;
        }
    }
}

Genesis::Color FleetStatus::GetShipIntegrityColour( int integrity ) const
{
    if ( integrity > 75 )
    {
        return Genesis::Color( 1.0f, 1.0f, 1.0f );
    }
    else if ( integrity > 0 )
    {
        return Genesis::Color( 1.0f, 0.4f, 0.0f );
    }
    else
    {
        return Genesis::Color( 1.0f, 0.0f, 0.0f );
    }
}

void FleetStatus::Update()
{
    DockingState dockingState = g_pGame->GetPlayer()->GetShip()->GetDockingState();
    if ( dockingState != DockingState::Docking && dockingState != DockingState::Docked )
    {
        m_pMainPanel->Show( true );

        for ( Entry& entry : m_Entries )
        {
            if ( entry.m_pShip != nullptr )
            {
                std::string shipName = "";
                if ( entry.m_pShip->GetShipInfo() == nullptr )
                {
                    shipName = g_pGame->GetPlayer()->GetShipCustomisationData().m_ShipName;
                }
                else
                {
                    shipName = entry.m_pShip->GetShipInfo()->GetDisplayName();
                }

                std::stringstream shipNameText;
                shipNameText << std::setw( 20 ) << std::left << shipName;
                entry.m_pRow->Set( 0, shipNameText.str() );

                int integrity = entry.m_pShip->GetIntegrity();
                std::stringstream integrityText;
                integrityText << std::setw( 4 ) << std::right << integrity << "%";
                entry.m_pRow->Set( 1, integrityText.str() );
                entry.m_pRow->SetColour( GetShipIntegrityColour( integrity ) );

                if ( entry.m_Destroyed == false && entry.m_pShip->IsDestroyed() )
                {
                    entry.m_Destroyed = true;
                    Genesis::Gui::PlaySFX( m_pShipLostSFX );
                }
            }
            else
            {
                break;
            }
        }

        m_pTable->NotifyContentUpdated();
    }
    else
    {
        m_pMainPanel->Show( false );
    }
}

} // namespace Hexterminate
