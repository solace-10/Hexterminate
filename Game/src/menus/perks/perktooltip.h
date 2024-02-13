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

#include <gui/gui.h>

#include "menus/perks/perkpanel.h"

namespace Hexterminate
{

class PerkTooltip
{
public:
    PerkTooltip();
    ~PerkTooltip();

    void Init( const std::string& name, const std::string& description, unsigned int cost );
    void Show( bool state );
    void SetState( PerkState state );

private:
    Genesis::Gui::Panel* m_pMainPanel;
    Genesis::Gui::Text* m_pTitle;
    Genesis::Gui::Text* m_pState;
    Genesis::Gui::Text* m_pDescription;
    Genesis::Gui::Text* m_pCost;
    Genesis::Gui::Image* m_pCostIcon;
    int m_Cost;
    PerkState m_PerkState;
};

} // namespace Hexterminate