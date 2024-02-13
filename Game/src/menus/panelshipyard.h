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
#include <gui/scrollingelement.h>

#include "menus/table.h"
#include "menus/tablerow.h"
#include "ship/moduleinfo.h"

namespace Hexterminate
{

class EvaWindow;
class ButtonModule;
class ButtonUndock;
class TableRowInventory;
class ModuleInfo;

///////////////////////////////////////////////////////////////////////////////
// PanelShipyard
// Window that contains all the UI related with customising a ship in a
// shipyard.
///////////////////////////////////////////////////////////////////////////////

class PanelShipyard
{
public:
    PanelShipyard();
    ~PanelShipyard();
    void Update( float delta );
    void SetSelectedGroup( ModuleType module );
    void UpdateModule( ModuleInfo* pModuleInfo );

private:
    void CreateTableWindow();
    void CreateButtonUndock();

    TableRow* CreateCommonRow() const;
    void CreateTable( ModuleType type, int columns, ... );
    void SetTitleFromGroup( ModuleType type );

    void FillTables();

    EvaWindow* m_pTableWindow;
    Genesis::Gui::Text* m_pTableTitle;
    ButtonModule* m_pIcons[ static_cast<unsigned int>( ModuleType::Count ) ];
    Table* m_pTable[ static_cast<unsigned int>( ModuleType::Count ) ];

    ModuleType m_SelectedGroup;

    ButtonUndock* m_pButtonUndock;
    Genesis::Gui::ScrollingElement* m_pScrollingElement;
};

///////////////////////////////////////////////////////////////////////////////
// ButtonModule
// PanelShipyard contains a row of icons (one for each module type) that
// the player can click. This is done with a number of ButtonImages with
// custom behaviour.
///////////////////////////////////////////////////////////////////////////////

class ButtonModule : public Genesis::Gui::ButtonImage
{
public:
    ButtonModule( PanelShipyard* pOwner, ModuleType moduleType );
    virtual ~ButtonModule() override{};
    virtual void OnPress() override;

private:
    PanelShipyard* m_pOwner;
    ModuleType m_ModuleType;
};

///////////////////////////////////////////////////////////////////////////////
// ButtonUndock
// Undocks the ship from the shipyard and closes the editing interface.
///////////////////////////////////////////////////////////////////////////////

class ButtonUndock : public Genesis::Gui::Button
{
public:
    ButtonUndock();
    virtual ~ButtonUndock() override{};
    virtual void OnPress() override;
};

///////////////////////////////////////////////////////////////////////////////
// TableRowInventory
///////////////////////////////////////////////////////////////////////////////

class TableRowInventory : public TableRow
{
public:
    TableRowInventory( PanelShipyard* pOwner, ModuleInfo* pModuleInfo );
    virtual ~TableRowInventory() override{};
    virtual void OnPress() override;
    virtual void OnHover() override;

private:
    ModuleInfo* m_pModuleInfo;
};

} // namespace Hexterminate
