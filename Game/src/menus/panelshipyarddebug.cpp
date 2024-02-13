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

#ifdef _DEBUG

#include "menus/panelshipyarddebug.h"
#include "globals.h"
#include "hexterminate.h"
#include "menus/eva.h"
#include "menus/table.h"
#include "menus/tablerow.h"
#include "player.h"
#include "ship/inventory.h"
#include "shipyard/shipyard.h"
#include "stringaux.h"

#include <configuration.h>
#include <genesis.h>
#include <gui/gui.h>

#include <filesystem>
#include <sstream>

namespace Hexterminate
{

class HexGridTableRow;

///////////////////////////////////////////////////////////////////////////
// PanelShipyardDebug
///////////////////////////////////////////////////////////////////////////

static std::string sLoadedFilename = "";

PanelShipyardDebug::PanelShipyardDebug()
    : m_pTableWindow( nullptr )
    , m_pTableTitle( nullptr )
    , m_pButtonSave( nullptr )
    , m_SelectedFaction( FactionId::Neutral )
{
    CreateTableWindow();
    CreateFactionSelectButtons();
    CreateButtonSave();
}

PanelShipyardDebug::~PanelShipyardDebug()
{
    delete m_pTableWindow;

    Genesis::FrameWork::GetGuiManager()->RemoveElement( m_pButtonSave );

    for ( int i = 0; i < (int)FactionId::Count; ++i )
    {
        Genesis::FrameWork::GetGuiManager()->RemoveElement( m_pButtonFactionSelect[ i ] );
    }
}

void PanelShipyardDebug::CreateFactionSelectButtons()
{
    float x = 674.0f;
    float w = 64.0f;
    for ( int i = 0; i < (int)FactionId::Count; ++i )
    {
        m_pButtonFactionSelect[ i ] = new ButtonFactionSelect( this, (FactionId)i );
        m_pButtonFactionSelect[ i ]->SetPosition( x, 32.0f );
        m_pButtonFactionSelect[ i ]->SetSize( w, w );
        m_pButtonFactionSelect[ i ]->SetColour( EVA_BUTTON_COLOUR_BACKGROUND );
        m_pButtonFactionSelect[ i ]->SetHoverColour( EVA_BUTTON_COLOUR_HOVER );
        m_pButtonFactionSelect[ i ]->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
        m_pButtonFactionSelect[ i ]->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
        m_pButtonFactionSelect[ i ]->SetFont( EVA_FONT );
        m_pButtonFactionSelect[ i ]->SetText( g_pGame->GetFaction( (FactionId)i )->GetName() );
        Genesis::FrameWork::GetGuiManager()->AddElement( m_pButtonFactionSelect[ i ] );

        x += w + 8.0f;
    }
}

void PanelShipyardDebug::CreateButtonSave()
{
    unsigned int screenWidth = Genesis::Configuration::GetScreenWidth();

    m_pButtonSave = new ButtonSaveHexGrid( this );
    m_pButtonSave->SetPosition( 666.0f, (float)Genesis::Configuration::GetScreenHeight() - 40.0f );
    m_pButtonSave->SetSize( (float)( screenWidth - 666 - 8 ), 32.0f );
    m_pButtonSave->SetColour( EVA_BUTTON_COLOUR_BACKGROUND );
    m_pButtonSave->SetHoverColour( EVA_BUTTON_COLOUR_HOVER );
    m_pButtonSave->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
    m_pButtonSave->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
    m_pButtonSave->SetFont( EVA_FONT );
    m_pButtonSave->SetText( "Save hexgrid" );
    Genesis::FrameWork::GetGuiManager()->AddElement( m_pButtonSave );
}

void PanelShipyardDebug::CreateTableWindow()
{
    unsigned int screenWidth = Genesis::Configuration::GetScreenWidth();
    unsigned int screenHeight = Genesis::Configuration::GetScreenHeight();

    m_pTableWindow = new EvaWindow( 666, 8, screenWidth - 666 - 8, screenHeight - 56, false );

    m_pTableTitle = new Genesis::Gui::Text();
    m_pTableTitle->SetSize( 256, 32 );
    m_pTableTitle->SetPosition( 8.0f, 8.0f );
    m_pTableTitle->SetColour( EVA_TEXT_COLOUR );
    m_pTableTitle->SetFont( EVA_FONT );

    if ( sLoadedFilename != "" )
    {
        std::stringstream ss;
        ss << "Shipyard > Hexgrid loaded (" << sLoadedFilename << ")";
        m_pTableTitle->SetText( ss.str() );
    }
    else
    {
        m_pTableTitle->SetText( "Shipyard > Hexgrid loaded" );
    }
    m_pTableWindow->GetMainPanel()->AddElement( m_pTableTitle );

    for ( int i = 0; i < (int)FactionId::Count; ++i )
    {
        CreateTable( (FactionId)i );
    }
}

void PanelShipyardDebug::CreateTable( FactionId factionId )
{
    unsigned int screenWidth = Genesis::Configuration::GetScreenWidth();
    unsigned int screenHeight = Genesis::Configuration::GetScreenHeight();

    int idx = (int)factionId;

    m_pTable[ idx ] = new Table();
    m_pTable[ idx ]->Show( true );
    m_pTable[ idx ]->SetPosition( 8.0f, 96.0f );
    m_pTable[ idx ]->SetSize( (float)( screenWidth - 666 - 24 ), screenHeight - 8.0f );
    m_pTable[ idx ]->Show( factionId == m_SelectedFaction );
    m_pTableWindow->GetMainPanel()->AddElement( m_pTable[ idx ] );

    TableRow* pTitleRow = new TableRow();
    pTitleRow->Add( "Filename" );

    m_pTable[ idx ]->AddRow( pTitleRow );

    std::filesystem::path shipsPath = "data/xml/ships";
    std::filesystem::path directory = shipsPath / ToLower( g_pGame->GetFaction( factionId )->GetName() );
    for ( const auto& filename : std::filesystem::directory_iterator( directory ) )
    {
        if ( filename.path().extension() == ".shp" )
        {
            TableRowHexGrid* pFilenameRow = new TableRowHexGrid( this, filename.path().generic_string() );
            pFilenameRow->Add( filename.path().generic_string() );
            m_pTable[ idx ]->AddRow( pFilenameRow );
        }
    }
}

const std::string& PanelShipyardDebug::GetLoadedFilename() const
{
    return sLoadedFilename;
}

void PanelShipyardDebug::SetLoadedFilename( const std::string& filename )
{
    sLoadedFilename = filename;

    std::stringstream ss;
    ss << "Shipyard > Hexgrid loaded (" << sLoadedFilename << ")";
    m_pTableTitle->SetText( ss.str() );
}

void PanelShipyardDebug::SetFactionFilter( FactionId faction )
{
    m_SelectedFaction = faction;
    for ( int i = 0; i < (int)FactionId::Count; ++i )
    {
        m_pTable[ i ]->Show( (int)faction == i );
    }
}

///////////////////////////////////////////////////////////////////////////////
// ButtonFactionSelect
///////////////////////////////////////////////////////////////////////////////

ButtonFactionSelect::ButtonFactionSelect( PanelShipyardDebug* pOwner, FactionId factionId )
    : m_pOwner( pOwner )
    , m_FactionId( factionId )
{
}

void ButtonFactionSelect::OnPress()
{
    m_pOwner->SetFactionFilter( m_FactionId );
}

///////////////////////////////////////////////////////////////////////////////
// ButtonSaveHexGrid
///////////////////////////////////////////////////////////////////////////////

ButtonSaveHexGrid::ButtonSaveHexGrid( PanelShipyardDebug* pOwner )
    : m_pOwner( pOwner )
{
}

void ButtonSaveHexGrid::OnPress()
{
    Shipyard* pShipyard = g_pGame->GetPlayer()->GetShip()->GetShipyard();
    if ( pShipyard != nullptr && m_pOwner->GetLoadedFilename() != "" )
    {
        pShipyard->SaveToFile( m_pOwner->GetLoadedFilename() );
    }
}

///////////////////////////////////////////////////////////////////////////////
// HexGridTableRow
///////////////////////////////////////////////////////////////////////////////

TableRowHexGrid::TableRowHexGrid( PanelShipyardDebug* pOwner, const std::string& filename )
    : m_pOwner( pOwner )
    , m_Filename( filename )
{
}

void TableRowHexGrid::OnPress()
{
    Shipyard* pShipyard = g_pGame->GetPlayer()->GetShip()->GetShipyard();
    if ( pShipyard != nullptr )
    {
        pShipyard->LoadFromFile( m_Filename );
        m_pOwner->SetLoadedFilename( m_Filename );
    }
}

} // namespace Hexterminate

#endif