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

#include <gui/gui.h>

namespace Hexterminate
{

// A namespace containing a collection of auxiliary functions to reduce boilerplate GUI code.
// All Create* functions create a new element with a consistent look across the game and add it to a parent element.

namespace GuiExtended
{
    Genesis::Gui::Text* CreateText( int x, int y, int w, int h, const std::string& text, Genesis::Gui::GuiElement* pParent, bool bold = false, bool multiline = false );

    // Add the element to a parent -or- to the GuiManager's root if pParent is null
    void AddToParent( Genesis::Gui::GuiElement* pElement, Genesis::Gui::GuiElement* pParent );
} // namespace GuiExtended

} // namespace Hexterminate
