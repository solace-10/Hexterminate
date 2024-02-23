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

private:
    void AddSection( const std::string& baseName, UI::PanelSharedPtr& pPanel, UI::TextSharedPtr& pSubtitle );
    void AddEntry( UI::PanelSharedPtr pParentPanel, const std::string& baseName, UI::IconSharedPtr& pIcon, UI::TextSharedPtr& pText, UI::TextSharedPtr& pValue );

    UI::PanelSharedPtr m_pWeaponsPanel;
    UI::TextSharedPtr m_pWeaponsSubtitle;
    UI::PanelSharedPtr m_pShieldsPanel;
    UI::TextSharedPtr m_pShieldsSubtitle;
    UI::PanelSharedPtr m_pCapacitorPanel;
    UI::TextSharedPtr m_pCapacitorSubtitle;
    UI::PanelSharedPtr m_pEnergyMatrixPanel;
    UI::TextSharedPtr m_pEnergyMatrixSubtitle;

    UI::IconSharedPtr m_pWeaponryKineticIcon;
    UI::TextSharedPtr m_pWeaponryKineticText;
    UI::TextSharedPtr m_pWeaponryKineticValue;
    UI::IconSharedPtr m_pWeaponryEnergyIcon;
    UI::TextSharedPtr m_pWeaponryEnergyText;
    UI::TextSharedPtr m_pWeaponryEnergyValue;
    UI::IconSharedPtr m_pWeaponryExoticIcon;
    UI::TextSharedPtr m_pWeaponryExoticText;
    UI::TextSharedPtr m_pWeaponryExoticValue;
};

} // namespace Hexterminate