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

#include <sstream>

#include "hexterminate.h"
#include "menus/codexwindow.h"
#include "menus/creditswindow.h"
#include "menus/eva.h"
#include "menus/loadgamewindow.h"
#include "menus/mainmenu.h"
#include "menus/newgamewindow.h"
#include "menus/popup.h"
#include "menus/settingswindow.h"
#include "misc/gui.h"
#include "ui/container.h"
#include "ui/fonts.h"
#include "ui/image.h"
#include "ui/rootelement.h"
#include <configuration.h>
#include <genesis.h>
#include <gui/gui.h>

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////
// MainMenu
///////////////////////////////////////////////////////////////////////////

MainMenu::MainMenu()
    : m_pTitle( nullptr )
    , m_pTitleImage( nullptr )
    , m_pVersionText( nullptr )
{
    CreateCharacterImage();

    m_pMenu = std::make_shared<UI::Panel>( "Main menu" );
    g_pGame->GetUIRoot( UIDesignId::MainMenu )->Add( m_pMenu );

    m_pTitle2 = std::make_shared<UI::Element>( "Title" );
    g_pGame->GetUIRoot( UIDesignId::MainMenu )->Add( m_pTitle2 );

    m_pToggleGroup = std::make_shared<UI::ToggleGroup>();

    using namespace std::placeholders;
    m_Buttons[ static_cast<std::size_t>( MainMenuOption::NewGame ) ] = std::make_shared<UI::Button>( "New game button", std::bind( &MainMenu::OnButtonPressed, this, _1 ), MainMenuOption::NewGame, m_pToggleGroup );
    m_Buttons[ static_cast<std::size_t>( MainMenuOption::LoadGame ) ] = std::make_shared<UI::Button>( "Load game button", std::bind( &MainMenu::OnButtonPressed, this, _1 ), MainMenuOption::LoadGame, m_pToggleGroup );
    m_Buttons[ static_cast<std::size_t>( MainMenuOption::Settings ) ] = std::make_shared<UI::Button>( "Settings button", std::bind( &MainMenu::OnButtonPressed, this, _1 ), MainMenuOption::Settings, m_pToggleGroup );
    m_Buttons[ static_cast<std::size_t>( MainMenuOption::Codex ) ] = std::make_shared<UI::Button>( "Codex button", std::bind( &MainMenu::OnButtonPressed, this, _1 ), MainMenuOption::Codex, m_pToggleGroup );
    m_Buttons[ static_cast<std::size_t>( MainMenuOption::Credits ) ] = std::make_shared<UI::Button>( "Credits button", std::bind( &MainMenu::OnButtonPressed, this, _1 ), MainMenuOption::Credits, m_pToggleGroup );
    m_Buttons[ static_cast<std::size_t>( MainMenuOption::Quit ) ] = std::make_shared<UI::Button>( "Quit button", std::bind( &MainMenu::OnButtonPressed, this, _1 ), MainMenuOption::Quit, m_pToggleGroup );

    for ( auto& pButton : m_Buttons )
    {
        m_pMenu->Add( pButton );
        m_pToggleGroup->Add( pButton );
    }

    m_Windows[ static_cast<std::size_t>( MainMenuOption::NewGame ) ] = std::make_shared<NewGameWindow>();
    m_Windows[ static_cast<std::size_t>( MainMenuOption::LoadGame ) ] = std::make_shared<LoadGameWindow>();
    m_Windows[ static_cast<std::size_t>( MainMenuOption::Settings ) ] = std::make_shared<SettingsWindow>();
    m_Windows[ static_cast<std::size_t>( MainMenuOption::Codex ) ] = std::make_shared<CodexWindow>();
    m_Windows[ static_cast<std::size_t>( MainMenuOption::Credits ) ] = std::make_shared<CreditsWindow>();

    for ( auto& pWindow : m_Windows )
    {
        if ( pWindow != nullptr )
        {
            g_pGame->GetUIRoot( UIDesignId::MainMenu )->Add( pWindow );
        }
    }

    m_pTitle = new EvaWindow( 0, 16, Genesis::Configuration::GetScreenWidth(), 96, false );
    m_pTitle->GetMainPanel()->SetBorderMode( Genesis::Gui::PANEL_BORDER_TOP | Genesis::Gui::PANEL_BORDER_BOTTOM );

    Genesis::ResourceImage* pTitleImage = (Genesis::ResourceImage*)Genesis::FrameWork::GetResourceManager()->GetResource( "data/ui/title.png" );
    m_pTitleImage = new Genesis::Gui::Image();
    m_pTitleImage->SetPosition( 8.0f, 10.0f );
    m_pTitleImage->SetSize( (float)pTitleImage->GetWidth(), (float)pTitleImage->GetHeight() );
    m_pTitleImage->SetTexture( pTitleImage );
    m_pTitleImage->SetColor( 1.0f, 1.0f, 1.0f, 1.0f );
    m_pTitle->GetMainPanel()->AddElement( m_pTitleImage );

    CreateVersionText();

    m_Buttons[ static_cast<std::size_t>( MainMenuOption::NewGame ) ]->Toggle( true );
}

MainMenu::~MainMenu()
{
    delete m_pTitle;
}

void MainMenu::CreateCharacterImage()
{
    m_pCharacterContainer = std::make_shared<UI::Container>( "Character container" );
    g_pGame->GetUIRoot( UIDesignId::MainMenu )->Add( m_pCharacterContainer );

    m_pCharacterImage = std::make_shared<UI::Image>( "Character image", "data/ui/portraits/Chrysamere_large.jpg" );
    m_pCharacterImage->SetFlags( UI::ElementFlags_NoSerialize | UI::ElementFlags_DynamicSize );
    m_pCharacterImage->SetAutoSize( false );
    m_pCharacterImage->SetBlendMode( UI::Image::BlendMode::Add );
    m_pCharacterImage->SetShader( "gui_portrait_menu" );
    const float ratio = static_cast<float>( m_pCharacterImage->GetWidth() ) / static_cast<float>( m_pCharacterImage->GetHeight() );
    const int h = static_cast<int>( Genesis::Configuration::GetScreenHeight() );
    const int w = static_cast<int>( static_cast<float>( h ) * ratio );
    m_pCharacterImage->SetSize( w, h );
    m_pCharacterContainer->Add( m_pCharacterImage );
}

void MainMenu::CreateVersionText()
{
    std::stringstream versionText;
    versionText << "Build " << HEXTERMINATE_BUILD_VERSION;
    Genesis::ResourceFont* pFont = UI::Fonts::Get( "kimberley18.fnt" );
    const float width = pFont->GetTextLength( versionText.str() );
    const float height = pFont->GetLineHeight();
    Genesis::Gui::Panel* pPanel = m_pTitle->GetMainPanel();
    m_pVersionText = GuiExtended::CreateText(
        (int)( pPanel->GetSize().x - width - 8.0f ),
        8,
        (int)ceilf( width ),
        (int)ceilf( height ),
        versionText.str(),
        pPanel );
    m_pVersionText->SetFont( pFont );
}

void MainMenu::Show( bool state )
{
    m_pMenu->Show( state );
    m_pTitle->GetMainPanel()->Show( state );
    m_pCharacterImage->Show( state );

    if ( state )
    {
        m_pToggleGroup->Select( m_Buttons[ static_cast<std::size_t>( MainMenuOption::NewGame ) ].get() );
    }
    else
    {
        for ( auto& pWindow : m_Windows )
        {
            if ( pWindow != nullptr )
            {
                pWindow->Show( false );
            }
        }
    }
}

void MainMenu::SetOption( MainMenuOption option )
{
    for ( size_t i = 0; i < m_Windows.size(); ++i )
    {
        UI::WindowSharedPtr pWindow = m_Windows[ i ];
        if ( pWindow != nullptr )
        {
            if ( i == static_cast<size_t>( option ) )
            {
                pWindow->Reset();
                pWindow->Show( true );
            }
            else
            {
                pWindow->Show( false );
            }
        }
    }
}

void MainMenu::OnButtonPressed( const std::any& userData )
{
    MainMenuOption option = std::any_cast<MainMenuOption>( userData );
    if ( option == MainMenuOption::Quit )
    {
        g_pGame->Quit();
    }
    else
    {
        SetOption( option );
    }
}

} // namespace Hexterminate
