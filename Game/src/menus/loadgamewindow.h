// Copyright 2021 Pedro Nunes
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

#include <any>
#include <vector>

#include "ui/types.fwd.h"
#include "ui/window.h"

#include "menus/table.h"
#include "menus/tablerow.h"

namespace Hexterminate
{

class ButtonLoadGameSelection;
class ButtonLoad;
class TableRowLoadGame;

using LoadGameSelectionVector = std::vector<ButtonLoadGameSelection*>;

class LoadGameWindow : public UI::Window
{
public:
    LoadGameWindow();
    virtual void Reset() override;

private:
    void CreateTable();

    UI::ScrollingElementSharedPtr m_pScrollingElement;
    Table* m_pTable;
    LoadGameSelectionVector m_Selections;
    bool m_TableDirty;
};

} // namespace Hexterminate
