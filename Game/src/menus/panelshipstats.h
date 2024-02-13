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

#pragma once

#include "ship/moduleinfo.h"
#include <gui/gui.h>

namespace Hexterminate
{

class Table;
class TableRow;

// Displays the stats for the player ship.
class PanelShipStats
{
public:
    PanelShipStats();
    virtual ~PanelShipStats();
    void Show( bool state );
    void OnShipConfigurationChanged();

private:
    void AddEmptyRow();
    void AddTitleRow( const std::string& title );

    void AddWeaponryRows();
    void AddShieldRows();
    void AddCapacitorRows();
    void AddPowerGridRows();

    void UpdateWeaponryStats();
    void UpdateShieldStats();
    void UpdateReactorStats();

    float CalculateBonusMultiplier( TowerBonus towerBonus ) const;

    Genesis::Gui::Panel* m_pMainPanel;
    Genesis::Gui::Text* m_pTitle;

    Table* m_pTable;

    TableRow* m_pWeaponryTurretRow;
    TableRow* m_pWeaponryFixedRow;
    TableRow* m_pShieldCapacityRow;
    TableRow* m_pShieldRechargeRow;
    TableRow* m_pShieldEfficiencyRow;
    TableRow* m_pEnergyCapacityRow;
    TableRow* m_pEnergyRechargeRow;
    TableRow* m_pWeaponsEnergyUsageRow;
    TableRow* m_pAddonsEnergyUsageRow;
    TableRow* m_pGridRow;
    TableRow* m_pGridUsedRow;
    TableRow* m_pGridAvailableRow;

    float m_PowerGrid;
    float m_PowerGridUsage;
};

} // namespace Hexterminate