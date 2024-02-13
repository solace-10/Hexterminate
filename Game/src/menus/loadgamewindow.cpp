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

#include <functional>

#include "menus/loadgamewindow.h"

#include "menus/table.h"
#include "menus/tablerow.h"

#include "savegameheader.h"
#include "savegamestorage.h"
#include "stringaux.h"
#include "ui/button.h"
#include "ui/checkbox.h"
#include "ui/fonts.h"
#include "ui/image.h"
#include "ui/inputarea.h"
#include "ui/meter.h"
#include "ui/panel.h"
#include "ui/radiobutton.h"
#include "ui/scrollingelement.h"
#include "ui/text.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// TableRowLoadGame
///////////////////////////////////////////////////////////////////////////////

class TableRowLoadGame : public TableRow
{
public:
    TableRowLoadGame( SaveGameHeaderWeakPtr pSaveGameHeader );
    virtual void OnPress() override;

private:
    SaveGameHeaderWeakPtr m_pSaveGameHeader;
};

TableRowLoadGame::TableRowLoadGame( SaveGameHeaderWeakPtr pSaveGameHeader )
    : m_pSaveGameHeader( pSaveGameHeader )
{
}

void TableRowLoadGame::OnPress()
{
    TableRow::OnPress();
    g_pGame->LoadGame( m_pSaveGameHeader );
}

///////////////////////////////////////////////////////////////////////////////
// LoadGameWindow
///////////////////////////////////////////////////////////////////////////////

LoadGameWindow::LoadGameWindow()
    : UI::Window( "Load game window" )
    , m_pTable( nullptr )
{
    m_pScrollingElement = std::make_shared<UI::ScrollingElement>( "Scrolling element" );
    GetContentPanel()->Add( m_pScrollingElement );

    CreateTable();
}

void LoadGameWindow::Reset()
{
    CreateTable();
}

void LoadGameWindow::CreateTable()
{
    if ( m_pTable != nullptr )
    {
        m_pScrollingElement->GetScrollingArea()->GetPanel()->RemoveElement( m_pTable );
        m_pTable = nullptr;
    }

    const float width = 800.0f - 16.0f;

    m_pTable = new Table();
    m_pTable->Show( true );
    m_pScrollingElement->GetScrollingArea()->GetPanel()->AddElement( m_pTable );

    TableRow* pTitleRow = new TableRow();
    pTitleRow->SetFont( UI::Fonts::Get( "kimberley18.fnt" ) );
    pTitleRow->Add( "Ship" );
    pTitleRow->Add( "Commander" );
    pTitleRow->Add( "Difficulty" );
    pTitleRow->Add( "Game mode" );
    pTitleRow->Add( "Time played" );
    m_pTable->AddRow( pTitleRow );

    SaveGameHeaderVector saveGameHeaders;
    g_pGame->GetSaveGameStorage()->GetSaveGameHeaders( saveGameHeaders );
    Genesis::ResourceFont* pFont = UI::Fonts::Get( "kimberley18light.fnt" );
    for ( auto& pSaveGameHeader : saveGameHeaders )
    {
        TableRowLoadGame* pRow = new TableRowLoadGame( pSaveGameHeader );
        pRow->SetFont( pFont );
        pRow->Add( pSaveGameHeader->GetShipName() );
        pRow->Add( pSaveGameHeader->GetCaptainName() );
        pRow->Add( ToString( pSaveGameHeader->GetDifficulty() ) );
        pRow->Add( ToString( pSaveGameHeader->GetGameMode() ) );
        pRow->Add( ToStringTime( pSaveGameHeader->GetPlayedTime() ) );

        if ( pSaveGameHeader->GetDifficulty() == Difficulty::Hardcore )
        {
            if ( pSaveGameHeader->IsAlive() )
            {
                pRow->SetColour( Genesis::Color( 0.8f, 0.0f, 0.0f, 1.0f ) );
            }
            else
            {
                pRow->SetColour( Genesis::Color( 0.6f, 0.0f, 0.0f, 0.6f ) );
            }
        }

        m_pTable->AddRow( pRow );
    }

    m_pTable->SetSize( width, m_pTable->GetRowHeight() * m_pTable->GetRowCount() );
    m_TableDirty = false;
}

} // namespace Hexterminate
