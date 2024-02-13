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

#include <gui/gui.h>

#include "gameevents.h"
#include "ui/types.fwd.h"
#include "ui/window.h"

namespace Hexterminate
{

class ButtonFleet;
class ButtonPerks;
class EndGameWindow;
class FleetWindow;
using FleetWindowSharedPtr = std::shared_ptr<FleetWindow>;
class PerksWindow;

/////////////////////////////////////////////////////////////////////
// GalaxyWindow
/////////////////////////////////////////////////////////////////////

class GalaxyWindow : public UI::Window, public GameEventHandler
{
public:
    GalaxyWindow();
    virtual ~GalaxyWindow();

    virtual void HandleGameEvent( GameEvent* pEvent ) override;

    void Show( bool state ) override;

    virtual void Update() override;

    void ShowFleetWindow( bool state );
    void ShowPerksWindow( bool state );
    void ShowEndGameWindow();

    bool IsFleetWindowVisible() const;
    bool IsPerksWindowVisible() const;
    bool IsEndGameWindowVisible() const;
    bool HasSubWindowsVisible() const;

    FleetWindow* GetFleetWindow() const;
    PerksWindow* GetPerksWindow() const;

private:
    void UpdatePerksText();
    void UpdateInfluenceText();
    void UpdateStardateText();

    UI::TextSharedPtr m_pStardateText;
    UI::TextSharedPtr m_pInfluenceText;
    UI::TextSharedPtr m_pPerksText;
    UI::ButtonSharedPtr m_pFleetsButton;
    UI::ButtonSharedPtr m_pPerksButton;
    UI::ImageSharedPtr m_pStardateImage;
    UI::ImageSharedPtr m_pInfluenceImage;
    UI::ImageSharedPtr m_pPerksImage;

    FleetWindowSharedPtr m_pFleetWindow;
    PerksWindow* m_pPerksWindow;
    EndGameWindow* m_pEndGameWindow;
};

inline FleetWindow* GalaxyWindow::GetFleetWindow() const
{
    return m_pFleetWindow.get();
}

inline PerksWindow* GalaxyWindow::GetPerksWindow() const
{
    return m_pPerksWindow;
}

} // namespace Hexterminate