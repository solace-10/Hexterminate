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

#ifdef _DEBUG

#include "faction/faction.h"
#include "menus/table.h"
#include "menus/tablerow.h"
#include "ship/moduleinfo.h"

#include <gui/gui.h>

namespace Hexterminate
{

class EvaWindow;
class ButtonSaveHexGrid;
class ButtonFactionSelect;
class TableRow;
class ModuleInfo;

///////////////////////////////////////////////////////////////////////////////
// PanelShipyardDebug
///////////////////////////////////////////////////////////////////////////////

class PanelShipyardDebug
{
public:
    PanelShipyardDebug();
    ~PanelShipyardDebug();
    void Update( float delta );

    const std::string& GetLoadedFilename() const;
    void SetLoadedFilename( const std::string& filename );

    void SetFactionFilter( FactionId faction );

private:
    void CreateFactionSelectButtons();
    void CreateTableWindow();
    void CreateTable( FactionId factionId );
    void CreateButtonSave();

    EvaWindow* m_pTableWindow;
    Genesis::Gui::Text* m_pTableTitle;
    Table* m_pTable[ static_cast<unsigned int>( FactionId::Count ) ];
    ButtonSaveHexGrid* m_pButtonSave;
    ButtonFactionSelect* m_pButtonFactionSelect[ static_cast<unsigned int>( FactionId::Count ) ];
    FactionId m_SelectedFaction;
};

///////////////////////////////////////////////////////////////////////////////
// ButtonFactionSelect
///////////////////////////////////////////////////////////////////////////////

class ButtonFactionSelect : public Genesis::Gui::Button
{
public:
    ButtonFactionSelect( PanelShipyardDebug* pOwner, FactionId factionId );
    virtual ~ButtonFactionSelect() override{};
    virtual void OnPress() override;

private:
    PanelShipyardDebug* m_pOwner;
    FactionId m_FactionId;
};

///////////////////////////////////////////////////////////////////////////////
// ButtonSaveHexGrid
///////////////////////////////////////////////////////////////////////////////

class ButtonSaveHexGrid : public Genesis::Gui::Button
{
public:
    ButtonSaveHexGrid( PanelShipyardDebug* pOwner );
    virtual ~ButtonSaveHexGrid() override{};
    virtual void OnPress() override;

private:
    PanelShipyardDebug* m_pOwner;
};

///////////////////////////////////////////////////////////////////////////////
// TableRowHexGrid
///////////////////////////////////////////////////////////////////////////////

class TableRowHexGrid : public TableRow
{
public:
    TableRowHexGrid( PanelShipyardDebug* pOwner, const std::string& filename );
    virtual ~TableRowHexGrid() override{};
    virtual void OnPress() override;

private:
    PanelShipyardDebug* m_pOwner;
    std::string m_Filename;
};

} // namespace Hexterminate

#endif
