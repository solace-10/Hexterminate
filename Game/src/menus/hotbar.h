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

#pragma once

#include <array>
#include <memory>

#include "../ship/ship.h"
#include <gui/gui.h>

namespace Hexterminate
{

static const unsigned int sMaxAbilityCount = 6;

class EvaWindow;

class Hotbar
{
public:
    Hotbar();
    ~Hotbar();
    void Update( float delta );

private:
    void UpdatePhaseBar( Ship* pPlayerShip );
    void UpdateShieldBar( Ship* pPlayerShip );
    void UpdateAbilities( Ship* pPlayerShip );
    void ResizeBar( Genesis::Gui::Panel* pBar, float ratio, float originalY );

    Genesis::Gui::GuiElement* m_pRoot;
    Genesis::Gui::Image* m_pBackground;
    Genesis::Gui::Image* m_pBackground2;
    Genesis::Gui::Image* m_pPower;
    Genesis::Gui::Image* m_pAddonGui[ sMaxAbilityCount ];
    Genesis::Gui::Text* m_pTextGui[ sMaxAbilityCount ];
    Genesis::Gui::Panel* m_pCooldownGui[ sMaxAbilityCount ];
    Genesis::Gui::Panel* m_pPhaseBarPanel;
    Genesis::Gui::Panel* m_pShieldBarPanel;
    Genesis::Gui::Image* m_pPhaseBar;
    Genesis::Gui::Image* m_pShieldBar;

    struct AbilityData
    {
        std::string key;
        std::string name;
        bool inCooldown;
        bool isActive;
        bool isUsable;
        float cooldownRatio;
        Genesis::ResourceImage* pIcon;
    };

    typedef std::array<AbilityData, sMaxAbilityCount> AbilityDataArray;
    AbilityDataArray m_Abilities;
};

typedef std::unique_ptr<Hotbar> HotbarUniquePtr;

} // namespace Hexterminate
