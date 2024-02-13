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

#include "globals.h"
#include <gui/gui.h>

namespace Hexterminate
{

class ModuleInfo;

///////////////////////////////////////////////////////////////////////////////
// ModuleDetails
// Window that shows up in the Shipyard screen when the player picks up a
// module either from his ship or from the inventory.
///////////////////////////////////////////////////////////////////////////////

class ModuleDetails
{
public:
    ModuleDetails();
    virtual ~ModuleDetails();

    void Update( float delta );
    void SetModuleInfo( ModuleInfo* pModuleInfo, bool iconOnly = false );

private:
    void CreateElements();
    void SetTitle();
    void SetIcon();
    void SetContents();
    void PlaceAtCursor();

    ModuleInfo* m_pModuleInfo;

    // These elements compose the full detailed window and are only visible
    // when -not- in "iconOnly" mode.
    Genesis::Gui::Panel* m_pMainPanel;
    Genesis::Gui::Image* m_pIcon;
    Genesis::Gui::Text* m_pTitleText;
    Genesis::Gui::Text* m_pContentsText;

    // This element is only visible when in "iconOnly" mode
    Genesis::Gui::Image* m_pIconOnly;
    bool m_InIconOnlyMode;

    int m_HideNextFrame;
};

} // namespace Hexterminate
