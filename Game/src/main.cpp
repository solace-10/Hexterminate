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

#ifdef _WIN32
#include "Shlwapi.h"
#include <windows.h>
#pragma comment( lib, "Shlwapi.lib" )
#endif

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

#include <imgui/imgui.h>

#include <configuration.h>
#include <genesis.h>
#include <gui/gui.h>
#include <gui/video.h>
#include <imgui/imgui_impl.h>
#include <logger.h>
#include <memory.h>
#include <physics/simulation.h>
#include <render/debugrender.h>
#include <rendersystem.h>
#include <resourcemanager.h>
#include <resources/resourcemodel.h>
#include <resources/resourceplaylist.h>
#include <resources/resourcesound.h>
#include <resources/resourcevideo.h>
#include <shadercache.h>
#include <sound/soundmanager.h>
#include <stringaux.h>
#include <taskmanager.h>
#include <time.h>
#include <timer.h>
#include <videoplayer.h>
#include <window.h>
#include <xml.h>

#include "achievements.h"
#include "faction/empirefaction.h"
#include "faction/faction.h"
#include "faction/irianifaction.h"
#include "faction/neutralfaction.h"
#include "faction/piratefaction.h"
#include "fleet/fleet.h"
#include "fleet/fleetrep.h"
#include "hexterminate.h"
#include "hyperscape/hyperscape.h"
#include "menus/console.h"
#include "menus/galaxywindow.h"
#include "menus/intelwindow.h"
#include "menus/loadingscreen.h"
#include "menus/mainmenu.h"
#include "menus/pointofinterest.h"
#include "menus/popup.h"
#include "menus/tutorialwindow.h"
#include "misc/gui.h"
#include "misc/random.h"
#include "misc/randomshuffle.h"
#include "perks.h"
#include "player.h"
#include "requests/campaigntags.h"
#include "requests/requestmanager.h"
#include "savegameheader.h"
#include "savegamestorage.h"
#include "sector/backgroundinfo.h"
#include "sector/events/chrysamere.h"
#include "sector/events/corsairfleet.h"
#include "sector/events/neutralflagship.h"
#include "sector/events/orbitaldefenses.h"
#include "sector/galaxy.h"
#include "sector/galaxycreationinfo.h"
#include "sector/galaxyrep.h"
#include "sector/sector.h"
#include "sector/sectorinfo.h"
#include "shadertweaks.h"
#include "ship/inventory.h"
#include "ship/module.h"
#include "ship/ship.h"
#include "ship/shipinfo.h"
#include "ship/shipoutline.h"
#include "stringaux.h"
#include "ui/editor.h"
#include "ui/rootelement.h"
#include "xmlaux.h"

#include "particles/particleemitter.h"
#include "particles/particlemanager.h"

namespace Hexterminate
{

Game* g_pGame = nullptr;

//-------------------------------------------------------------------
// Game
//-------------------------------------------------------------------

Game::Game()
    : m_pMainMenu( nullptr )
    , m_pConsole( nullptr )
    , m_pSector( nullptr )
    , m_pPlayer( nullptr )
    , m_pGalaxy( nullptr )
    , m_pMusicTitle( nullptr )
    , m_pTutorialWindow( nullptr )
    , m_State( GameState::Menu )
    , m_pIntelWindow( nullptr )
    , m_pAchievementsManager( nullptr )
#ifdef _DEBUG
    , m_ImGuiToggleToken( Genesis::InputManager::sInvalidInputCallbackToken )
    , m_pFrameText( nullptr )
#endif
    , m_PlayedTime( 0.0f )
    , m_EndGame( false )
    , m_FirstTimeInCombat( false )
    , m_IsPaused( false )
    , m_IsInputBlocked( false )
    , m_InputBlockedTimer( 0.0f )
    , m_pNPCPerks( nullptr )
    , m_ContextualTipsEnabled( true )
    , m_QuitRequested( false )
    , m_pVideoElement( nullptr )
    , m_CursorType( CursorType::Pointer )
    , m_LoadToState( GameState::Unknown )
    , m_Difficulty( Difficulty::Normal )
    , m_GameMode( GameMode::Campaign )
    , m_KillSave( false )
    , m_ShowImguiTestWindow( false )
    , m_AllResourcesLoaded( false )
    , m_pModuleInfoManager( nullptr )
    , m_pPhysicsSimulation( nullptr )
    , m_pPopup( nullptr )
    , m_pShipInfoManager( nullptr )
{
    using namespace Genesis;

    srand( (unsigned int)time( nullptr ) );

    TaskManager* taskManager = FrameWork::GetTaskManager();
    taskManager->AddTask( "GameLoop", this, (TaskFunc)&Game::Update, TaskPriority::GameLogic );

    for ( int i = 0; i < (int)FactionId::Count; ++i )
    {
        m_pFaction[ i ] = nullptr;
    }

#ifdef _DEBUG
    InputManager* pInputManager = FrameWork::GetInputManager();
    m_ImGuiToggleToken = pInputManager->AddKeyboardCallback( std::bind( &Game::ToggleImGui, this ), SDL_SCANCODE_F1, ButtonState::Pressed );
#endif
}

Game::~Game()
{
    ShaderTweaksDebugWindow::Unregister();

    delete m_pSector;
    delete m_pTutorialWindow;
    delete m_pConsole;
    delete m_pMainMenu;
    delete m_pPopup;
    delete m_pGalaxy;
    delete m_pShipInfoManager;
    delete m_pMusicTitle;
    delete m_pModuleInfoManager;
    delete m_pNPCPerks;
    delete m_pIntelWindow;
    delete m_pAchievementsManager;

    for ( unsigned int i = 0; i < (unsigned int)FactionId::Count; ++i )
    {
        delete m_pFaction[ i ];
    }

    delete m_pPhysicsSimulation;

    for ( auto& pSectorEvent : m_SectorEvents )
    {
        delete pSectorEvent;
    }
    m_SectorEvents.clear();

#ifdef _DEBUG
    Genesis::InputManager* pInputManager = Genesis::FrameWork::GetInputManager();
    if ( pInputManager != nullptr )
    {
        pInputManager->RemoveKeyboardCallback( m_ImGuiToggleToken );
    }
#endif

    if ( m_LoaderThread.joinable() )
    {
        m_LoaderThread.join();
    }

#if USE_STEAM
    SteamAPI_Shutdown();
#endif
}

void Game::Initialise()
{
    Random::Initialise();
    RandomShuffle::Initialise();

#ifdef _DEBUG
    m_pConsole = new Console(); // Should happen early so all the warnings / log entries are visible
#endif

    std::stringstream build;
    build << "HEXTERMINATE build " << HEXTERMINATE_BUILD_VERSION;
    Genesis::FrameWork::GetLogger()->LogInfo( "%s", build.str().c_str() );

    m_pLoadingScreen = LoadingScreenUniquePtr( new LoadingScreen );
    m_pBlackboard = std::make_shared<Blackboard>();
    m_pModuleInfoManager = new ModuleInfoManager();
    m_pShipInfoManager = new ShipInfoManager();

    m_pNPCPerks = new Perks();
    m_pNPCPerks->Enable( Perk::GunshipConstruction );
    m_pNPCPerks->Enable( Perk::BattlecruiserConstruction );
    m_pNPCPerks->Enable( Perk::BattleshipConstruction );
    m_pNPCPerks->Enable( Perk::DreadnaughtConstruction );
    m_pNPCPerks->Enable( Perk::Superconductors );

    SetupBackgrounds();
    SetupFactions();
    m_pShipInfoManager->Initialise();

    m_pPhysicsSimulation = new Genesis::Physics::Simulation();
    Genesis::FrameWork::GetTaskManager()->AddTask(
        "Physics",
        m_pPhysicsSimulation,
        (Genesis::TaskFunc)&Genesis::Physics::Simulation::Update,
        Genesis::TaskPriority::Physics );

    m_pPopup = new Popup();
    m_pMusicTitle = new MusicTitle();

#ifdef _DEBUG
    m_pFrameText = GuiExtended::CreateText( 8, 8, 1024, 128, "", nullptr );
    m_pFrameText->SetColor( 1.0f, 0.4f, 0.0f, 1.0f );
#endif

    SetCursorType( CursorType::Pointer );

    ShaderTweaksDebugWindow::Register();

    using namespace Genesis;
    if ( FrameWork::GetCommandLineParameters()->HasParameter( "--no-intro" ) == false )
    {
        ResourceVideo* pWingsOfSteelVideo = (ResourceVideo*)FrameWork::GetResourceManager()->GetResource( "data/videos/WingsOfSteel.ivf" );
        pWingsOfSteelVideo->SetSkippable( true );
        Genesis::FrameWork::GetVideoPlayer()->Play( pWingsOfSteelVideo );
    }

#if USE_STEAM
    if ( SteamAPI_RestartAppIfNecessary( STEAM_APP_ID ) )
    {
        Quit();
        return;
    }

    if ( SteamAPI_Init() )
    {
        Genesis::FrameWork::GetLogger()->LogInfo( "Steam API initialised." );
    }
    else
    {
        Genesis::FrameWork::GetLogger()->LogWarning( "SteamAPI_Init() failed." );
    }
#endif // USE_STEAM

    Genesis::ImGuiImpl::RegisterMenu( "Tools", "ImGui test window", &m_ShowImguiTestWindow );

    m_pAchievementsManager = new AchievementsManager();
    m_pShipOutline = std::make_unique<ShipOutline>();
    m_pUIEditor = std::make_unique<UI::Editor>();
    LoadUIDesigns();

    SetState( GameState::Intro );
}

SaveGameStorage* Game::GetSaveGameStorage() const
{
    return m_pSaveGameStorage.get();
}

void Game::ToggleImGui()
{
#ifdef _DEBUG
    Genesis::ImGuiImpl::Enable( !Genesis::ImGuiImpl::IsEnabled() );
#endif
}

void Game::SetInputBlocked( bool state )
{
    m_IsInputBlocked = state;

    if ( m_IsInputBlocked )
        m_InputBlockedTimer = 0.2f;
}

bool Game::IsInputBlocked() const
{
    return m_InputBlockedTimer > 0.0f || Genesis::ImGuiImpl::IsEnabled();
}

Genesis::TaskStatus Game::Update( float delta )
{
#if USE_STEAM
    SteamAPI_RunCallbacks();
#endif

    if ( m_ShowImguiTestWindow )
    {
        ImGui::ShowDemoWindow( &m_ShowImguiTestWindow );
    }

    for ( auto& pRootElement : m_UIRootElements )
    {
        if ( pRootElement )
        {
            pRootElement->Update();
        }
    }

    m_pUIEditor->UpdateDebugUI();
    GetBlackboard()->UpdateDebugUI();
    ShaderTweaksDebugWindow::Update();

    if ( GetSaveGameStorage() != nullptr )
    {
        GetSaveGameStorage()->UpdateDebugUI();
    }

    if ( GetState() == GameState::LoadResources )
    {
        if ( m_AllResourcesLoaded )
        {
            m_pLoadingScreen->Show( false );

            // Initialising the save game storage has been delayed from startup until the game assets are loaded due to
            // an issue with Steam, where a race condition can cause Steam to report that there are no files in storage.
            // This issue has been reported and acknowledged by the Steam team and is expected to be fixed in an upcoming
            // Steam client update. However, as there's no external visibility of Steam's issue tracker, this workaround
            // will remain in place for the foreseeable future.
            m_pSaveGameStorage = std::make_unique<SaveGameStorage>();

            SetState( GameState::Menu );
        }
        else
        {
            m_pLoadingScreen->Update();
        }
    }

    if ( m_LoadToState != GameState::Unknown )
    {
        m_pLoadingScreen->Show( false );
        SetState( m_LoadToState );
        m_LoadToState = GameState::Unknown;
    }

    if ( m_GameToLoad.empty() == false )
    {
        LoadGameAux();
    }

    if ( m_IsInputBlocked == false && m_InputBlockedTimer > 0.0f )
    {
        m_InputBlockedTimer = std::max( 0.0f, m_InputBlockedTimer - delta );
    }

    if ( m_pGalaxy )
    {
        m_pGalaxy->Update( delta );
    }

    if ( m_pHyperscape )
    {
        m_pHyperscape->Update( delta );
    }

    if ( m_pPopup )
    {
        m_pPopup->Update( delta );
    }

    if ( m_pTutorialWindow )
    {
        m_pTutorialWindow->Update( delta );
    }

    if ( m_pMusicTitle )
    {
        m_pMusicTitle->Update( delta );
    }

    if ( m_pSector )
    {
        m_pSector->Update( delta );
    }

    if ( m_pConsole )
    {
        m_pConsole->Update( delta );
    }

    if ( m_pIntelWindow )
    {
        m_pIntelWindow->Update( delta );
    }

    if ( m_pAchievementsManager )
    {
        m_pAchievementsManager->Update();
    }

    if ( GetState() == GameState::GalaxyView || GetState() == GameState::HyperscapeView || GetState() == GameState::Combat || GetState() == GameState::Shipyard )
    {
        if ( IsPaused() == false )
        {
            m_PlayedTime += delta * GetGalaxy()->GetCompression();
        }
    }

    if ( GetState() == GameState::Intro && Genesis::FrameWork::GetVideoPlayer()->IsPlaying() == false )
    {
        SetState( GameState::LoadResources );
    }

    if ( m_EndGame )
    {
        EndGameAux();
    }

    return Genesis::TaskStatus::Continue;
}

void Game::StartNewLegacyGame( const ShipCustomisationData& customisationData, const std::string& companionShipTemplate, bool tutorialEnabled, const GalaxyCreationInfo& galaxyCreationInfo )
{
    SDL_assert( GetPlayer() == nullptr );

    m_pBlackboard->Clear();

    m_FirstTimeInCombat = true;
    m_pPlayer = new Player( customisationData, companionShipTemplate );
    m_pMainMenu->Show( false );
    m_pGalaxy->Create( galaxyCreationInfo );

    SetPlayedTime( 0.0f );

    InitialiseSectorEvents();

    // Create a fleet for the player
    const SectorInfoVector& controlledSectors = GetFaction( FactionId::Empire )->GetControlledSectors();
    if ( controlledSectors.empty() )
    {
        Genesis::FrameWork::GetLogger()->LogError( "Game starting with no sectors controlled by the Empire, can't spawn player's fleet." );
    }
    else
    {
        SectorInfo* pStartingSector = nullptr;
        const GameMode gameMode = GetGameMode();
        for ( auto& pSectorInfo : controlledSectors )
        {
            if ( gameMode == GameMode::Campaign )
            {
                if ( pSectorInfo->GetName().find( "Tannhauser" ) != std::string::npos )
                {
                    pStartingSector = pSectorInfo;
                    break;
                }
            }
            else if ( gameMode == GameMode::InfiniteWar )
            {
                if ( pSectorInfo->IsHomeworld() )
                {
                    pStartingSector = pSectorInfo;
                    break;
                }
            }
        }

        FleetSharedPtr pFleet = GetPlayerFaction()->BuildFleet( pStartingSector ).lock();

        // Add two ships to the player's fleet. These are of the same type as the player so they can
        // stick together and share the same style of play.
        const ShipInfo* pCompanionShipInfo = GetShipInfoManager()->Get( GetFaction( FactionId::Empire ), companionShipTemplate );
        if ( pCompanionShipInfo == nullptr )
        {
            Genesis::FrameWork::GetLogger()->LogWarning( "Couldn't find ship '%s'", companionShipTemplate.c_str() );
        }
        else
        {
            pFleet->AddShip( pCompanionShipInfo );
            pFleet->AddShip( pCompanionShipInfo );
        }

        if ( GetPlayerFleet().expired() )
        {
            Genesis::FrameWork::GetLogger()->LogError( "Couldn't find starting sector, can't spawn player's fleet." );
        }
    }

    m_ContextualTipsEnabled = tutorialEnabled;
    if ( m_ContextualTipsEnabled )
    {
        GetBlackboard()->Add( "#contextual_tips" );
    }

    if ( tutorialEnabled )
    {
        SetupNewGameTutorial();
    }

    for ( int i = 0; i < static_cast<int>( FactionId::Count ); ++i )
    {
        FactionId factionId = static_cast<FactionId>( i );
        g_pGame->GetFaction( factionId )->SetInitialPresence( galaxyCreationInfo.GetFactionPresence( factionId ) );
    }

    SetState( GameState::GalaxyView );
}

void Game::StartNewHyperscapeGame( const ShipCustomisationData& customisationData, bool tutorialEnabled )
{
    SDL_assert( GetPlayer() == nullptr );

    m_pBlackboard->Clear();
    m_pGalaxy->Show( false );

    m_FirstTimeInCombat = true;
    m_pPlayer = new Player( customisationData, "phalanx" );
    m_pMainMenu->Show( false );

    m_pHyperscape = std::make_unique<Hyperscape>();

    SetPlayedTime( 0.0f );

    // TODO: Create a fleet for the player

    m_ContextualTipsEnabled = tutorialEnabled;
    if ( m_ContextualTipsEnabled )
    {
        GetBlackboard()->Add( "#contextual_tips" );
    }

    if ( tutorialEnabled )
    {
        SetupNewGameTutorial();
    }

    SetState( GameState::HyperscapeView );
}

void Game::EndGame()
{
    m_EndGame = true;
}

void Game::EndGameAux()
{
    if ( GetCurrentSector() != nullptr )
    {
        ExitSector();
    }

    SaveGame();

    delete m_pTutorialWindow;

    if ( m_pIntelWindow != nullptr )
    {
        m_pIntelWindow->Clear();
    }

    SetupFactions(); // Recreate the factions to clear all stored data regarding fleets / sectors
    m_pGalaxy->Reset();
    m_pGalaxy->Show( true );

    delete m_pPlayer;
    m_pPlayer = nullptr;

    m_pMainMenu->Show( true );
    m_pMainMenu->SetOption( MainMenuOption::NewGame );

    SetState( GameState::Menu );

    m_EndGame = false;
}

void Game::SetupNewGameTutorial()
{
    if ( m_pTutorialWindow == nullptr )
    {
        m_pTutorialWindow = new TutorialWindow();
    }
    else
    {
        m_pTutorialWindow->Clear();
    }

    if ( GetGameMode() == GameMode::Campaign )
    {
        TutorialStep tutorialStep1(
            "Welcome to Hexterminate, Captain.\n\n"
            "This is the galaxy view, where you can see the location of "
            "your fleet and decide on your next move.\n\n"
            "Your objective is to defeat the other factions, reuniting "
            "what was once a galaxy spanning Empire." );
        m_pTutorialWindow->AddStep( tutorialStep1 );

        TutorialStep tutorialStep2(
            "Blue sectors belong to the Empire. As you reclaim sectors, "
            "the Empire will be able to field a greater number of fleets.\n\n"
            "As fleets are built and deployed, they'll start expanding our "
            "borders and will assist you in battle." );
        m_pTutorialWindow->AddStep( tutorialStep2 );

        TutorialStep tutorialStep3(
            "This is your fleet.\n\n"
            "As you move through the galaxy, you will be able to decide "
            "whether you want to try to conquer a given sector or not. "
            "When you arrive to a sector you'll see an associated threat "
            "rating, which will give you an idea of how difficult the battle "
            "would be.\n\n"
            "Keep in mind that this rating is just an estimate based on "
            "data from our probes and isn't necessarily accurate, so if "
            "you see that a battle is about to turn sour, jump out." );
        FleetSharedPtr pPlayerFleet = GetPlayerFleet().lock();
        tutorialStep3.SetPointOfInterest( pPlayerFleet->GetRepresentation() );
        m_pTutorialWindow->AddStep( tutorialStep3 );

        TutorialStep tutorialStep4(
            "The sector you are currently in has a shipyard, represented "
            "by the shipyard icon. If you enter the sector and then dock "
            "with the shipyard, you'll be able to customise your ship with "
            "the modules you recover during your battles.\n\n"
            "However, be aware that shipyards in a sector controlled by an "
            "enemy faction will be well defended." );
        m_pTutorialWindow->AddStep( tutorialStep4 );

        TutorialStep tutorialStep5(
            "We recommend attacking the unaligned sectors surrounding the\n"
            "Empire first, as their fleets should be fairly manageable.\n\n"
            "Good luck, Captain." );
        m_pTutorialWindow->AddStep( tutorialStep5 );
    }
    else if ( GetGameMode() == GameMode::InfiniteWar )
    {
        TutorialStep tutorialStep1(
            "Welcome to the Infinite War, Captain.\n\n"
            "If you have not played the Campaign before, I recommend doing "
            "so first.\n\n" );
        m_pTutorialWindow->AddStep( tutorialStep1 );

        TutorialStep tutorialStep2(
            "This view should be familiar, but note that it is now mostly "
            "covered by fog of war. You can see the areas surrounding the "
            "Empire, your fleet and allied fleets. You will have to explore "
            "to discover the location of the other factions." );
        m_pTutorialWindow->AddStep( tutorialStep2 );

        TutorialStep tutorialStep3(
            "To assist you with this, you will now have the option to "
            "deploy hyperspace probes on Imperial or neutral sectors. "
            "These will permanently reveal surrounding sectors, providing "
            "an early warning of incoming fleets or encroaching factions." );
        m_pTutorialWindow->AddStep( tutorialStep3 );

        TutorialStep tutorialStep4(
            "Additionally, you can also deploy starforts. These are "
            "very powerful defensive structures, which make a sector very "
            "difficult for a hostile fleet to take over.\n\n"
            "Your homeworld starts with one, so it should be quite safe for now." );
        m_pTutorialWindow->AddStep( tutorialStep4 );

        TutorialStep tutorialStep5(
            "Your objective is to conquer the homeworld sector of all enemy factions. "
            "These will be marked once you discover them.\n\n"
            "Good luck, Captain." );
        m_pTutorialWindow->AddStep( tutorialStep5 );
    }
    else if ( GetGameMode() == GameMode::Hyperscape )
    {
        // TODO: Write Hyperscape tutorial.
        TutorialStep tutorialStep1(
            "Welcome to the Hyperscape, Captain." );
        m_pTutorialWindow->AddStep( tutorialStep1 );
    }
}

void Game::KillSaveGame()
{
    m_KillSave = true;
}

bool Game::SaveGame()
{
    return GetSaveGameStorage() != nullptr && GetSaveGameStorage()->SaveGame( m_KillSave );
}

void Game::LoadGame( SaveGameHeaderWeakPtr pSaveGameHeaderWeakPtr )
{
    SaveGameHeaderSharedPtr pSaveGameHeader = pSaveGameHeaderWeakPtr.lock();
    if ( pSaveGameHeader == nullptr )
    {
        g_pGame->RaiseInteractiveWarning( "Couldn't load game, header is null." );
        return;
    }

    if ( pSaveGameHeader->IsValid() == false )
    {
        g_pGame->RaiseInteractiveWarning( "Couldn't load game, header is invalid." );
        return;
    }
    else if ( pSaveGameHeader->GetDifficulty() == Difficulty::Hardcore && !pSaveGameHeader->IsAlive() )
    {
        GetPopup()->Show( PopupMode::Ok, "This ship has been destroyed and serves the Empire no more..." );
        return;
    }

    SetDifficulty( pSaveGameHeader->GetDifficulty() );
    SetGameMode( pSaveGameHeader->GetGameMode() );

    m_pLoadingScreen->Show( true );
    m_GameToLoad = pSaveGameHeader->GetFilename();
}

void Game::LoadGameAux()
{
    using namespace tinyxml2;

    if ( m_GameToLoad.empty() )
    {
        return;
    }

    std::filesystem::path filename = m_GameToLoad;
    m_GameToLoad.clear();

    tinyxml2::XMLDocument xmlDoc;
    if ( m_pSaveGameStorage->LoadGame( filename, xmlDoc ) == false )
    {
        return;
    }

    bool hasErrors = false;
    XMLElement* pRootElem = xmlDoc.FirstChildElement();

    if ( !hasErrors )
    {
        // Load the Player and therefore the Inventory / ship's hexgrid template
        for ( XMLElement* pElem = pRootElem->FirstChildElement(); pElem != nullptr; pElem = pElem->NextSiblingElement() )
        {
            const std::string value( pElem->Value() );
            if ( value == "Player" )
            {
                m_pPlayer = new Player();
                hasErrors = ( m_pPlayer->Read( pElem ) == false );
                break;
            }
        }
    }

    if ( !hasErrors )
    {
        // Load the galaxy but make sure we already have the Player created.
        // Also load the blackboard if one exists
        for ( XMLElement* pElem = pRootElem->FirstChildElement(); pElem != nullptr; pElem = pElem->NextSiblingElement() )
        {
            const std::string value( pElem->Value() );
            if ( value == "Galaxy" )
            {
                m_pGalaxy->Create( GalaxyCreationInfo( GalaxyCreationInfo::CreationMode::Empty ) );
                hasErrors |= ( m_pGalaxy->Read( pElem ) == false );
            }
            else if ( value == "Factions" )
            {
                int i = 0;
                for ( XMLElement* pFactionElement = pElem->FirstChildElement(); pFactionElement != NULL; pFactionElement = pFactionElement->NextSiblingElement() )
                {
                    SDL_assert_release( i < static_cast<int>( FactionId::Count ) );
                    hasErrors |= ( m_pFaction[ i ]->Read( pFactionElement ) == false );
                    i++;
                }
            }
            else if ( value == "Blackboard" )
            {
                hasErrors |= ( m_pBlackboard->Read( pElem ) == false );
            }
        }
    }

    if ( !hasErrors )
    {
        m_pMainMenu->Show( false );
        m_ContextualTipsEnabled = m_pBlackboard->Exists( "#contextual_tips" );
        InitialiseSectorEvents();
        SetState( GameState::GalaxyView );
    }
    else
    {
        delete m_pPlayer;
        RaiseInteractiveWarning( "Invalid save file." );
    }

    m_pLoadingScreen->Show( false );
}

void Game::SetState( GameState newState )
{
    m_State = newState;

    using namespace Genesis;
    std::string playlistName;

    if ( m_State == GameState::LoadResources )
    {
        LoadResourcesAsync();
        playlistName = "data/playlists/menu.m3u";
    }
    else if ( m_State == GameState::Menu )
    {
        m_KillSave = false;
        playlistName = "data/playlists/menu.m3u";

        if ( m_pMainMenu == nullptr )
        {
            ShowCursor( true );

            if ( m_pIntelWindow == nullptr )
            {
                m_pIntelWindow = new IntelWindow();
            }

            if ( m_pGalaxy == nullptr )
            {
                m_pGalaxy = new Galaxy();
                m_pGalaxy->Show( true );
            }

            m_pMainMenu = new MainMenu();
        }
    }
    else if ( m_State == GameState::GalaxyView )
    {
        playlistName = "data/playlists/galaxyview.m3u";
    }
    else if ( m_State == GameState::HyperscapeView )
    {
        playlistName = "data/playlists/galaxyview.m3u";
    }

    if ( m_pGalaxy )
    {
        m_pGalaxy->GetRepresentation()->GetGalaxyWindow()->Show( m_State == GameState::GalaxyView );
    }

    if ( playlistName.empty() == false )
    {
        ResourcePlaylist* pPlaylistResource = FrameWork::GetResourceManager()->GetResource<ResourcePlaylist*>( playlistName );
        FrameWork::GetSoundManager()->SetPlaylist( pPlaylistResource, true );
    }

    if ( m_State == GameState::Intro && m_pVideoElement == nullptr )
    {
        ShowCursor( false );

        m_pVideoElement = new Genesis::Gui::Video();
        m_pVideoElement->SetSize( (int)Configuration::GetScreenWidth(), (int)Configuration::GetScreenHeight() );
        m_pVideoElement->SetColor( 1.0f, 1.0f, 1.0f, 1.0f );
        m_pVideoElement->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
        m_pVideoElement->SetPosition( 0, 0 );
        FrameWork::GetGuiManager()->AddElement( m_pVideoElement );
    }
    else if ( m_pVideoElement != nullptr )
    {
        FrameWork::GetGuiManager()->RemoveElement( m_pVideoElement );
        m_pVideoElement = nullptr;
    }
}

void Game::LoadToState( GameState state )
{
    m_pLoadingScreen->Show( true );
    m_LoadToState = state;
}

void Game::SetupBackgrounds()
{
    m_Backgrounds.push_back( BackgroundInfo( 0, "CarinaNebula.jpg", Genesis::Color( 0.04f, 0.09f, 0.11f ) ) );
    m_Backgrounds.push_back( BackgroundInfo( 1, "Tannhauser.jpg", Genesis::Color( 0.04f, 0.09f, 0.03f ) ) );
    m_Backgrounds.push_back( BackgroundInfo( 2, "TheRim.jpg", Genesis::Color( 0.24f, 0.06f, 0.19f ) ) );
    m_Backgrounds.push_back( BackgroundInfo( 3, "DepthsOfSpace.jpg", Genesis::Color( 0.04f, 0.04f, 0.08f ) ) );
    m_Backgrounds.push_back( BackgroundInfo( 4, "HeartOfFire.jpg", Genesis::Color( 0.03f, 0.09f, 0.16f ) ) );
    m_Backgrounds.push_back( BackgroundInfo( 5, "Gardens.jpg", Genesis::Color( 0.29f, 0.17f, 0.34f ) ) );
    m_Backgrounds.push_back( BackgroundInfo( 6, "TheEnd.jpg", Genesis::Color( 0.06f, 0.08f, 0.20f ) ) );
}

void Game::SetupFactions()
{
    for ( int i = 0; i < (int)FactionId::Count; ++i )
    {
        delete m_pFaction[ i ];
    }

    // The neutral faction initially owns all the sectors but deploys no roaming fleets.
    // As the "beginner's target" faction, they field no capital ships.
    FactionInfo infoNeutral;
    infoNeutral.m_Name = "Neutral";
    infoNeutral.m_Colors[ (int)FactionColorId::Base ] = Genesis::Color( 1.0f, 1.0f, 1.0f );
    infoNeutral.m_Colors[ (int)FactionColorId::Primary ] = Genesis::Color( 0.7f, 0.7f, 0.7f );
    infoNeutral.m_Colors[ (int)FactionColorId::Secondary ] = Genesis::Color( 1.0f, 1.0f, 1.0f );
    infoNeutral.m_Colors[ (int)FactionColorId::PrimaryFlagship ] = Genesis::Color( 0.0f, 0.0f, 0.0f );
    infoNeutral.m_Colors[ (int)FactionColorId::SecondaryFlagship ] = Genesis::Color( 0.0f, 0.0f, 0.0f );
    infoNeutral.m_Colors[ (int)FactionColorId::Glow ] = Genesis::Color( 1.0f, 0.8f, 0.8f );
    infoNeutral.m_Colors[ (int)FactionColorId::GlowFlagship ] = Genesis::Color( 1.0f, 0.8f, 0.8f );
    infoNeutral.m_Colors[ (int)FactionColorId::FleetChevron ] = Genesis::Color( 1.0f, 1.0f, 1.0f );
    infoNeutral.m_BaseFleetPoints = 1000; // Unused
    infoNeutral.m_SectorToShipyardRatio = 0.0f;
    infoNeutral.m_Doctrine = FleetDoctrine( FleetBehaviourType::None, 0.75f, 0.25f, 0.0f );
    infoNeutral.m_LootProbability = LootProbability( 0.75f, 0.25f, 0.00f, 0.00f, 0.00f );
    infoNeutral.m_LootProbabilityFlagship = LootProbability( 0.00f, 0.89f, 0.10f, 0.01f, 0.00f );
    infoNeutral.m_LootProbabilityFlagshipHc = LootProbability( 0.00f, 0.78f, 0.20f, 0.02f, 0.00f );
    infoNeutral.m_ThreatValueMultiplier = 1.0f;
    infoNeutral.m_ConquestReward = 200;
    infoNeutral.m_UsesFormations = false;
    infoNeutral.m_RegionalFleetMultiplier = 1.0f;
    infoNeutral.m_SpawnsFlagshipInGalaxy = false;
    m_pFaction[ (int)FactionId::Neutral ] = new NeutralFaction( infoNeutral );

    // The Player faction contains a single, player-controlled fleet.
    FactionInfo infoPlayer;
    infoPlayer.m_Name = "Player";
    infoPlayer.m_Colors[ (int)FactionColorId::Base ] = Genesis::Color( 0.0f, 1.0f, 1.0f );
    infoPlayer.m_Colors[ (int)FactionColorId::Primary ] = Genesis::Color( 0.0f, 0.2f, 0.6f );
    infoPlayer.m_Colors[ (int)FactionColorId::Secondary ] = Genesis::Color( 1.0f, 1.0f, 1.0f );
    infoPlayer.m_Colors[ (int)FactionColorId::PrimaryFlagship ] = Genesis::Color( 0.0f, 0.0f, 0.0f );
    infoPlayer.m_Colors[ (int)FactionColorId::SecondaryFlagship ] = Genesis::Color( 1.0f, 1.0f, 1.0f );
    infoPlayer.m_Colors[ (int)FactionColorId::Glow ] = Genesis::Color( 0.0f, 1.0f, 1.0f );
    infoPlayer.m_Colors[ (int)FactionColorId::GlowFlagship ] = Genesis::Color( 0.0f, 1.0f, 1.0f );
    infoPlayer.m_Colors[ (int)FactionColorId::FleetChevron ] = Genesis::Color( 0.0f, 1.0f, 1.0f );
    infoPlayer.m_BaseFleetPoints = 1000; // Unused
    infoPlayer.m_SectorToShipyardRatio = 1.0f;
    infoPlayer.m_Doctrine = FleetDoctrine( FleetBehaviourType::None, 1.0f, 0.0f, 0.0f );
    infoPlayer.m_LootProbability = LootProbability( 0.00f, 0.00f, 0.00f, 0.00f, 0.00f );
    infoPlayer.m_LootProbabilityFlagship = LootProbability( 0.00f, 0.00f, 0.00f, 0.00f, 0.00f );
    infoPlayer.m_LootProbabilityFlagshipHc = LootProbability( 0.00f, 0.00f, 0.00f, 0.00f, 0.00f );
    infoPlayer.m_ThreatValueMultiplier = 1.0f;
    infoPlayer.m_ConquestReward = 0;
    infoPlayer.m_UsesFormations = true;
    infoPlayer.m_RegionalFleetMultiplier = 1.0f;
    infoPlayer.m_SpawnsFlagshipInGalaxy = false;
    m_pFaction[ (int)FactionId::Player ] = new Faction( infoPlayer, FactionId::Player );

    // The Empire deploys large fleets from their few shipyards, with a preference for large number of capitals.
    FactionInfo infoEmpire;
    infoEmpire.m_Name = "Empire";
    infoEmpire.m_Colors[ (int)FactionColorId::Base ] = Genesis::Color( 0.0f, 0.0f, 1.0f );
    infoEmpire.m_Colors[ (int)FactionColorId::Primary ] = Genesis::Color( 0.0f, 0.2f, 0.6f );
    infoEmpire.m_Colors[ (int)FactionColorId::Secondary ] = Genesis::Color( 1.0f, 1.0f, 1.0f );
    infoEmpire.m_Colors[ (int)FactionColorId::PrimaryFlagship ] = Genesis::Color( 0.0f, 0.0f, 0.0f );
    infoEmpire.m_Colors[ (int)FactionColorId::SecondaryFlagship ] = Genesis::Color( 1.0f, 1.0f, 1.0f );
    infoEmpire.m_Colors[ (int)FactionColorId::Glow ] = Genesis::Color( 0.0f, 1.0f, 1.0f );
    infoEmpire.m_Colors[ (int)FactionColorId::GlowFlagship ] = Genesis::Color( 0.0f, 1.0f, 1.0f );
    infoEmpire.m_Colors[ (int)FactionColorId::FleetChevron ] = Genesis::Color( 0.2f, 0.2f, 1.0f );
    infoEmpire.m_BaseFleetPoints = 1000;
    infoEmpire.m_SectorToShipyardRatio = 0.10f;
    infoEmpire.m_Doctrine = FleetDoctrine( FleetBehaviourType::Expansionist, 0.2f, 0.2f, 0.6f );
    infoEmpire.m_LootProbability = LootProbability( 0.00f, 0.00f, 0.00f, 0.00f, 0.00f );
    infoEmpire.m_LootProbabilityFlagship = LootProbability( 0.00f, 0.00f, 0.00f, 0.00f, 0.00f );
    infoEmpire.m_LootProbabilityFlagshipHc = LootProbability( 0.00f, 0.00f, 0.00f, 0.00f, 0.00f );
    infoEmpire.m_ThreatValueMultiplier = 1.0f;
    infoEmpire.m_ConquestReward = 0;
    infoEmpire.m_UsesFormations = true;
    infoEmpire.m_RegionalFleetMultiplier = 1.0f;
    infoEmpire.m_SpawnsFlagshipInGalaxy = false;
    m_pFaction[ (int)FactionId::Empire ] = new EmpireFaction( infoEmpire );

    // The Ascent have a large number of medium sized, medium weight fleets. The practical effect of this is that they tend to swarm in
    // during a battle, with fleets joining in mid-fight.
    FactionInfo infoAscent;
    infoAscent.m_Name = "Ascent";
    infoAscent.m_Colors[ (int)FactionColorId::Base ] = Genesis::Color( 1.0f, 0.3f, 0.0f );
    infoAscent.m_Colors[ (int)FactionColorId::Primary ] = Genesis::Color( 1.0f, 0.3f, 0.0f );
    infoAscent.m_Colors[ (int)FactionColorId::Secondary ] = Genesis::Color( 1.0f, 1.0f, 1.0f );
    infoAscent.m_Colors[ (int)FactionColorId::PrimaryFlagship ] = Genesis::Color( 0.0f, 0.0f, 0.0f );
    infoAscent.m_Colors[ (int)FactionColorId::SecondaryFlagship ] = Genesis::Color( 1.0f, 1.0f, 1.0f );
    infoAscent.m_Colors[ (int)FactionColorId::Glow ] = Genesis::Color( 1.0f, 0.3f, 0.0f );
    infoAscent.m_Colors[ (int)FactionColorId::GlowFlagship ] = Genesis::Color( 1.0f, 0.3f, 0.0f );
    infoAscent.m_Colors[ (int)FactionColorId::FleetChevron ] = Genesis::Color( 1.0f, 0.3f, 0.0f );
    infoAscent.m_BaseFleetPoints = 1200;
    infoAscent.m_SectorToShipyardRatio = 0.15f;
    infoAscent.m_Doctrine = FleetDoctrine( FleetBehaviourType::Expansionist, 1.0f, 1.0f, 0.25f );
    infoAscent.m_LootProbability = LootProbability( 0.00f, 0.20f, 0.70f, 0.10f, 0.00f );
    infoAscent.m_LootProbabilityFlagship = LootProbability( 0.00f, 0.00f, 0.00f, 0.75f, 0.25f );
    infoAscent.m_LootProbabilityFlagshipHc = LootProbability( 0.00f, 0.00f, 0.00f, 0.00f, 1.00f );
    infoAscent.m_ThreatValueMultiplier = 3.0f;
    infoAscent.m_ConquestReward = 800;
    infoAscent.m_CollapseTag = sKillAscentFlagshipCompleted;
    infoAscent.m_UsesFormations = true;
    infoAscent.m_FlagshipFleetShips.push_back( "special_flagship" );
    infoAscent.m_FlagshipFleetShips.push_back( "capital1" );
    infoAscent.m_FlagshipFleetShips.push_back( "capital1" );
    infoAscent.m_FlagshipFleetShips.push_back( "battlecruiser1" );
    infoAscent.m_FlagshipFleetShips.push_back( "battlecruiser1" );
    infoAscent.m_RegionalFleetMultiplier = 1.5f;
    infoAscent.m_SpawnsFlagshipInGalaxy = true;
    m_pFaction[ (int)FactionId::Ascent ] = new Faction( infoAscent, FactionId::Ascent );

    // Pirates prefer small fleets but plenty of them. They aren't capable of fielding capital ships.
    FactionInfo infoPirate;
    infoPirate.m_Name = "Pirate";
    infoPirate.m_Colors[ (int)FactionColorId::Base ] = Genesis::Color( 0.5f, 0.2f, 0.0f );
    infoPirate.m_Colors[ (int)FactionColorId::Primary ] = Genesis::Color( 0.5f, 0.2f, 0.0f );
    infoPirate.m_Colors[ (int)FactionColorId::Secondary ] = Genesis::Color( 0.0f, 0.0f, 0.0f );
    infoPirate.m_Colors[ (int)FactionColorId::PrimaryFlagship ] = Genesis::Color( 0.0f, 0.0f, 0.0f );
    infoPirate.m_Colors[ (int)FactionColorId::SecondaryFlagship ] = Genesis::Color( 0.5f, 0.2f, 0.0f );
    infoPirate.m_Colors[ (int)FactionColorId::Glow ] = Genesis::Color( 0.5f, 0.2f, 0.0f );
    infoPirate.m_Colors[ (int)FactionColorId::GlowFlagship ] = Genesis::Color( 0.7f, 0.4f, 0.0f );
    infoPirate.m_Colors[ (int)FactionColorId::FleetChevron ] = Genesis::Color( 0.6f, 0.3f, 0.1f );
    infoPirate.m_BaseFleetPoints = 800;
    infoPirate.m_SectorToShipyardRatio = 1.0f;
    infoPirate.m_Doctrine = FleetDoctrine( FleetBehaviourType::Roaming, 0.8f, 0.2f, 0.0f );
    infoPirate.m_LootProbability = LootProbability( 0.30f, 0.60f, 0.10f, 0.00f, 0.00f );
    infoPirate.m_LootProbabilityFlagship = LootProbability( 0.00f, 0.00f, 0.00f, 0.95f, 0.05f );
    infoPirate.m_LootProbabilityFlagshipHc = LootProbability( 0.00f, 0.00f, 0.00f, 0.60f, 0.40f );
    infoPirate.m_ThreatValueMultiplier = 1.5f;
    infoPirate.m_ConquestReward = 400;
    infoPirate.m_CollapseTag = sKillPirateFlagshipCompleted;
    infoPirate.m_UsesFormations = false;
    infoPirate.m_FlagshipFleetShips.push_back( "special_flagship" );
    infoPirate.m_FlagshipFleetShips.push_back( "pirate_battlecruiser_2" );
    infoPirate.m_FlagshipFleetShips.push_back( "pirate_battlecruiser_2" );
    infoPirate.m_RegionalFleetMultiplier = 1.0f;
    infoPirate.m_SpawnsFlagshipInGalaxy = true;
    m_pFaction[ (int)FactionId::Pirate ] = new PirateFaction( infoPirate ); // Custom faction due to breaking the rules and respawning on neutral sectors

    // Marauders field capital-heavy fleets
    FactionInfo infoMarauders;
    infoMarauders.m_Name = "Marauders";
    infoMarauders.m_Colors[ (int)FactionColorId::Base ] = Genesis::Color( 1.0f, 0.0f, 0.0f );
    infoMarauders.m_Colors[ (int)FactionColorId::Primary ] = Genesis::Color( 0.7f, 0.0f, 0.0f );
    infoMarauders.m_Colors[ (int)FactionColorId::Secondary ] = Genesis::Color( 0.0f, 0.0f, 0.0f );
    infoMarauders.m_Colors[ (int)FactionColorId::PrimaryFlagship ] = Genesis::Color( 0.7f, 0.0f, 0.0f );
    infoMarauders.m_Colors[ (int)FactionColorId::SecondaryFlagship ] = Genesis::Color( 0.7f, 0.4f, 0.1f );
    infoMarauders.m_Colors[ (int)FactionColorId::Glow ] = Genesis::Color( 0.75f, 0.0f, 0.0f );
    infoMarauders.m_Colors[ (int)FactionColorId::GlowFlagship ] = Genesis::Color( 1.00f, 0.10f, 0.10f );
    infoMarauders.m_Colors[ (int)FactionColorId::FleetChevron ] = Genesis::Color( 1.0f, 0.0f, 0.0f );
    infoMarauders.m_BaseFleetPoints = 1600;
    infoMarauders.m_SectorToShipyardRatio = 0.15f;
    infoMarauders.m_Doctrine = FleetDoctrine( FleetBehaviourType::Raiding, 0.0f, 0.5f, 1.0f );
    infoMarauders.m_LootProbability = LootProbability( 0.00f, 0.40f, 0.55f, 0.05f, 0.00f );
    infoMarauders.m_LootProbabilityFlagship = LootProbability( 0.00f, 0.00f, 0.00f, 0.90f, 0.10f );
    infoMarauders.m_LootProbabilityFlagshipHc = LootProbability( 0.00f, 0.00f, 0.00f, 0.25f, 0.75f );
    infoMarauders.m_ThreatValueMultiplier = 2.5f;
    infoMarauders.m_ConquestReward = 1000;
    infoMarauders.m_CollapseTag = sKillMarauderFlagshipCompleted;
    infoMarauders.m_UsesFormations = false;
    infoMarauders.m_FlagshipFleetShips.push_back( "special_flagship" );
    infoMarauders.m_FlagshipFleetShips.push_back( "marauder_capital_1" );
    infoMarauders.m_FlagshipFleetShips.push_back( "marauder_capital_1" );
    infoMarauders.m_FlagshipFleetShips.push_back( "marauder_battlecruiser_2" );
    infoMarauders.m_FlagshipFleetShips.push_back( "marauder_battlecruiser_2" );
    infoMarauders.m_RegionalFleetMultiplier = 1.25f;
    infoMarauders.m_SpawnsFlagshipInGalaxy = false;
    m_pFaction[ (int)FactionId::Marauders ] = new Faction( infoMarauders, FactionId::Marauders );

    // The Iriani rely on a mix of battlecruisers and battleships. They field few but very hard hitting ships.
    FactionInfo infoIriani;
    infoIriani.m_Name = "Iriani";
    infoIriani.m_Colors[ (int)FactionColorId::Base ] = Genesis::Color( 0.8f, 0.0f, 1.0f );
    infoIriani.m_Colors[ (int)FactionColorId::Primary ] = Genesis::Color( 0.6f, 0.0f, 1.0f );
    infoIriani.m_Colors[ (int)FactionColorId::Secondary ] = Genesis::Color( 1.0f, 1.0f, 1.0f );
    infoIriani.m_Colors[ (int)FactionColorId::PrimaryFlagship ] = Genesis::Color( 1.0f, 1.0f, 1.0f );
    infoIriani.m_Colors[ (int)FactionColorId::SecondaryFlagship ] = Genesis::Color( 0.6f, 0.0f, 1.0f );
    infoIriani.m_Colors[ (int)FactionColorId::Glow ] = Genesis::Color( 0.4f, 0.0f, 1.0f );
    infoIriani.m_Colors[ (int)FactionColorId::GlowFlagship ] = Genesis::Color( 0.6f, 0.1f, 1.0f );
    infoIriani.m_Colors[ (int)FactionColorId::FleetChevron ] = Genesis::Color( 0.8f, 0.0f, 1.0f );
    infoIriani.m_BaseFleetPoints = 1200;
    infoIriani.m_SectorToShipyardRatio = 0.15f;
    infoIriani.m_Doctrine = FleetDoctrine( FleetBehaviourType::Expansionist, 0.0f, 1.0f, 1.0f );
    infoIriani.m_LootProbability = LootProbability( 0.00f, 0.00f, 0.80f, 0.15f, 0.05f );
    infoIriani.m_LootProbabilityFlagship = LootProbability( 0.00f, 0.00f, 0.00f, 0.00f, 1.00f );
    infoIriani.m_LootProbabilityFlagshipHc = LootProbability( 0.00f, 0.00f, 0.00f, 0.00f, 1.00f );
    infoIriani.m_ThreatValueMultiplier = 5.0f;
    infoIriani.m_ConquestReward = 2000;
    infoIriani.m_CollapseTag = sIrianiArcFinished;
    infoIriani.m_UsesFormations = true;
    infoIriani.m_RegionalFleetMultiplier = 2.0f;
    infoIriani.m_SpawnsFlagshipInGalaxy = true;
    m_pFaction[ (int)FactionId::Iriani ] = new IrianiFaction( infoIriani );

    // Special faction for one-off events
    FactionInfo infoSpecial;
    infoSpecial.m_Name = "Special";
    infoSpecial.m_Colors[ (int)FactionColorId::Base ] = Genesis::Color( 0.0f, 0.0f, 0.0f );
    infoSpecial.m_Colors[ (int)FactionColorId::Primary ] = Genesis::Color( 0.0f, 0.0f, 0.8f );
    infoSpecial.m_Colors[ (int)FactionColorId::Secondary ] = Genesis::Color( 0.0f, 0.0f, 0.0f );
    infoSpecial.m_Colors[ (int)FactionColorId::PrimaryFlagship ] = Genesis::Color( 0.0f, 0.0f, 0.8f );
    infoSpecial.m_Colors[ (int)FactionColorId::SecondaryFlagship ] = Genesis::Color( 0.0f, 0.0f, 0.0f );
    infoSpecial.m_Colors[ (int)FactionColorId::Glow ] = Genesis::Color( 0.0f, 0.5f, 1.0f );
    infoSpecial.m_Colors[ (int)FactionColorId::GlowFlagship ] = Genesis::Color( 0.0f, 0.5f, 1.0f );
    infoSpecial.m_Colors[ (int)FactionColorId::FleetChevron ] = Genesis::Color( 0.0f, 0.0f, 0.0f );
    infoSpecial.m_BaseFleetPoints = 1000;
    infoSpecial.m_SectorToShipyardRatio = 0.0f;
    infoSpecial.m_Doctrine = FleetDoctrine( FleetBehaviourType::Defensive, 1.0f, 1.0f, 1.0f );
    infoSpecial.m_LootProbability = LootProbability( 0.00f, 0.00f, 0.00f, 0.00f, 0.00f );
    infoSpecial.m_LootProbabilityFlagship = LootProbability( 0.00f, 0.00f, 0.00f, 0.00f, 0.00f );
    infoSpecial.m_LootProbabilityFlagshipHc = LootProbability( 0.00f, 0.00f, 0.00f, 0.00f, 0.00f );
    infoSpecial.m_ConquestReward = 0;
    infoSpecial.m_UsesFormations = true;
    infoSpecial.m_SpawnsFlagshipInGalaxy = false;
    m_pFaction[ (int)FactionId::Special ] = new Faction( infoSpecial, FactionId::Special );

    // Hegemon field capital-heavy fleets
    FactionInfo infoHegemon;
    infoHegemon.m_Name = "Hegemon";
    infoHegemon.m_Colors[ (int)FactionColorId::Base ] = Genesis::Color( 0.0f, 1.0f, 0.0f );
    infoHegemon.m_Colors[ (int)FactionColorId::Primary ] = Genesis::Color( 0.0f, 1.0f, 0.0f );
    infoHegemon.m_Colors[ (int)FactionColorId::Secondary ] = Genesis::Color( 0.0f, 0.0f, 0.0f );
    infoHegemon.m_Colors[ (int)FactionColorId::PrimaryFlagship ] = Genesis::Color( 0.0f, 0.0f, 0.0f );
    infoHegemon.m_Colors[ (int)FactionColorId::SecondaryFlagship ] = Genesis::Color( 0.0f, 0.0f, 0.0f );
    infoHegemon.m_Colors[ (int)FactionColorId::Glow ] = Genesis::Color( 0.00f, 1.0f, 0.0f );
    infoHegemon.m_Colors[ (int)FactionColorId::GlowFlagship ] = Genesis::Color( 0.00f, 1.0f, 0.0f );
    infoHegemon.m_Colors[ (int)FactionColorId::FleetChevron ] = Genesis::Color( 0.0f, 1.0f, 0.0f );
    infoHegemon.m_BaseFleetPoints = 1600;
    infoHegemon.m_SectorToShipyardRatio = 0.1f;
    infoHegemon.m_Doctrine = FleetDoctrine( FleetBehaviourType::Expansionist, 0.0f, 0.5f, 1.0f );
    infoHegemon.m_LootProbability = LootProbability( 0.00f, 0.40f, 0.55f, 0.05f, 0.00f );
    infoHegemon.m_LootProbabilityFlagship = LootProbability( 0.00f, 0.00f, 0.00f, 0.90f, 0.10f );
    infoHegemon.m_LootProbabilityFlagshipHc = LootProbability( 0.00f, 0.00f, 0.00f, 0.25f, 0.75f );
    infoHegemon.m_ThreatValueMultiplier = 7.5f;
    infoHegemon.m_ConquestReward = 4000;
    infoHegemon.m_UsesFormations = true;
    infoHegemon.m_FlagshipFleetShips.push_back( "special_flagship" );
    infoHegemon.m_FlagshipFleetShips.push_back( "hegemon_capital_3" );
    infoHegemon.m_FlagshipFleetShips.push_back( "hegemon_capital_3" );
    infoHegemon.m_FlagshipFleetShips.push_back( "hegemon_battlecruiser_2" );
    infoHegemon.m_FlagshipFleetShips.push_back( "hegemon_battlecruiser_2" );
    infoHegemon.m_RegionalFleetMultiplier = 1.25f;
    infoHegemon.m_SpawnsFlagshipInGalaxy = true;
    m_pFaction[ (int)FactionId::Hegemon ] = new Faction( infoHegemon, FactionId::Hegemon );
}

Faction* Game::GetFaction( const std::string& name ) const
{
    for ( int i = 0; i < (int)FactionId::Count; ++i )
    {
        if ( m_pFaction[ i ] && m_pFaction[ i ]->GetName() == name )
            return m_pFaction[ i ];
    }
    return nullptr;
}

Faction* Game::GetPlayerFaction() const
{
    return m_pFaction[ (int)FactionId::Player ];
}

FleetWeakPtr Game::GetPlayerFleet() const
{
    Faction* pPlayerFaction = GetPlayerFaction();
    if ( pPlayerFaction != nullptr )
    {
        const FleetList& fleets = pPlayerFaction->GetFleets();
        if ( fleets.empty() == false )
        {
            return fleets.back();
        }
    }
    return FleetWeakPtr();
}

void Game::InitialiseSectorEvents()
{
    if ( m_SectorEvents.empty() == false )
    {
        for ( auto& pSectorEvent : m_SectorEvents )
        {
            delete pSectorEvent;
        }
        m_SectorEvents.clear();
    }

    m_SectorEvents.push_back( new SectorEventChrysamere() );
    m_SectorEvents.push_back( new SectorEventOrbitalDefenses() );
    m_SectorEvents.push_back( new SectorEventNeutralFlagship() );
    m_SectorEvents.push_back( new SectorEventCorsairFleet() );
}

void Game::EnterSector( SectorInfo* pSectorInfo )
{
    SDL_assert( pSectorInfo != nullptr );

    // This should never happen. The only way for this not to be nullptr is if we've entered a sector but didn't exit it.
    SDL_assert( m_pSector == nullptr );

    // Disable auto-resolve so battles don't get processed as if we weren't fighting them...
    pSectorInfo->SetAutoResolve( false );

    // If we've just exited a sector we need to make sure our fleet isn't invulnerable before contesting again
    FleetSharedPtr pPlayerFleet = g_pGame->GetPlayerFleet().lock();
    if ( pPlayerFleet )
    {
        pPlayerFleet->SetImmunity( false );
    }

    pSectorInfo->Contest();

    m_pSector = new Sector( pSectorInfo );
    bool init = m_pSector->Initialise();
    SDL_assert( init );

    SetState( GameState::Combat );
}

void Game::ExitSector()
{
    SDL_assert( m_pSector != nullptr );

    if ( GetDifficulty() == Difficulty::Hardcore )
    {
        Ship* pShip = g_pGame->GetPlayer()->GetShip();
        if ( pShip->IsDestroyed() )
        {
            KillSaveGame();
        }
    }

    SectorInfo* pSectorInfo = m_pSector->GetSectorInfo();

    // Hack to link in the Tactical Relocation achievement, as there's no support in Sector Components for
    // detecting exiting the sector.
    if ( pSectorInfo->GetName() == "Cradle of Isaldren" && g_pGame->GetPlayer()->GetShip()->IsDestroyed() == false )
    {
        g_pGame->GetAchievementsManager()->UnlockAchievement( ACH_TACTICAL_RELOCATION );
    }

    pSectorInfo->FleetDisengaged( g_pGame->GetPlayerFleet() );

    delete m_pSector;
    m_pSector = nullptr;

    // Forcing the next turn will cause the sector to lose its contested status and allow the
    // player to start fighting again without waiting.
    GetGalaxy()->ForceNextTurn();

    SetState( GameState::GalaxyView );

    SaveGame();

    m_FirstTimeInCombat = false;
}

RequestManager* Game::GetRequestManager() const
{
    Faction* pFaction = GetFaction( FactionId::Empire );
    if ( pFaction == nullptr )
        return nullptr;

    EmpireFaction* pEmpireFaction = (EmpireFaction*)pFaction;
    return pEmpireFaction->GetRequestManager();
}

SectorInfo* Game::FindSpawnSector() const
{
    const SectorInfoVector& controlledSectors = GetFaction( FactionId::Empire )->GetControlledSectors();
    if ( controlledSectors.empty() )
    {
        return nullptr;
    }
    else if ( GetGameMode() == GameMode::Campaign )
    {
        // We give priority to the Tannhauser sector, but if we can't find it (presumably because it got conquered by another faction),
        // we'll spawn at another sector that has a shipyard in it
        SectorInfo* pBackupSectorInfo = nullptr;
        for ( auto& pSectorInfo : controlledSectors )
        {
            if ( pSectorInfo->GetName().find( "Tannhauser" ) != std::string::npos && pSectorInfo->HasShipyard() )
            {
                return pSectorInfo;
            }
            else if ( pSectorInfo->HasShipyard() )
            {
                pBackupSectorInfo = pSectorInfo;
            }
        }
        return pBackupSectorInfo;
    }
    else if ( GetGameMode() == GameMode::InfiniteWar )
    {
        for ( auto& pSectorInfo : controlledSectors )
        {
            if ( pSectorInfo->IsHomeworld() )
            {
                return pSectorInfo;
            }
        }

        Genesis::FrameWork::GetLogger()->LogError( "Failed to find homeworld sector for the Empire." );
        return nullptr;
    }
    else
    {
        return nullptr;
    }
}

void Game::AddFleetCommandIntel( const std::string& text, ModuleInfo* pModuleInfo /* = nullptr */ )
{
    if ( GetIntelWindow() != nullptr )
    {
        GetIntelWindow()->AddFragment( IntelFragment( GameCharacter::FleetIntelligence, "Fleet Intelligence Officer", text, pModuleInfo ) );
    }
}

// The "canBeQueued" parameter is meant to be used for intel that is only relevant if displayed at this very moment.
// If "canBeQueued" is false but the queue already has intel in it, the fragment will be discarded.
void Game::AddIntel( GameCharacter character, const std::string& text, bool canBeQueued /* = true */ )
{
    IntelWindow* pIntelWindow = GetIntelWindow();
    if ( pIntelWindow != nullptr )
    {
        static const std::string characterNames[ static_cast<unsigned int>( GameCharacter::Count ) ] = {
            "Fleet Intelligence Officer",
            "Navarre Hexer",
            "Harkon Stormchaser",
            "Jeroen Lightbringer",
            "Aelise Gloriam",
            "Chrysamere"
        };

        pIntelWindow->AddFragment( IntelFragment( character, characterNames[ static_cast<unsigned int>( character ) ], text, nullptr, canBeQueued ) );
    }
}

void Game::LoadResourcesAsync()
{
    m_pLoadingScreen->Show( true );
    m_LoaderThread = std::thread( &Game::LoaderThreadMain, this );
}

void Game::LoaderThreadMain()
{
    size_t currentProgress = 0;
    size_t maximumProgress = 0;

    Genesis::Window* pWindow = Genesis::FrameWork::GetWindow();
    SDL_GL_MakeCurrent( pWindow->GetSDLWindow(), pWindow->GetSDLThreadGLContext() );

    using namespace Genesis;
    ShaderCache* pShaderCache = FrameWork::GetRenderSystem()->GetShaderCache();
    std::filesystem::path shadersPath( "data/shaders" );
    for ( const auto& entry : std::filesystem::recursive_directory_iterator( shadersPath ) )
    {
        if ( entry.path().extension() == ".vert" )
        {
            pShaderCache->Load( ToString( entry.path().stem() ) );
        }
    }

    ResourceManager* pResourceManager = FrameWork::GetResourceManager();
    std::vector<std::string> filesToLoad;
    filesToLoad.reserve( 512 );
    std::filesystem::path dataPath( "data" );
    for ( const auto& entry : std::filesystem::recursive_directory_iterator( dataPath ) )
    {
        const std::string entryPath = ToString( entry.path().c_str() );
        if ( entry.is_regular_file() && pResourceManager->CanLoadResource( entryPath ) )
        {
            filesToLoad.push_back( entryPath );
        }
    }
    maximumProgress += filesToLoad.size();

    for ( const auto& entryPath : filesToLoad )
    {
        pResourceManager->GetResource( entryPath );
        m_pLoadingScreen->SetProgress( ++currentProgress, maximumProgress );
    }

    // Ensure all GPU commands are complete so our render context can use the resources
    // this thread has loaded.
    const GLsync fenceId = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );
    const GLuint64 timeout = 5000000000; // 5 second timeout
    while ( true )
    {
        GLenum result = glClientWaitSync( fenceId, GL_SYNC_FLUSH_COMMANDS_BIT, timeout );
        if ( result == GL_WAIT_FAILED )
        {
            Genesis::FrameWork::GetLogger()->LogError( "glClientWaitSync failed: GL_WAIT_FAILED." );
            exit( -1 );
        }
        else if ( result != GL_TIMEOUT_EXPIRED )
        {
            break;
        }
    }

    Genesis::FrameWork::GetLogger()->LogInfo( "All resources loaded." );

    m_AllResourcesLoaded = true;
    SDL_GL_MakeCurrent( pWindow->GetSDLWindow(), nullptr );
}

bool Game::IsShipCaptureModeActive() const
{
    return false;
}

bool Game::IsTutorialActive() const
{
    if ( m_pTutorialWindow == nullptr )
    {
        return false;
    }
    else
    {
        return m_pTutorialWindow->IsActive();
    }
}

bool Game::RequisitionShip( const ShipInfo* pShipInfo )
{
    Player* pPlayer = GetPlayer();
    if ( pPlayer == nullptr || pPlayer->GetInfluence() < pShipInfo->GetCost() )
        return false;

    FleetSharedPtr pPlayerFleet = GetPlayerFleet().lock();
    if ( pPlayerFleet != nullptr && pPlayerFleet->GetShips().size() + 1 < pPlayer->GetFleetMaxShips() )
    {
        pPlayerFleet->AddShip( pShipInfo );
        pPlayer->SetInfluence( pPlayer->GetInfluence() - pShipInfo->GetCost() );

        if ( pShipInfo->GetDisplayName() == "Arbellatris" )
        {
            g_pGame->GetAchievementsManager()->UnlockAchievement( ACH_WALK_SLOWLY );
        }

        return true;
    }

    return false;
}

bool Game::ReturnShip( const ShipInfo* pShipInfo )
{
    Player* pPlayer = GetPlayer();
    if ( pPlayer == nullptr )
    {
        return false;
    }

    FleetSharedPtr pPlayerFleet = GetPlayerFleet().lock();
    if ( pPlayerFleet != nullptr && pPlayerFleet->RemoveShip( pShipInfo ) )
    {
        pPlayer->SetInfluence( pPlayer->GetInfluence() + pShipInfo->GetCost() );
        return true;
    }
    else
    {
        return false;
    }
}

void Game::RaiseInteractiveWarning( const std::string& text ) const
{
    Popup* pPopup = GetPopup();
    if ( pPopup != nullptr )
    {
        pPopup->Show( PopupMode::Ok, text );
    }

    Genesis::FrameWork::GetLogger()->LogWarning( "%s", text.c_str() );
}

void Game::SetCursorType( CursorType type )
{
    using namespace Genesis;
    ResourceImage* pCursorTexture = nullptr;
    if ( type == CursorType::Pointer )
    {
        pCursorTexture = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/ui/cursor.png" );
    }
    else if ( type == CursorType::Crosshair )
    {
        pCursorTexture = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/ui/crosshair.png" );
    }

    FrameWork::GetGuiManager()->GetCursor()->SetTexture( pCursorTexture );
    m_CursorType = type;
}

void Game::ShowCursor( bool state )
{
    Genesis::FrameWork::GetGuiManager()->GetCursor()->Show( state );
}

void Game::Pause()
{
    m_IsPaused = true;

    if ( GetPhysicsSimulation() != nullptr )
    {
        GetPhysicsSimulation()->Pause( true );
    }
}

void Game::Unpause()
{
    m_IsPaused = false;

    if ( GetPhysicsSimulation() != nullptr )
    {
        GetPhysicsSimulation()->Pause( false );
    }
}

void Game::LoadUIDesigns()
{
    LoadUIDesign( UIDesignId::MainMenu, "data/ui/designs/main_menu.json" );
    LoadUIDesign( UIDesignId::Shipyard, "data/ui/designs/shipyard.json" );
}

void Game::LoadUIDesign( UIDesignId id, const std::filesystem::path& designPath )
{
    const size_t idx = static_cast<size_t>( id );
    SDL_assert( m_UIRootElements[ idx ] == nullptr );
    m_UIRootElements[ idx ] = std::make_unique<UI::RootElement>( designPath );
}

//-------------------------------------------------------------------
// Main
//-------------------------------------------------------------------

int Main( Genesis::CommandLineParameters* parameters )
{
    using namespace Genesis;
    FrameWork::Initialize();

    FrameWork::CreateWindowGL(
        "HEXTERMINATE",
        Configuration::GetScreenWidth(),
        Configuration::GetScreenHeight(),
        Configuration::GetMultiSampleSamples() );

    TaskManager* taskManager = FrameWork::GetTaskManager();

    g_pGame = new Game();
    g_pGame->Initialise();

    while ( taskManager->IsRunning() && g_pGame->IsQuitRequested() == false )
    {
        taskManager->Update();
    }

    Configuration::Save();

    delete g_pGame;
    delete parameters;
    FrameWork::Shutdown();

    return 0;
}

} // namespace Hexterminate

//-----------------------------------------------------------------------------
// Entry point
//-----------------------------------------------------------------------------

int main( int argc, char* argv[] )
{
    return Hexterminate::Main( Genesis::FrameWork::CreateCommandLineParameters( (const char**)argv, (int)argc ) );
}
