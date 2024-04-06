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

#pragma once

#include <color.h>

#include "ship/moduleinfo.h"
#include "ui/types.fwd.h"
#include "ui/window.h"

namespace Hexterminate
{

/////////////////////////////////////////////////////////////////////
// ShipStatsWindow
/////////////////////////////////////////////////////////////////////

class ShipStatsWindow : public UI::Window
{
public:
    ShipStatsWindow();
    virtual ~ShipStatsWindow();
    virtual void Show( bool state ) override;

    void OnShipConfigurationChanged();

private:
    struct Section
    {
        UI::PanelSharedPtr pPanel;
        UI::TextSharedPtr pSubtitle;    
    };

    struct Entry
    {
        UI::IconSharedPtr pIcon;
        UI::TextSharedPtr pText;
        UI::TextSharedPtr pValue;
    };

    void AddSection( const std::string& baseName, Section& section );
    void AddEntry( Section& section, const std::string& baseName, Entry& entry );

    void UpdatePosition();
    void UpdateWeaponryStats();
    void UpdateShieldStats();
    void UpdateCapacitorStats();
    void UpdateEnergyGridStats();
    void UpdateNavigationStats();
    float CalculateBonusMultiplier( TowerBonus towerBonus ) const;
    void SetEntryColor( Entry& entry, const Genesis::Color& color );

    Section m_Weaponry;
    Entry m_WeaponryKinetic;
    Entry m_WeaponryEnergy;
    Entry m_WeaponryExotic;
    
    Section m_Shields;
    Entry m_ShieldsCapacity;
    Entry m_ShieldsRecharge;
    Entry m_ShieldsEfficiency;

    Section m_Capacitor;
    Entry m_CapacitorCapacity;
    Entry m_CapacitorRecharge;
    Entry m_CapacitorWeapons;
    Entry m_CapacitorAddons;

    Section m_Grid;
    Entry m_GridCapacity;
    Entry m_GridUsed;

    Section m_Navigation;
    Entry m_NavigationLinear;
    Entry m_NavigationAngular;
    Entry m_NavigationMass;
};

} // namespace Hexterminate