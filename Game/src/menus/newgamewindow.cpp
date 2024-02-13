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
#include <sstream>

#include "hexterminate.h"
#include "menus/newgamewindow.h"
#include "menus/popup.h"
#include "savegamestorage.h"
#include "ship/shipinfo.h"
#include "ui/button.h"
#include "ui/checkbox.h"
#include "ui/image.h"
#include "ui/inputarea.h"
#include "ui/meter.h"
#include "ui/panel.h"
#include "ui/radiobutton.h"
#include "ui/text.h"
#include "ui/togglegroup.h"

namespace Hexterminate
{

NewGameWindow::NewGameWindow()
    : UI::Window( "New game window" )
    , m_GalaxyCreationInfo( GalaxyCreationInfo::CreationMode::Empty )
{
    using namespace std::placeholders;

    m_Pages.resize( static_cast<size_t>( PageId::Count ) );
    m_CurrentPage = PageId::Count;

    m_pButtonNext = std::make_shared<UI::Button>( "Next button", std::bind( &NewGameWindow::OnPageSwitchButtonPressed, this, _1 ) );
    m_pButtonPrevious = std::make_shared<UI::Button>( "Previous button", std::bind( &NewGameWindow::OnPageSwitchButtonPressed, this, _1 ) );
    GetContentPanel()->Add( m_pButtonNext );
    GetContentPanel()->Add( m_pButtonPrevious );

    CreateGameModeSelectionPage();
    CreateShipSelectionPage();
    CreateCustomisationPage();
    CreateFactionPresencePage();

    for ( auto& pPage : m_Pages )
    {
        if ( pPage != nullptr )
        {
            GetContentPanel()->Add( pPage );
        }
    }

    Select( PageId::GameMode );
}

void NewGameWindow::Update()
{
    UI::Window::Update();

    if ( m_CurrentPage == PageId::Customisation )
    {
        m_pButtonNext->Enable( !m_pShipNameInputArea->GetText().empty() && !m_pCaptainNameInputArea->GetText().empty() );
        m_ShipCustomisationData.m_ShipName = m_pShipNameInputArea->GetText();
        m_ShipCustomisationData.m_CaptainName = m_pCaptainNameInputArea->GetText();

        Popup* pPopup = g_pGame->GetPopup();
        if ( pPopup->IsActive() && pPopup->GetState() == PopupState::Yes )
        {
            pPopup->Close();
            StartNewGame();
        }
    }
    else if ( m_CurrentPage == PageId::FactionPresence )
    {
        const bool canCreate = ( m_GalaxyCreationInfo.GetFactionPresence( FactionId::Pirate ) != FactionPresence::None ) || ( m_GalaxyCreationInfo.GetFactionPresence( FactionId::Marauders ) != FactionPresence::None ) || ( m_GalaxyCreationInfo.GetFactionPresence( FactionId::Ascent ) != FactionPresence::None ) || ( m_GalaxyCreationInfo.GetFactionPresence( FactionId::Iriani ) != FactionPresence::None ) || ( m_GalaxyCreationInfo.GetFactionPresence( FactionId::Hegemon ) != FactionPresence::None );

        m_pButtonNext->Enable( canCreate );
    }

#ifdef _DEBUG
    if ( Genesis::FrameWork::GetCommandLineParameters()->HasParameter( "--new-hyperscape" ) )
    {
        static bool sStartHyperscape = true;
        if ( sStartHyperscape == true )
        {
            m_ShipCustomisationData.m_CaptainName = "Unknown";
            m_ShipCustomisationData.m_ShipName = "Ascension";
            g_pGame->StartNewHyperscapeGame( m_ShipCustomisationData, false );
            sStartHyperscape = false;
        }
    }
#endif
}

void NewGameWindow::Reset()
{
    UI::Window::Reset();

    Select( PageId::GameMode );

    m_pTipsCheckbox->SetChecked( true );
    m_pCaptainNameInputArea->SetText( "Unknown" );
    m_pShipNameInputArea->SetText( "Ascension" );
}

void NewGameWindow::Select( PageId pageId )
{
    if ( pageId == PageId::GameMode )
    {
        SetTitle( "> NEW GAME - GAME MODE" );
        m_pButtonPrevious->Show( false );
        m_pButtonNext->SetUserData( PageId::ShipSelection );
    }
    else if ( pageId == PageId::ShipSelection )
    {
        SetTitle( "> NEW GAME - SHIP SELECTION" );
        m_pButtonPrevious->Show( true );
        m_pButtonPrevious->SetUserData( PageId::GameMode );

        const GameMode gameMode = g_pGame->GetGameMode();
        if ( gameMode == GameMode::Campaign )
        {
            m_pButtonNext->SetUserData( PageId::Customisation );
        }
        else if ( gameMode == GameMode::InfiniteWar )
        {
            m_pButtonNext->SetUserData( PageId::FactionPresence );
        }
        else
        {
            Genesis::FrameWork::GetLogger()->LogError( "Unsupported game mode." );
        }
    }
    else if ( pageId == PageId::Customisation )
    {
        SetTitle( "> NEW GAME - CUSTOMISATION" );
        m_pButtonPrevious->SetUserData( PageId::ShipSelection );
        m_pButtonNext->SetUserData( PageId::StartNewGame );
    }
    else if ( pageId == PageId::FactionPresence )
    {
        SetTitle( "> NEW GAME - FACTION PRESENCE" );
        m_pButtonPrevious->SetUserData( PageId::ShipSelection );
        m_pButtonNext->SetUserData( PageId::Customisation );
    }
    else if ( pageId == PageId::StartNewGame )
    {
        const bool showOverwritePopup = g_pGame->GetSaveGameStorage()->Exists( m_pCaptainNameInputArea->GetText(), m_pShipNameInputArea->GetText() );
        if ( showOverwritePopup )
        {
            g_pGame->GetPopup()->Show( PopupMode::YesNo, "A save game with this name already exists - do you want to overwrite it?" );
            return;
        }
        else
        {
            StartNewGame();
        }
    }

    for ( size_t i = 0; i < m_Pages.size(); ++i )
    {
        if ( m_Pages[ i ] != nullptr )
        {
            m_Pages[ i ]->Show( i == static_cast<size_t>( pageId ) );
        }
    }

    m_CurrentPage = pageId;
}

void NewGameWindow::StartNewGame()
{
    GameMode gameMode = g_pGame->GetGameMode();
    if ( gameMode == GameMode::Campaign )
    {
        m_GalaxyCreationInfo = GalaxyCreationInfo( GalaxyCreationInfo::CreationMode::Campaign );
        g_pGame->StartNewLegacyGame( m_ShipCustomisationData, m_CompanionShipTemplate, m_pTipsCheckbox->IsChecked(), m_GalaxyCreationInfo );
    }
    else if ( gameMode == GameMode::InfiniteWar )
    {
        m_GalaxyCreationInfo = GalaxyCreationInfo( GalaxyCreationInfo::CreationMode::InfiniteWar );
        g_pGame->StartNewLegacyGame( m_ShipCustomisationData, m_CompanionShipTemplate, m_pTipsCheckbox->IsChecked(), m_GalaxyCreationInfo );
    }
    else if ( gameMode == GameMode::Hyperscape )
    {
        g_pGame->StartNewHyperscapeGame( m_ShipCustomisationData, m_pTipsCheckbox->IsChecked() );
    }
    else
    {
        Genesis::FrameWork::GetLogger()->LogError( "Not implemented." );
        return;
    }

    Reset();
}

void NewGameWindow::SetFactionPresence( FactionId factionId, FactionPresence presence )
{
    m_GalaxyCreationInfo.SetFactionPresence( factionId, presence );
}

void NewGameWindow::CreateGameModeSelectionPage()
{
    using namespace std::placeholders;

    m_pGameModeToggleGroup = std::make_shared<UI::ToggleGroup>();
    UI::ElementSharedPtr pPage = std::make_shared<UI::Element>( "Game mode selection page" );
    UI::ButtonSharedPtr pButtonCampaign = std::make_shared<UI::Button>( "Campaign button", std::bind( &NewGameWindow::OnGameModeButtonPressed, this, _1 ), GameMode::Campaign, m_pGameModeToggleGroup );
    UI::ButtonSharedPtr pButtonInfiniteWar = std::make_shared<UI::Button>( "Infinite War button", std::bind( &NewGameWindow::OnGameModeButtonPressed, this, _1 ), GameMode::InfiniteWar, m_pGameModeToggleGroup );
    UI::ButtonSharedPtr pButtonHyperscape = std::make_shared<UI::Button>( "Hyperscape button", std::bind( &NewGameWindow::OnGameModeButtonPressed, this, _1 ), GameMode::Hyperscape, m_pGameModeToggleGroup );

    pPage->Add( pButtonCampaign );
    pPage->Add( pButtonInfiniteWar );
    pPage->Add( pButtonHyperscape );

    m_pGameModeToggleGroup->Add( pButtonCampaign );
    m_pGameModeToggleGroup->Add( pButtonInfiniteWar );
    m_pGameModeToggleGroup->Add( pButtonHyperscape );

    pPage->Add( CreateGameModeInfoPanel( "Campaign info panel" ) );
    pPage->Add( CreateGameModeInfoPanel( "Infinite War info panel" ) );
    pPage->Add( CreateGameModeInfoPanel( "Hyperscape info panel" ) );

    m_Pages[ static_cast<size_t>( PageId::GameMode ) ] = pPage;
    pButtonCampaign->Toggle( true );

#ifdef _DEBUG
    pButtonHyperscape->Enable( false );
#else
    pButtonHyperscape->Enable( true );
#endif
}

void NewGameWindow::CreateShipSelectionPage()
{
    using namespace std::placeholders;

    m_pShipToggleGroup = std::make_shared<UI::ToggleGroup>();
    UI::ElementSharedPtr pPage = std::make_shared<UI::Element>( "Ship selection page" );
    UI::ButtonSharedPtr pButtonLancer = std::make_shared<UI::Button>( "Lancer button", std::bind( &NewGameWindow::OnShipButtonPressed, this, _1 ), std::string( "lancer" ), m_pShipToggleGroup );
    UI::ButtonSharedPtr pButtonPhalanx = std::make_shared<UI::Button>( "Phalanx button", std::bind( &NewGameWindow::OnShipButtonPressed, this, _1 ), std::string( "phalanx" ), m_pShipToggleGroup );
    UI::ButtonSharedPtr pButtonIronclad = std::make_shared<UI::Button>( "Ironclad button", std::bind( &NewGameWindow::OnShipButtonPressed, this, _1 ), std::string( "ironclad" ), m_pShipToggleGroup );

    pPage->Add( pButtonLancer );
    pPage->Add( pButtonPhalanx );
    pPage->Add( pButtonIronclad );

    m_pShipToggleGroup->Add( pButtonLancer );
    m_pShipToggleGroup->Add( pButtonPhalanx );
    m_pShipToggleGroup->Add( pButtonIronclad );

    pPage->Add( CreateShipInfoPanel( "Lancer info panel" ) );
    pPage->Add( CreateShipInfoPanel( "Phalanx info panel" ) );
    pPage->Add( CreateShipInfoPanel( "Ironclad info panel" ) );

    m_Pages[ static_cast<size_t>( PageId::ShipSelection ) ] = pPage;
    pButtonLancer->Toggle( true );
}

void NewGameWindow::CreateCustomisationPage()
{
    using namespace std::placeholders;

    UI::ElementSharedPtr pPage = std::make_shared<UI::Element>( "Customisation page" );
    pPage->Add( CreateShipDetailsPanel() );
    pPage->Add( CreateDifficultyPanel() );
    pPage->Add( CreatePreferencesPanel() );
    m_Pages[ static_cast<size_t>( PageId::Customisation ) ] = pPage;
}

void NewGameWindow::CreateFactionPresencePage()
{
    using namespace std::placeholders;

    UI::ElementSharedPtr pPage = std::make_shared<UI::Element>( "Faction presence page" );

    std::array<FactionId, 6> factions = {
        FactionId::Empire,
        FactionId::Pirate,
        FactionId::Marauders,
        FactionId::Ascent,
        FactionId::Iriani,
        FactionId::Hegemon
    };

    for ( FactionId id : factions )
    {
        pPage->Add( CreateFactionPresencePanel( id ) );
    }

    m_Pages[ static_cast<size_t>( PageId::FactionPresence ) ] = pPage;
}

UI::ElementSharedPtr NewGameWindow::CreateGameModeInfoPanel( const std::string& name ) const
{
    UI::PanelSharedPtr pPanel = std::make_shared<UI::Panel>( name );
    UI::TextSharedPtr pText = std::make_shared<UI::Text>( "Description" );
    pPanel->Add( pText );
    return pPanel;
}

UI::ElementSharedPtr NewGameWindow::CreateShipInfoPanel( const std::string& name ) const
{
    UI::PanelSharedPtr pPanel = std::make_shared<UI::Panel>( name );
    std::vector<UI::ElementSharedPtr> elements;

    elements.push_back( std::make_shared<UI::Text>( "Description" ) );
    elements.push_back( std::make_shared<UI::Meter>( "Defense meter", 5 ) );
    elements.push_back( std::make_shared<UI::Meter>( "Attack meter", 5 ) );
    elements.push_back( std::make_shared<UI::Meter>( "Speed meter", 5 ) );
    elements.push_back( std::make_shared<UI::Image>( "Defense icon" ) );
    elements.push_back( std::make_shared<UI::Image>( "Attack icon" ) );
    elements.push_back( std::make_shared<UI::Image>( "Speed icon" ) );

    for ( auto& pElement : elements )
    {
        pPanel->Add( pElement );
    }

    return pPanel;
}

UI::ElementSharedPtr NewGameWindow::CreateShipDetailsPanel()
{
    UI::PanelSharedPtr pPanel = std::make_shared<UI::Panel>( "Ship details panel" );

    pPanel->Add( std::make_shared<UI::Text>( "Title" ) );
    pPanel->Add( std::make_shared<UI::Text>( "Ship name" ) );
    pPanel->Add( std::make_shared<UI::Text>( "Captain name" ) );

    m_pShipNameInputArea = std::make_shared<UI::InputArea>( "Ship name input area" );
    pPanel->Add( m_pShipNameInputArea );

    m_pCaptainNameInputArea = std::make_shared<UI::InputArea>( "Captain name input area" );
    pPanel->Add( m_pCaptainNameInputArea );

    return pPanel;
}

UI::ElementSharedPtr NewGameWindow::CreateDifficultyPanel()
{
    UI::PanelSharedPtr pPanel = std::make_shared<UI::Panel>( "Difficulty panel" );

    pPanel->Add( std::make_shared<UI::Text>( "Title" ) );

    pPanel->Add( std::make_shared<UI::RadioButton>( "Easy radio button", "difficulty", []() { g_pGame->SetDifficulty( Difficulty::Easy ); } ) );
    pPanel->Add( std::make_shared<UI::RadioButton>( "Hardcore radio button", "difficulty", []() { g_pGame->SetDifficulty( Difficulty::Hardcore ); } ) );

    // We need to keep this one stored so we can reset to defaults when coming back to this window.
    m_pNormalDifficultyRadioButton = std::make_shared<UI::RadioButton>( "Normal radio button", "difficulty", []() { g_pGame->SetDifficulty( Difficulty::Normal ); } );
    pPanel->Add( m_pNormalDifficultyRadioButton );

    const std::array<std::string, 3> panelNames = { "Easy panel", "Normal panel", "Hardcore panel" };
    for ( const std::string& panelName : panelNames )
    {
        UI::PanelSharedPtr pDescriptionPanel = std::make_shared<UI::Panel>( panelName );
        pDescriptionPanel->Add( std::make_shared<UI::Text>( "Text" ) );
        pPanel->Add( pDescriptionPanel );
    }

    return pPanel;
}

UI::ElementSharedPtr NewGameWindow::CreateFactionPresencePanel( FactionId id )
{
    Faction* pFaction = g_pGame->GetFaction( id );
    std::stringstream elementName;
    elementName << pFaction->GetName() << " element";
    UI::ElementSharedPtr pElement = std::make_shared<UI::Element>( elementName.str() );

    UI::PanelSharedPtr pPresencePanel = std::make_shared<UI::Panel>( "Presence panel" );
    pElement->Add( pPresencePanel );

    UI::TextSharedPtr pFactionName = std::make_shared<UI::Text>( "Faction text" );
    pPresencePanel->Add( pFactionName );

    std::stringstream groupName;
    groupName << pFaction->GetName() << " presence";

    UI::RadioButtonSharedPtr pPresenceNone = std::make_shared<UI::RadioButton>( "No presence radio button", groupName.str(), [ this, id ]() { SetFactionPresence( id, FactionPresence::None ); } );
    UI::RadioButtonSharedPtr pPresenceLight = std::make_shared<UI::RadioButton>( "Light presence radio button", groupName.str(), [ this, id ]() { SetFactionPresence( id, FactionPresence::Light ); } );
    UI::RadioButtonSharedPtr pPresenceHeavy = std::make_shared<UI::RadioButton>( "Heavy presence radio button", groupName.str(), [ this, id ]() { SetFactionPresence( id, FactionPresence::Heavy ); } );
    pPresencePanel->Add( pPresenceNone );
    pPresencePanel->Add( pPresenceLight );
    pPresencePanel->Add( pPresenceHeavy );

    UI::PanelSharedPtr pImagePanel = std::make_shared<UI::Panel>( "Image panel" );
    pElement->Add( pImagePanel );

    UI::ImageSharedPtr pFactionImage = std::make_shared<UI::Image>( "Faction image" );
    pImagePanel->Add( pFactionImage );

    return pElement;
}

UI::ElementSharedPtr NewGameWindow::CreatePreferencesPanel()
{
    UI::PanelSharedPtr pPanel = std::make_shared<UI::Panel>( "Preferences panel" );

    pPanel->Add( std::make_shared<UI::Text>( "Title" ) );

    m_pTipsCheckbox = std::make_shared<UI::Checkbox>( "TipsCheckbox" );
    pPanel->Add( m_pTipsCheckbox );

    UI::PanelSharedPtr pDescriptionPanel = std::make_shared<UI::Panel>( "Description panel" );
    pDescriptionPanel->Add( std::make_shared<UI::Text>( "Text" ) );
    pPanel->Add( pDescriptionPanel );

    return pPanel;
}

void NewGameWindow::OnPageSwitchButtonPressed( const std::any& userData )
{
    PageId pageId = std::any_cast<PageId>( userData );
    Select( pageId );
}

void NewGameWindow::OnGameModeButtonPressed( const std::any& userData )
{
    GameMode mode = std::any_cast<GameMode>( userData );
    g_pGame->SetGameMode( mode );

    if ( mode == GameMode::Hyperscape )
    {
        m_pButtonNext->SetUserData( PageId::Customisation );
    }
    else
    {
        m_pButtonNext->SetUserData( PageId::ShipSelection );
    }
}

void NewGameWindow::OnShipButtonPressed( const std::any& userData )
{
    std::string selectedShip = std::any_cast<std::string>( userData );
    const ShipInfo* pInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetPlayerFaction(), selectedShip );
    if ( pInfo == nullptr )
    {
        Genesis::FrameWork::GetLogger()->LogError( "Couldn't find hexgrid '%s' for player faction", selectedShip.c_str() );
    }
    else
    {
        m_ShipCustomisationData.m_pModuleInfoHexGrid = pInfo->GetModuleInfoHexGrid();

        // The player's fleet is composed of the same type of ship that he starts the game with.
        // This helps with keeping the entire fleet using the same style of play.
        m_CompanionShipTemplate = selectedShip;
    }
}

} // namespace Hexterminate
