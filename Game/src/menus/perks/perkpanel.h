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

#include <string>
#include <vector>

#include <gui/gui.h>

#include "perks.h"

namespace Hexterminate
{

class PerkButton;
class PerkCategoryPanel;
class PerkPanel;
class PerkTooltip;

enum class PerkState
{
    Enabled,
    Locked,
    Disabled
};

typedef std::vector<PerkPanel*> PerkPanelArray;

/////////////////////////////////////////////////////////////////////
// PerkPanel
// A panel representing a single perk.
/////////////////////////////////////////////////////////////////////

class PerkPanel : public Genesis::Gui::Panel
{
public:
    PerkPanel();
    virtual ~PerkPanel();
    virtual void Update( float delta ) override;
    virtual void Show( bool state ) override;

    void Init( Perk perk, const std::string& name, const std::string& description, const std::string& icon, int cost, PerkState state );

    Perk GetPerk() const;
    void SetState( PerkState state );

    void SetPendingPurchase();

private:
    void AlignToCentre( Genesis::Gui::Text* pText );

    Genesis::Gui::Text* m_pTitle;
    PerkButton* m_pIcon;
    Genesis::Gui::Text* m_pCost;
    int m_Cost;
    Perk m_Perk;
    PerkTooltip* m_pTooltip;
    bool m_PendingPurchase;
};

inline Perk PerkPanel::GetPerk() const
{
    return m_Perk;
}

/////////////////////////////////////////////////////////////////////
// PerkButton
// The button the player presses to acquire a perk
/////////////////////////////////////////////////////////////////////

class PerkButton : public Genesis::Gui::ButtonImage
{
public:
    PerkButton( PerkPanel* pOwner, Perk perk, int cost );
    virtual void OnPress() override;
    void SetEnabled( bool state );

private:
    PerkPanel* m_pOwner;
    int m_Cost;
    bool m_Enabled;
};

inline void PerkButton::SetEnabled( bool state )
{
    m_Enabled = state;
}

} // namespace Hexterminate
