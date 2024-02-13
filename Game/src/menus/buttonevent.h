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
#include <gui/guieventhandler.h>

namespace Hexterminate
{

class ButtonEvent : public Genesis::Gui::Button
{
public:
    ButtonEvent( Genesis::Gui::GuiEventHandler* pHandler, Genesis::Gui::GuiEvent event );
    virtual void OnPress() override;

private:
    Genesis::Gui::GuiEventHandler* m_pHandler;
    Genesis::Gui::GuiEvent m_Event;
};

} // namespace Hexterminate