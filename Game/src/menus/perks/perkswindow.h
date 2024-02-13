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
#include <gui/guieventhandler.h>
#include <gui/scrollingelement.h>

#include "menus/perks/perkcategorypanel.h"

namespace Hexterminate
{

class ButtonEvent;

class PerksWindow : public Genesis::Gui::GuiEventHandler
{
public:
    PerksWindow();
    virtual ~PerksWindow();

    virtual bool HandleEvent( Genesis::Gui::GuiEvent event ) override;

    void Init();
    void Show( bool state );
    bool IsVisible() const;

private:
    void InitCategories();
    PerkCategoryPanel* AddCategory( const std::string& category, const std::string& backgroundImage );

    Genesis::Gui::Panel* m_pMainPanel;
    ButtonEvent* m_pButtonClose;
    Genesis::Gui::Text* m_pTitle;
    Genesis::Gui::ScrollingElement* m_pScrollingElement;
    PerkCategoryPanelArray m_Categories;
};

} // namespace Hexterminate