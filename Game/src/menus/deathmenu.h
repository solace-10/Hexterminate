// Copyright 2014 Pedro Nunes
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

#include "menus/eva.h"
#include <gui/gui.h>

namespace Hexterminate
{

class ButtonDeathExit;
class ButtonDeathRespawn;
class SectorInfo;

///////////////////////////////////////////////////////////////////////////////
// DeathMenu
///////////////////////////////////////////////////////////////////////////////

class DeathMenu
{
public:
    DeathMenu();
    ~DeathMenu();

    void Update( float delta );
    void Show( bool state );
    SectorInfo* GetRespawnSector() const;

private:
    void UpdateButtons();
    void AlignToCentre( Genesis::Gui::Text* pText );

    EvaWindow* m_pWindow;
    Genesis::Gui::Text* m_pText;
    ButtonDeathExit* m_pButtonExit;
    ButtonDeathRespawn* m_pButtonRespawn;
    float m_DisplayTimer;
    SectorInfo* m_pRespawnSector;
};

inline SectorInfo* DeathMenu::GetRespawnSector() const
{
    return m_pRespawnSector;
}

///////////////////////////////////////////////////////////////////////////////
// ButtonDeathExit
///////////////////////////////////////////////////////////////////////////////

class ButtonDeathExit : public Genesis::Gui::Button
{
public:
    ButtonDeathExit();
    virtual ~ButtonDeathExit() override{};
    virtual void OnPress() override;
};

///////////////////////////////////////////////////////////////////////////////
// ButtonDeathRespawn
///////////////////////////////////////////////////////////////////////////////

class ButtonDeathRespawn : public Genesis::Gui::Button
{
public:
    ButtonDeathRespawn( DeathMenu* pOwner );
    virtual ~ButtonDeathRespawn() override{};
    virtual void OnPress() override;

private:
    DeathMenu* m_pOwner;
};

} // namespace Hexterminate
