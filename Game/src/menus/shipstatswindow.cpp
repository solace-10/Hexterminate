// Copyright 2024 Pedro Nunes
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

#include "menus/shipstatswindow.h"

#include <sstream>

#include <configuration.h>
#include <genesis.h>

#include "fleet/fleet.h"
#include "hexterminate.h"
#include "player.h"
#include "ship/shipinfo.h"
#include "stringaux.h"
#include "ui/icon.h"
#include "ui/image.h"
#include "ui/panel.h"
#include "ui/rootelement.h"
#include "ui/text.h"

namespace Hexterminate
{

/////////////////////////////////////////////////////////////////////
// ShipStatsWindow
/////////////////////////////////////////////////////////////////////

ShipStatsWindow::ShipStatsWindow()
    : UI::Window( "Ship stats window", true )
{
    AddSection( "Weapons", m_pWeaponsPanel, m_pWeaponsSubtitle );
    AddEntry( m_pWeaponsPanel, "Kinetic", m_pWeaponryKineticIcon, m_pWeaponryKineticText, m_pWeaponryKineticValue );
    AddEntry( m_pWeaponsPanel, "Energy", m_pWeaponryEnergyIcon, m_pWeaponryEnergyText, m_pWeaponryEnergyValue );
    AddEntry( m_pWeaponsPanel, "Exotic", m_pWeaponryExoticIcon, m_pWeaponryExoticText, m_pWeaponryExoticValue );

    AddSection( "Shields", m_pShieldsPanel, m_pShieldsSubtitle );
    AddSection( "Capacitor", m_pCapacitorPanel, m_pCapacitorSubtitle );
    AddSection( "Energy", m_pEnergyMatrixPanel, m_pEnergyMatrixSubtitle );
}

ShipStatsWindow::~ShipStatsWindow()
{
}

void ShipStatsWindow::Show( bool state )
{
    UI::Window::Show( state );
}

void ShipStatsWindow::AddSection( const std::string& baseName, UI::PanelSharedPtr& pPanel, UI::TextSharedPtr& pSubtitle )
{
    pPanel = std::make_shared<UI::Panel>( baseName + " panel" );
    GetContentPanel()->Add( pPanel );
    pSubtitle = std::make_shared<UI::Text>( baseName + " subtitle" );
    pPanel->Add( pSubtitle );
}

void ShipStatsWindow::AddEntry( UI::PanelSharedPtr pParentPanel, const std::string& baseName, UI::IconSharedPtr& pIcon, UI::TextSharedPtr& pText, UI::TextSharedPtr& pValue )
{
    pIcon = std::make_shared<UI::Icon>( baseName + " icon" );
    pParentPanel->Add( pIcon );
    pText = std::make_shared<UI::Text>( baseName + " text" );
    pParentPanel->Add( pText );
    pValue = std::make_shared<UI::Text>( baseName + " value" );
    pParentPanel->Add( pValue );
}

} // namespace Hexterminate
