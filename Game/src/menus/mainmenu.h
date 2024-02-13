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

#include <array>

#include <gui/gui.h>

#include "ui/button.h"
#include "ui/element.h"
#include "ui/panel.h"
#include "ui/togglegroup.h"
#include "ui/window.h"

namespace Hexterminate
{

class EvaWindow;

enum class MainMenuOption
{
    NewGame,
    LoadGame,
    Settings,
    Codex,
    Credits,
    Quit,
    Count
};

class MainMenu
{
public:
    MainMenu();
    ~MainMenu();
    void Show( bool state );
    void SetOption( MainMenuOption option );

private:
    void OnButtonPressed( const std::any& userData );
    void CreateCharacterImage();
    void CreateVersionText();

    UI::PanelSharedPtr m_pMenu;
    UI::ElementSharedPtr m_pTitle2;
    UI::ToggleGroupSharedPtr m_pToggleGroup;
    UI::ImageSharedPtr m_pCharacterImage;

    EvaWindow* m_pTitle;

    Genesis::Gui::Image* m_pTitleImage;
    Genesis::Gui::Text* m_pVersionText;

    using MenuOptionArray = std::array<UI::ButtonSharedPtr, static_cast<std::size_t>( MainMenuOption::Count )>;
    MenuOptionArray m_Buttons;
    using WindowArray = std::array<UI::WindowSharedPtr, static_cast<std::size_t>( MainMenuOption::Count )>;
    WindowArray m_Windows;
};

} // namespace Hexterminate
