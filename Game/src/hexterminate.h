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
#include <filesystem>
#include <list>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <taskmanager.h>

#if USE_STEAM
// clang-format off
#include "beginexternalheaders.h"
#include "steam/steam_api.h"
#include "endexternalheaders.h"
// clang-format on
#endif

#include "blackboard.h"
#include "faction/faction.h"
#include "menus/cursortype.h"
#include "menus/musictitle.h"
#include "sector/events/sectorevent.h"
#include "sector/galaxycreationinfo.h"
#include "ship/ship.h"

#ifndef HEXTERMINATE_BUILD_VERSION
#if USE_STEAM
#define HEXTERMINATE_BUILD_VERSION "DEV (Steam)"
#else
#define HEXTERMINATE_BUILD_VERSION "DEV"
#endif // USE_STEAM
#endif // HEXTERMINATE_BUILD_VERSION

namespace Genesis
{

struct Material;
class ResourceSound;

namespace Gui
{
    class Text;
    class Video;
} // namespace Gui

namespace Physics
{
    class Simulation;
}

} // namespace Genesis

namespace Hexterminate
{

namespace UI
{
    class Editor;
    using EditorUniquePtr = std::unique_ptr<Editor>;
    class RootElement;
    using RootElementUniquePtr = std::unique_ptr<RootElement>;
} // namespace UI

class Sector;
class Game;
class ModuleInfoManager;
class ModuleInfo;
class ShipInfoManager;
class MainMenu;
class Console;
class Player;
class Hotbar;
class IntelWindow;
class Galaxy;
class Fleet;
class HyperspaceMenu;
class Popup;
class MusicTitle;
class TutorialWindow;
class RequestManager;
class BackgroundInfo;
class Perks;
class LoadingScreen;
class AchievementsManager;
class SaveGameStorage;
class ShipOutline;
class Hyperscape;
GENESIS_DECLARE_SMART_PTR( SaveGameHeader );

extern Game* g_pGame;

using StringVector = std::vector<std::string>;
using WStringVector = std::vector<std::wstring>;
using BackgroundInfoVector = std::vector<BackgroundInfo>;
using LoadingScreenUniquePtr = std::unique_ptr<LoadingScreen>;
using ShipOutlineUniquePtr = std::unique_ptr<ShipOutline>;

struct NewGameParameters;

enum class GameCharacter
{
    FleetIntelligence,
    NavarreHexer,
    HarkonStormchaser,
    JeroenLightbringer,
    AeliseGloriam,
    Chrysamere,

    Count
};

enum class Difficulty
{
    Easy,
    Normal,
    Hardcore
};

enum class GameMode
{
    Campaign,
    InfiniteWar,
    Hyperscape
};

///////////////////////////////////////////////////////////////////////////
// Game
///////////////////////////////////////////////////////////////////////////

enum class GameState
{
    Intro = 0,
    LoadResources,
    Menu,
    GalaxyView,
    HyperscapeView,
    Shipyard,
    Combat,
    Unknown
};

enum class UIDesignId
{
    MainMenu = 0,
    GalaxyView,

    Count
};

class Game : public Genesis::Task
{
public:
    Game();
    ~Game();
    void Initialise();
    Genesis::TaskStatus Update( float delta );

    Genesis::Physics::Simulation* GetPhysicsSimulation() const;
    Sector* GetCurrentSector() const;
    ModuleInfoManager* GetModuleInfoManager() const;
    ShipInfoManager* GetShipInfoManager() const;
    Faction* GetFaction( const std::string& name ) const;
    Faction* GetFaction( FactionId faction ) const;
    Faction* GetPlayerFaction() const;
    Player* GetPlayer() const;
    FleetWeakPtr GetPlayerFleet() const;
    Galaxy* GetGalaxy() const;
    IntelWindow* GetIntelWindow() const;
    Popup* GetPopup() const;
    TutorialWindow* GetTutorialWindow() const;
    RequestManager* GetRequestManager() const;
    SectorInfo* FindSpawnSector() const;
    BlackboardSharedPtr GetBlackboard() const;
    Perks* GetNPCPerks() const;
    AchievementsManager* GetAchievementsManager() const;
    SaveGameStorage* GetSaveGameStorage() const;
    ShipOutline* GetShipOutline() const;
    Difficulty GetDifficulty() const;
    void SetDifficulty( Difficulty difficulty );
    GameMode GetGameMode() const;
    void SetGameMode( GameMode mode );

    void RaiseInteractiveWarning( const std::string& text ) const;
    void SetCursorType( CursorType type );
    CursorType GetCursorType() const;
    void ShowCursor( bool state );

    void EnterSector( SectorInfo* pSectorInfo );
    void ExitSector();

    void StartNewLegacyGame( const ShipCustomisationData& customisationData, const std::string& companionShipTemplate, bool tutorial, const GalaxyCreationInfo& galaxyCreationInfo );
    void StartNewHyperscapeGame( const ShipCustomisationData& customisationData, bool tutorial );
    void EndGame();
    void KillSaveGame();
    bool SaveGame();
    void LoadGame( SaveGameHeaderWeakPtr pSaveGameHeader );
    void LoadToState( GameState state );
    void SetState( GameState state );
    GameState GetState() const;

    void AddFleetCommandIntel( const std::string& text, ModuleInfo* pModuleInfo = nullptr );
    void AddIntel( GameCharacter character, const std::string& text, bool canBeQueued = true );
    float GetPlayedTime() const;
    void SetPlayedTime( float time );

    void InitialiseSectorEvents();
    const SectorEventVector& GetSectorEvents();

    bool IsShipCaptureModeActive() const;
    bool IsTutorialActive() const;
    bool IsFirstTimeInCombat() const;
    bool AreContextualTipsEnabled() const;

    bool RequisitionShip( const ShipInfo* pShipInfo );
    bool ReturnShip( const ShipInfo* pShipInfo );

    void Pause();
    void Unpause();
    bool IsPaused() const;

    void SetInputBlocked( bool state );
    bool IsInputBlocked() const;

    const BackgroundInfoVector& GetBackgrounds() const;

    void Quit();
    bool IsQuitRequested() const;

    UI::RootElement* GetUIRoot( UIDesignId designId ) const;
    std::array<UI::RootElement*, static_cast<size_t>( UIDesignId::Count )> GetUIRoots() const;

private:
    void SetupFactions();
    void SetupNewGameTutorial();
    void LoadResourcesAsync();
    void LoaderThreadMain();
    void EndGameAux();
    void SetupBackgrounds();
    void LoadGameAux();
    void ToggleImGui();
    void LoadUIDesigns();

    MainMenu* m_pMainMenu;
    Console* m_pConsole;
    ModuleInfoManager* m_pModuleInfoManager;
    ShipInfoManager* m_pShipInfoManager;
    Sector* m_pSector;
    Player* m_pPlayer;
    Galaxy* m_pGalaxy;
    MusicTitle* m_pMusicTitle;
    TutorialWindow* m_pTutorialWindow;
    GameState m_State;
    IntelWindow* m_pIntelWindow;
    LoadingScreenUniquePtr m_pLoadingScreen;
    AchievementsManager* m_pAchievementsManager;
    ShipOutlineUniquePtr m_pShipOutline;

#ifdef _DEBUG
    Genesis::InputCallbackToken m_ImGuiToggleToken;
    Genesis::Gui::Text* m_pFrameText;
#endif // _DEBUG

    Faction* m_pFaction[ static_cast<unsigned int>( FactionId::Count ) ];
    float m_PlayedTime;
    bool m_EndGame;
    bool m_FirstTimeInCombat;

    Popup* m_pPopup;

    SectorEventVector m_SectorEvents;

    bool m_IsPaused;
    bool m_IsInputBlocked;
    float m_InputBlockedTimer;

    BlackboardSharedPtr m_pBlackboard;
    BackgroundInfoVector m_Backgrounds;

    Perks* m_pNPCPerks;

    bool m_ContextualTipsEnabled;
    bool m_QuitRequested;
    Genesis::Gui::Video* m_pVideoElement;
    CursorType m_CursorType;
    GameState m_LoadToState;
    std::filesystem::path m_GameToLoad;

    Genesis::Physics::Simulation* m_pPhysicsSimulation;

    std::unique_ptr<SaveGameStorage> m_pSaveGameStorage;
    Difficulty m_Difficulty;
    GameMode m_GameMode;
    bool m_KillSave;

    std::array<UI::RootElementUniquePtr, static_cast<size_t>( UIDesignId::Count )> m_UIRootElements;
    UI::EditorUniquePtr m_pUIEditor;
    bool m_ShowImguiTestWindow;

    std::unique_ptr<Hyperscape> m_pHyperscape;
    std::thread m_LoaderThread;
    std::atomic_bool m_AllResourcesLoaded;
};

inline Genesis::Physics::Simulation* Game::GetPhysicsSimulation() const
{
    return m_pPhysicsSimulation;
}

inline Sector* Game::GetCurrentSector() const
{
    return m_pSector;
}

inline ModuleInfoManager* Game::GetModuleInfoManager() const
{
    return m_pModuleInfoManager;
}

inline ShipInfoManager* Game::GetShipInfoManager() const
{
    return m_pShipInfoManager;
}

inline AchievementsManager* Game::GetAchievementsManager() const
{
    return m_pAchievementsManager;
}

inline ShipOutline* Game::GetShipOutline() const
{
    return m_pShipOutline.get();
}

inline Player* Game::GetPlayer() const
{
    return m_pPlayer;
}

inline Galaxy* Game::GetGalaxy() const
{
    return m_pGalaxy;
}

inline Faction* Game::GetFaction( FactionId id ) const
{
    return m_pFaction[ static_cast<int>( id ) ];
}

inline GameState Game::GetState() const
{
    return m_State;
}

inline IntelWindow* Game::GetIntelWindow() const
{
    return m_pIntelWindow;
}

inline float Game::GetPlayedTime() const
{
    return m_PlayedTime;
}

inline void Game::SetPlayedTime( float time )
{
    m_PlayedTime = time;
}

inline Difficulty Game::GetDifficulty() const
{
    return m_Difficulty;
}

inline void Game::SetDifficulty( Difficulty difficulty )
{
    m_Difficulty = difficulty;
}

inline GameMode Game::GetGameMode() const
{
    return m_GameMode;
}

inline void Game::SetGameMode( GameMode mode )
{
    m_GameMode = mode;
}

inline bool Game::IsFirstTimeInCombat() const
{
    return m_FirstTimeInCombat;
}

inline const SectorEventVector& Game::GetSectorEvents()
{
    return m_SectorEvents;
}

inline Popup* Game::GetPopup() const
{
    return m_pPopup;
}

inline TutorialWindow* Game::GetTutorialWindow() const
{
    return m_pTutorialWindow;
}

inline bool Game::IsPaused() const
{
    return m_IsPaused;
}

inline BlackboardSharedPtr Game::GetBlackboard() const
{
    return m_pBlackboard;
}

inline const BackgroundInfoVector& Game::GetBackgrounds() const
{
    return m_Backgrounds;
}

inline Perks* Game::GetNPCPerks() const
{
    return m_pNPCPerks;
}

inline bool Game::AreContextualTipsEnabled() const
{
    return m_ContextualTipsEnabled;
}

inline void Game::Quit()
{
    m_QuitRequested = true;
}

inline bool Game::IsQuitRequested() const
{
    return m_QuitRequested;
}

inline CursorType Game::GetCursorType() const
{
    return m_CursorType;
}

inline UI::RootElement* Game::GetUIRoot( UIDesignId id ) const
{
    return m_UIRootElements[ static_cast<size_t>( id ) ].get();
}

inline std::array<UI::RootElement*, static_cast<size_t>( UIDesignId::Count )> Game::GetUIRoots() const
{
    std::array<UI::RootElement*, static_cast<size_t>( UIDesignId::Count )> roots;

    for ( size_t i = 0; i < static_cast<size_t>( UIDesignId::Count ); i++ )
    {
        roots[ i ] = m_UIRootElements[ i ].get();
    }

    return roots;
}

} // namespace Hexterminate
