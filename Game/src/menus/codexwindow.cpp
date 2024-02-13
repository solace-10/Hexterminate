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

#include <filesystem>
#include <fstream>

#include "menus/codexwindow.h"

#include "json.hpp"
#include "savegameheader.h"
#include "savegamestorage.h"
#include "ui/button.h"
#include "ui/fonts.h"
#include "ui/image.h"
#include "ui/panel.h"
#include "ui/scrollingelement.h"
#include "ui/text.h"
#include "ui/togglegroup.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// CodexWindow
///////////////////////////////////////////////////////////////////////////////

CodexWindow::CodexWindow()
    : UI::Window( "Codex window" )
{
    m_pCategoriesPanel = std::make_shared<UI::Panel>( "Categories panel" );
    GetContentPanel()->Add( m_pCategoriesPanel );
    m_pCategoriesArea = std::make_shared<UI::ScrollingElement>( "Categories area" );
    m_pCategoriesPanel->Add( m_pCategoriesArea );

    m_pContentPanel = std::make_shared<UI::Panel>( "Content panel" );
    GetContentPanel()->Add( m_pContentPanel );

    m_pContentText = std::make_shared<UI::Text>( "Content text" );
    m_pContentPanel->Add( m_pContentText );

    m_pModuleWireframe = std::make_shared<UI::Image>( "Module wireframe", "data/codex/modulewireframe.png" );
    m_pContentPanel->Add( m_pModuleWireframe );

    m_pEntryToggleGroup = std::make_shared<UI::ToggleGroup>();

    LoadEntries();
    RefreshUI();
}

void CodexWindow::ShowEntry( const std::string& content )
{
    m_pModuleWireframe->Show( false );
    m_pContentText->SetText( content );
}

void CodexWindow::LoadEntries()
{
    for ( const auto& entry : std::filesystem::directory_iterator( "data/codex" ) )
    {
        if ( entry.path().extension() == ".json" && std::filesystem::file_size( entry ) > 0 )
        {
            LoadEntry( entry );
        }
    }
}

void CodexWindow::LoadEntry( const std::filesystem::path& path )
{
    using namespace nlohmann;
    json data;
    std::ifstream file( path, std::ios::in );
    if ( file.is_open() )
    {
        file >> data;
        file.close();

        static const std::string sName( "name" );
        static const std::string sCategory( "category" );
        static const std::string sContent( "content" );
        if ( data.contains( sName ) && data.contains( sCategory ) && data.contains( sContent ) )
        {
            const std::string& entryName = data[ sName ].get<std::string>();
            const std::string& categoryName = data[ sCategory ].get<std::string>();
            const std::string& content = data[ sContent ].get<std::string>();
            AddEntry( categoryName, entryName, content );
        }
        else
        {
            Genesis::FrameWork::GetLogger()->LogWarning( "Couldn't load codex entry '%s'.", path.c_str() );
        }
    }
}

void CodexWindow::RefreshUI()
{
    const int buttonHeight = 30;
    const int categoryGap = 16;
    const int entryGap = 4;
    const int xOffset = 8;
    int yOffset = 8;

    for ( auto& category : m_Categories )
    {
        category.second.GetButton()->SetPosition( xOffset, yOffset );
        yOffset += buttonHeight;

        CodexEntryVector entries = category.second.GetEntries();
        if ( entries.size() > 0 )
        {
            yOffset += entryGap;
            for ( auto& entry : entries )
            {
                entry->GetButton()->SetPosition( xOffset, yOffset );
                yOffset += buttonHeight + entryGap;
            }

            yOffset += categoryGap - entryGap;
        }
        else
        {
            yOffset += categoryGap;
        }
    }
}

void CodexWindow::AddEntry( const std::string& categoryName, const std::string& entryName, const std::string& content )
{
    GetCategory( categoryName ).AddEntry( entryName, content, m_pCategoriesArea->GetScrollingArea(), this, m_pEntryToggleGroup );
}

CodexCategory& CodexWindow::GetCategory( const std::string& name )
{
    Categories::iterator it = m_Categories.find( name );
    if ( it == m_Categories.end() )
    {
        m_Categories[ name ] = CodexCategory( name, m_pCategoriesArea->GetScrollingArea() );
        return m_Categories[ name ];
    }
    else
    {
        return it->second;
    }
}

///////////////////////////////////////////////////////////////////////////////
// CodexCategory
///////////////////////////////////////////////////////////////////////////////

CodexCategory::CodexCategory()
{
}

CodexCategory::CodexCategory( const std::string& name, UI::Element* pParent )
{
    m_pButton = std::make_shared<UI::Button>( name, nullptr );
    m_pButton->SetDynamic( true );
    m_pButton->SetDefaultImage( "data/ui/widgets/togglebutton.png" );
    m_pButton->SetHoveredImage( "data/ui/widgets/togglebutton_hover.png" );
    m_pButton->SetPressedImage( "data/ui/widgets/togglebutton_selected.png" );
    m_pButton->SetText( name, false, true );
    pParent->Add( m_pButton );
}

UI::Button* CodexCategory::GetButton() const
{
    return m_pButton.get();
}

void CodexCategory::AddEntry( const std::string& name, const std::string& content, UI::Element* pParent, CodexWindow* pCodexWindow, UI::ToggleGroupSharedPtr pEntryToggleGroup )
{
    m_Entries[ name ] = std::make_unique<CodexEntry>( name, content, pParent, pCodexWindow, pEntryToggleGroup );
}

CodexEntryVector CodexCategory::GetEntries()
{
    CodexEntryVector entries;
    for ( auto& entryPair : m_Entries )
    {
        entries.push_back( entryPair.second.get() );
    }
    return entries;
}

///////////////////////////////////////////////////////////////////////////////
// CodexEntry
///////////////////////////////////////////////////////////////////////////////

CodexEntry::CodexEntry()
    : m_pButton( nullptr )
    , m_pCodexWindow( nullptr )
{
}

CodexEntry::CodexEntry( const std::string& name, const std::string& content, UI::Element* pParent, CodexWindow* pCodexWindow, UI::ToggleGroupSharedPtr pEntryToggleGroup )
    : m_Content( content )
    , m_pCodexWindow( pCodexWindow )
{
    m_pButton = std::make_shared<UI::Button>(
        name, [ this ]( std::any userData ) { OnButtonPressed(); }, nullptr, pEntryToggleGroup );
    m_pButton->SetDynamic( true );
    m_pButton->SetDefaultImage( "data/ui/widgets/stackbutton.png" );
    m_pButton->SetHoveredImage( "data/ui/widgets/stackbutton_hover.png" );
    m_pButton->SetPressedImage( "data/ui/widgets/stackbutton_selected.png" );
    m_pButton->SetFont( "kimberley18light.fnt" );
    m_pButton->SetText( name, false, true );
    pParent->Add( m_pButton );
    pEntryToggleGroup->Add( m_pButton );
}

UI::Button* CodexEntry::GetButton() const
{
    return m_pButton.get();
}

void CodexEntry::OnButtonPressed()
{
    m_pCodexWindow->ShowEntry( m_Content );
}

} // namespace Hexterminate
