// Copyright 2018 Pedro Nunes
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
#include <gui/guieventhandler.h>
#include <gui/scrollingelement.h>

namespace Hexterminate
{

class ButtonEvent;
class ButtonReturnToMenu;

class EndGameWindow : public Genesis::Gui::GuiEventHandler
{
public:
    EndGameWindow();
    virtual ~EndGameWindow();

    virtual bool HandleEvent( Genesis::Gui::GuiEvent event ) override;

    void Init();
    void Show( bool state );
    bool IsVisible() const;

private:
    Genesis::Gui::Image* m_pMainPanel;
    ButtonEvent* m_pButtonClose;
    Genesis::Gui::Text* m_pTitle;
    ButtonEvent* m_pButtonResume;
    ButtonReturnToMenu* m_pButtonReturnToMenu;
};

/////////////////////////////////////////////////////////////////////
// ButtonReturnToMenu
/////////////////////////////////////////////////////////////////////

class ButtonReturnToMenu : public Genesis::Gui::Button
{
public:
    virtual void OnPress() override;
};

} // namespace Hexterminate