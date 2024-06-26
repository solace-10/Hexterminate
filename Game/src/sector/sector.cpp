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

#include <algorithm>
#include <genesis.h>
#include <inputmanager.h>
#include <logger.h>
#include <math/misc.h>
#include <render/debugrender.h>
#include <resources/resourceplaylist.h>
#include <scene/layer.h>
#include <scene/scene.h>
#include <scene/sceneobject.h>
#include <sound/soundinstance.h>
#include <sound/soundmanager.h>
#include <sstream>

// clang-format off
#include <beginexternalheaders.h>
#include <tinyxml2.h>
#include <endexternalheaders.h>
// clang-format on

#include "achievements.h"
#include "ammo/ammomanager.h"
#include "faction/faction.h"
#include "fleet/fleet.h"
#include "fleet/fleetcommand.h"
#include "fleet/fleetspawner.h"
#include "laser/lasermanager.h"
#include "menus/contextualtips.h"
#include "menus/deathmenu.h"
#include "menus/fleetstatus.h"
#include "menus/galaxywindow.h"
#include "menus/hyperspacemenu.h"
#include "menus/intelwindow.h"
#include "menus/lootwindow.h"
#include "menus/radar.h"
#include "menus/shiptweaks.h"
#include "muzzleflash/muzzleflashmanager.h"
#include "muzzleflash/muzzleflashmanagerrep.h"
#include "particles/particlemanager.h"
#include "particles/particlemanagerrep.h"
#include "player.h"
#include "requests/campaigntags.h"
#include "requests/imperialrequest.h"
#include "requests/requestmanager.h"
#include "sector.h"
#include "sector/background.h"
#include "sector/boundary.h"
#include "sector/dust.h"
#include "sector/sectorcamera.h"
#include "sector/sectorspawner.h"
#include "sector/starinfo.h"
#include "ship/collisionmasks.h"
#include "ship/damagetracker.h"
#include "ship/hyperspacecore.h"
#include "ship/ship.h"
#include "ship/shipinfo.h"
#include "shipyard/shipyard.h"
#include "sprite/spritemanager.h"
#include "trail/trailmanager.h"
#include "trail/trailmanagerrep.h"

#include "sector/events/orbitaldefenses.h"
#include "sector/events/sectorevent.h"

#include "hexterminate.h"
#include "menus/hotbar.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// Sector
///////////////////////////////////////////////////////////////////////////////

Sector::Sector( SectorInfo* pSectorInfo )
    : m_pSectorInfo( pSectorInfo )
    , m_pBackground( nullptr )
    , m_pDust( nullptr )
    , m_pBoundary( nullptr )
    , m_pAmmoManager( nullptr )
    , m_pLaserManager( nullptr )
    , m_pSpriteManager( nullptr )
    , m_pTrailManager( nullptr )
    , m_pTrailManagerRep( nullptr )
    , m_pRadar( nullptr )
    , m_pRegionalFleet( nullptr )
    , m_pShipyard( nullptr )
    , m_pSectorEvent( nullptr )
    , m_IsPlayerVictorious( false )
    , m_pLootWindow( nullptr )
    , m_AdditionalWaves( 0u )
    , m_AdditionalWavesSpawned( 0u )
    , m_TimeToNextReinforcements( 0.0f )
{
    SDL_assert( pSectorInfo != nullptr );

    Genesis::Scene* pScene = Genesis::FrameWork::GetScene();
    m_pBackgroundLayer = pScene->AddLayer( LAYER_BACKGROUND, true );
    m_pShipLayer = pScene->AddLayer( LAYER_SHIP );
    m_pFxLayer = pScene->AddLayer( LAYER_FX );
    m_pAmmoLayer = pScene->AddLayer( LAYER_AMMO );
    m_pPhysicsLayer = pScene->AddLayer( LAYER_PHYSICS );

    m_pCamera = new SectorCamera();

    m_pHyperspaceMenu = new HyperspaceMenu();
    m_pDeathMenu = new DeathMenu();
    m_pLootWindow = new LootWindow();

    m_pHotbar = std::make_unique<Hotbar>();
    m_pFleetStatus = std::make_unique<FleetStatus>();

    for ( int i = 0; i < (int)FactionId::Count; ++i )
    {
        m_TowerBonus[ i ] = TowerBonus::None;
        m_TowerBonusMagnitude[ i ] = 0.0f;
    }

    g_pGame->SetCursorType( CursorType::Crosshair );

    m_pShipTweaks = std::make_unique<ShipTweaks>();
    m_pSectorSpawner = std::make_unique<SectorSpawner>();
}

Sector::~Sector()
{
    DamageTrackerDebugWindow::Unregister();

    if ( g_pGame->GetPlayer() )
    {
        g_pGame->GetPlayer()->UnassignShip();
    }

    Genesis::Scene* pScene = Genesis::FrameWork::GetScene();
    pScene->RemoveLayer( LAYER_BACKGROUND );
    pScene->RemoveLayer( LAYER_SHIP );
    pScene->RemoveLayer( LAYER_FX );
    pScene->RemoveLayer( LAYER_AMMO );
    pScene->RemoveLayer( LAYER_PHYSICS );

    delete m_pCamera;
    delete m_pHyperspaceMenu;
    delete m_pDeathMenu;
    delete m_pLootWindow;
    delete m_pTrailManager;

    if ( m_pParticleManager != nullptr )
    {
        if ( m_pParticleManagerRep != nullptr )
        {
            m_pParticleManagerRep->SetParticleManager( nullptr );
        }
        delete m_pParticleManager;
    }

    if ( m_pMuzzleflashManager != nullptr )
    {
        if ( m_pMuzzleflashManagerRep != nullptr )
        {
            m_pMuzzleflashManagerRep->SetManager( nullptr );
        }
        delete m_pMuzzleflashManager;
    }

    if ( m_pRadar != nullptr )
    {
        Genesis::FrameWork::GetGuiManager()->RemoveElement( m_pRadar );
    }

    g_pGame->SetCursorType( CursorType::Pointer );
}

bool Sector::Initialise()
{
    SelectBackground();

    m_pTrailManager = new TrailManager();
    m_pTrailManagerRep = new TrailManagerRep( m_pTrailManager );
    m_pShipLayer->AddSceneObject( m_pTrailManagerRep );

    m_pParticleManager = new ParticleManager();
    m_pParticleManagerRep = new ParticleManagerRep( m_pParticleManager );
    m_pFxLayer->AddSceneObject( m_pParticleManagerRep );

    m_pMuzzleflashManager = new MuzzleflashManager();
    m_pMuzzleflashManagerRep = new MuzzleflashManagerRep( m_pMuzzleflashManager );
    m_pShipLayer->AddSceneObject( m_pMuzzleflashManagerRep );

    m_pDust = new Dust();
    m_pShipLayer->AddSceneObject( m_pDust );

    m_pBoundary = new Boundary();
    m_pShipLayer->AddSceneObject( m_pBoundary );

    m_pPhysicsLayer->AddSceneObject( Genesis::FrameWork::GetDebugRender(), false );

    m_pAmmoManager = new AmmoManager();
    m_pAmmoLayer->AddSceneObject( m_pAmmoManager );

    m_pLaserManager = new LaserManager();
    m_pAmmoLayer->AddSceneObject( m_pLaserManager );

    m_pSpriteManager = new SpriteManager();
    m_pAmmoLayer->AddSceneObject( m_pSpriteManager );

    InitialiseComponents();

    if ( GetSectorInfo()->HasShipyard() == true )
    {
        glm::vec3 shipyardPosition( 0.0f, 0.0f, 0.0f );
        m_pShipyard = new Shipyard( shipyardPosition );
        m_pShipLayer->AddSceneObject( m_pShipyard );

        const ShipInfo* pTurretInfo = g_pGame->GetShipInfoManager()->Get( GetSectorInfo()->GetFaction(), "special_turret" );
        if ( pTurretInfo != nullptr )
        {
            for ( int i = 0; i < 2; ++i )
            {
                ShipSpawnData spawnData;
                spawnData.m_PositionX = ( i == 0 ) ? -160.0f : 230.0f;
                spawnData.m_PositionY = 0.0f;

                ShipCustomisationData customisationData;
                customisationData.m_CaptainName = "";
                customisationData.m_ShipName = "";
                customisationData.m_pModuleInfoHexGrid = pTurretInfo->GetModuleInfoHexGrid();

                Ship* pTurret = new Ship();
                pTurret->SetInitialisationParameters(
                    GetSectorInfo()->GetFaction(),
                    GetRegionalFleet(),
                    customisationData,
                    spawnData,
                    pTurretInfo );

                pTurret->Initialize();

                AddShip( pTurret );
            }
        }
    }

    if ( GetSectorInfo()->HasStarfort() )
    {
        SpawnStarfort();
    }

    m_pRadar = new Radar();
    Genesis::FrameWork::GetGuiManager()->AddElement( m_pRadar );

    IntelStart();

    if ( GetSectorInfo()->HasProceduralSpawning() )
    {
        SpawnContestingFleets();
        SpawnRegionalFleet();

        if ( SelectFixedEvent() == false )
        {
            SelectRandomEvent();
        }

        SelectPlaylist();

        // How many waves should we create for the faction that owns this sector?
        m_AdditionalWaves = GetSectorInfo()->HasComponentName( "ReinforcementsComponent" ) ? 0u : 1u;

        const Difficulty difficulty = g_pGame->GetDifficulty();
        const float playedTime = g_pGame->GetPlayedTime();
        int minWaves = 2;
        int maxWaves = 2;
        if ( difficulty == Difficulty::Easy )
        {
            if ( playedTime > ( cWaveThresholdThree * 60.0f ) )
            {
                maxWaves = 3;
            }
            else if ( playedTime > ( cWaveThresholdFour * 60.0f ) )
            {
                maxWaves = 4;
            }
        }
        else if ( difficulty == Difficulty::Normal )
        {
            maxWaves = 3;

            if ( playedTime > ( cWaveThresholdFour * 60.0f ) )
            {
                maxWaves = 4;
            }
        }
        else
        {
            minWaves = 3;
            maxWaves = 4;
        }

        m_AdditionalWaves = gRand( minWaves, maxWaves );

        // If the faction we're attacking owns bordering sectors, there's an increased
        // chance of an extra wave.
        if ( GetSectorInfo()->GetFaction()->GetFactionId() != FactionId::Neutral )
        {
            float waveChance = 0.1f;
            SectorInfoVector borderingSectors;
            GetSectorInfo()->GetBorderingSectors( borderingSectors );
            for ( auto& pBorderingSector : borderingSectors )
            {
                if ( pBorderingSector->GetFaction() == GetSectorInfo()->GetFaction() )
                {
                    waveChance += 0.1f;
                }
            }

            if ( waveChance > gRand() )
            {
                m_AdditionalWaves++;
            }
        }

        Genesis::FrameWork::GetLogger()->LogInfo( "Additional waves for this sector: %d", m_AdditionalWaves );
    }
    else
    {
        SelectPlaylist();
        SelectFixedEvent();
        SpawnContestingFleets();
    }

    DamageTrackerDebugWindow::Register();

    return true;
}

void Sector::InitialiseComponents()
{
    using namespace Genesis;
    for ( std::string componentName : GetSectorInfo()->GetComponentNames() )
    {
        ComponentSharedPtr pComponent = ComponentFactory::Create( componentName );
        if ( pComponent == nullptr )
        {
            FrameWork::GetLogger()->LogWarning( "Couldn't instantiate a component with the name '%s'", componentName.c_str() );
        }
        else
        {
            pComponent->Initialise();
            m_Components.Add( pComponent );
        }
    }
}

bool Sector::SelectFixedEvent()
{
    const ImperialRequestList& requests = g_pGame->GetRequestManager()->GetRequests();
    for ( auto& request : requests )
    {
        if ( request->GoalExists( GetSectorInfo() ) )
        {
            request->OnPlayerEnterSector();
            return true;
        }
    }

    return false;
}

void Sector::SelectRandomEvent()
{
    const SectorEventVector& sectorEvents = g_pGame->GetSectorEvents();
    SectorEventVector availableSectorEvents;
    for ( auto& pSectorEvent : sectorEvents )
    {
        if ( pSectorEvent->IsAvailableAt( GetSectorInfo() ) )
        {
            availableSectorEvents.push_back( pSectorEvent );
        }
    }

    if ( availableSectorEvents.empty() || gRand() > RandomEventChance )
    {
        m_pSectorEvent = nullptr;
    }
    else
    {
        m_pSectorEvent = availableSectorEvents[ rand() % availableSectorEvents.size() ];
        m_pSectorEvent->OnPlayerEnterSector();
    }
}

void Sector::SelectPlaylist()
{
    using namespace Genesis;

    bool flagshipPresent = false;
    const ShipList& shipList = GetShipList();
    for ( auto& pShip : shipList )
    {
        if ( Faction::sIsEnemyOf( pShip->GetFaction(), g_pGame->GetPlayerFaction() ) && pShip->IsFlagship() )
        {
            flagshipPresent = true;
            break;
        }
    }

    std::string playlist = flagshipPresent ? "data/playlists/bossfight.m3u" : "data/playlists/combat.m3u";
    ResourcePlaylist* pPlaylistResource = FrameWork::GetResourceManager()->GetResource<ResourcePlaylist*>( playlist );
    FrameWork::GetSoundManager()->SetPlaylist( pPlaylistResource, true );
}

void Sector::Update( float delta )
{
#ifdef _DEBUG
    m_pShipTweaks->Update( delta );
#endif

    m_pTrailManager->Update( delta );
    m_pAmmoManager->Update( delta );
    m_pLaserManager->Update( delta );
    m_pSpriteManager->Update( delta );
    m_pParticleManager->Update( delta );
    m_pMuzzleflashManager->Update( delta );
    m_pCamera->Update( delta );
    m_pLootWindow->Update( delta );
    m_pSectorSpawner->Update();

    if ( m_pHotbar != nullptr )
    {
        m_pHotbar->Update( delta );
    }

    if ( m_pFleetStatus != nullptr )
    {
        m_pFleetStatus->Update();
    }

    DeleteRemovedShips();

    Ship* pPlayerShip = g_pGame->GetPlayer()->GetShip();
    if ( pPlayerShip != nullptr && pPlayerShip->GetDockingState() == DockingState::Undocked && !pPlayerShip->GetHyperspaceCore()->IsCharging() && !pPlayerShip->GetHyperspaceCore()->IsJumping() )
    {
        Genesis::InputManager* pInputManager = Genesis::FrameWork::GetInputManager();
        if ( pInputManager->IsButtonPressed( SDL_SCANCODE_ESCAPE ) )
        {
            m_pHyperspaceMenu->Show( true );
        }
    }

    m_pHyperspaceMenu->Update( delta );
    m_pDeathMenu->Update( delta );

    UpdateComponents( delta );
    UpdateReinforcements( delta );
    UpdateSectorResolution();

    for ( auto& pFleetCommand : m_FleetCommands )
    {
        pFleetCommand->Update();
    }

    DamageTrackerDebugWindow::Update();
}

void Sector::UpdateComponents( float delta )
{
    using namespace Genesis;
    for ( auto& pComponentPair : m_Components )
    {
        pComponentPair.second->Update( delta );
    }
}

void Sector::UpdateSectorResolution()
{
    // TODO: If the player is killed and there is only one faction left, that faction should claim the sector
    if ( m_IsPlayerVictorious || GetSectorInfo()->HasProceduralSpawning() == false )
    {
        return;
    }

    bool hostilesPresent = false;
    int hostilesKilled = 0;
    int regionalShips = 0;
    int regionalShipsKilled = 0;
    for ( auto& pShip : m_ShipList )
    {
        if ( pShip->GetFaction() != g_pGame->GetFaction( FactionId::Player ) && pShip->GetFaction() != g_pGame->GetFaction( FactionId::Empire ) )
        {
            if ( pShip->IsDestroyed() )
            {
                hostilesKilled++;
            }
            else
            {
                hostilesPresent = true;
            }
        }

        if ( pShip->GetFaction() == GetSectorInfo()->GetFaction() )
        {
            regionalShips++;

            if ( pShip->IsDestroyed() )
            {
                regionalShipsKilled++;
            }
        }
    }

    // Once too many ships belonging to the owner of the sector have been destroyed, see if we should spawn a reinforcement wave
    bool reinforced = false;
    if ( regionalShips > 0 )
    {
        const float attritionRatio = static_cast<float>( regionalShipsKilled ) / static_cast<float>( regionalShips );
        if ( attritionRatio >= 0.75f && m_AdditionalWaves > 0u )
        {
            m_AdditionalWaves--;

            const int fleetPoints = (int)( (float)( gRand( 400, 600 ) + m_AdditionalWavesSpawned * 350 ) * m_pSectorInfo->GetFaction()->GetRegionalFleetStrengthMultiplier() );
            FleetSharedPtr temporaryFleet = std::make_shared<Fleet>();
            temporaryFleet->Initialise( m_pSectorInfo->GetFaction(), m_pSectorInfo );
            temporaryFleet->GenerateProceduralFleet( fleetPoints );

            Reinforce( temporaryFleet, true );
            m_TemporaryFleets.push_back( temporaryFleet );
            reinforced = true;
            m_AdditionalWavesSpawned++;
        }
    }

    if ( reinforced == false && hostilesPresent == false && m_PendingHostileReinforcements.empty() )
    {
        m_IsPlayerVictorious = true;

        bool bonusApplied = false;
        const int conquestReward = GetSectorInfo()->GetFaction()->GetConquestReward( GetSectorInfo(), &bonusApplied );
        if ( conquestReward > 0 )
        {
            Player* pPlayer = g_pGame->GetPlayer();
            pPlayer->SetInfluence( pPlayer->GetInfluence() + conquestReward );
        }

        FactionId originalOwner = GetSectorInfo()->GetFaction()->GetFactionId();
        GetSectorInfo()->ForceResolve( g_pGame->GetFaction( FactionId::Empire ) );

        // The standard "All hostiles destroyed" line is only said when there are no bonuses in play as each
        // ImperialRequisiton will have its own "success" intel line to say.
        if ( bonusApplied == false && originalOwner != FactionId::Empire )
        {
            std::stringstream ss;

            if ( hostilesKilled == 0 && conquestReward > 0 )
            {
                ss << "No hostiles present, we claim this sector for the Empire." << std::endl
                   << std::endl
                   << "Our influence with Imperial HQ has increased by " << conquestReward << ".";
            }
            else
            {
                ss << "All hostiles destroyed." << std::endl
                   << std::endl
                   << "Our influence with Imperial HQ has increased by " << conquestReward << ".";
            }

            g_pGame->AddFleetCommandIntel( ss.str() );

            ContextualTips::Present( ContextualTipType::NoEnemies );
        }

        if ( GetSectorInfo()->IsHomeworld() && originalOwner != FactionId::Empire )
        {
            GetSectorInfo()->SetHomeworld( false );
            g_pGame->GetFaction( originalOwner )->SetHomeworld( nullptr );
        }

        BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
        if ( originalOwner == FactionId::Pirate && pBlackboard->Exists( sFirstPirateEvent ) && pBlackboard->Exists( sFirstPirateEventCompleted ) == false )
        {
            const int numShipyardsCaptured = pBlackboard->Get( sPirateShipyardsCaptured ) + 1;
            pBlackboard->Add( sPirateShipyardsCaptured, numShipyardsCaptured );
        }
    }
}

void Sector::DeleteRemovedShips()
{
    for ( Ship* pShip : m_ShipsToRemove )
    {
        if ( g_pGame->GetPlayer() && pShip == g_pGame->GetPlayer()->GetShip() )
            g_pGame->GetPlayer()->UnassignShip();

        m_pShipLayer->RemoveSceneObject( pShip );
        m_ShipList.remove( pShip );
    }

    m_ShipsToRemove.clear();
}

void Sector::SpawnContestingFleets()
{
    const FleetWeakPtrList& fleets = m_pSectorInfo->GetContestedFleets();
    for ( FleetWeakPtrList::const_iterator it = fleets.cbegin(); it != fleets.cend(); ++it )
    {
        FleetSharedPtr pFleet = it->lock();
        if ( pFleet )
        {
            Reinforce( pFleet );
        }
    }
    UpdateReinforcements( 0.0f );
}

void Sector::SpawnRegionalFleet()
{
    int numRegionalFleetPoints = (int)( (float)m_pSectorInfo->GetRegionalFleetPoints() * m_pSectorInfo->GetFaction()->GetRegionalFleetStrengthMultiplier() );
    if ( numRegionalFleetPoints > 0 )
    {
        m_pRegionalFleet = std::make_shared<Fleet>();
        m_pRegionalFleet->Initialise( m_pSectorInfo->GetFaction(), m_pSectorInfo );
        m_pRegionalFleet->GenerateProceduralFleet( numRegionalFleetPoints );

        const glm::vec2 spawnPosition = GetFleetSpawnPosition( m_pSectorInfo->GetFaction() );
        FleetSpawner::Spawn( m_pRegionalFleet, this, nullptr, spawnPosition );
    }
}

void Sector::SpawnStarfort()
{
    ShipSpawnData spawnData;
    spawnData.m_PositionX = 0.0f;
    spawnData.m_PositionY = -600.0f;

    const ShipInfo* pStarfortInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetFaction( FactionId::Empire ), "special_starfort" );
    if ( pStarfortInfo == nullptr )
    {
        Genesis::FrameWork::GetLogger()->LogError( "Couldn't find Empire's special_starfort." );
        return;
    }

    ShipCustomisationData customisationData;
    customisationData.m_CaptainName = "";
    customisationData.m_ShipName = "";
    customisationData.m_pModuleInfoHexGrid = pStarfortInfo->GetModuleInfoHexGrid();

    Ship* pStarfort = new Ship();
    pStarfort->SetInitialisationParameters(
        g_pGame->GetFaction( FactionId::Empire ),
        GetRegionalFleet(),
        customisationData,
        spawnData,
        pStarfortInfo );

    pStarfort->Initialize();

    AddShip( pStarfort );
}

void Sector::Reinforce( FleetSharedPtr pFleet, bool immediate /* = false */, ShipVector* pSpawnedShips /* = nullptr */ )
{
    if ( immediate )
    {
        ReinforceImmediate( pFleet, pSpawnedShips );
    }
    else
    {
        SDL_assert( pSpawnedShips == nullptr ); // pSpawnedShips can only be used when immediately reinforcing.
        if ( Faction::sIsEnemyOf( pFleet->GetFaction(), g_pGame->GetPlayerFaction() ) )
        {
            m_PendingHostileReinforcements.push_back( pFleet );
        }
        else
        {
            m_PendingImperialReinforcements.push_back( pFleet );
        }
    }
}

void Sector::UpdateReinforcements( float delta )
{
    if ( m_TimeToNextReinforcements > 0.0f )
    {
        m_TimeToNextReinforcements -= delta;
        return;
    }

    int imperialFleetCommands = 0;
    int hostileFleetCommands = 0;
    int imperialShips = 0;
    int hostileShips = 0;

    for ( auto& pFleetCommand : m_FleetCommands )
    {
        bool fleetCommandAdded = false;
        for ( Ship* pShip : pFleetCommand->GetShips() )
        {
            if ( pShip->IsDestroyed() == false )
            {
                if ( Faction::sIsEnemyOf( pShip->GetFaction(), g_pGame->GetPlayerFaction() ) )
                {
                    hostileShips++;
                    if ( !fleetCommandAdded )
                    {
                        hostileFleetCommands++;
                        fleetCommandAdded = true;
                    }
                }
                else
                {
                    imperialShips++;
                    if ( !fleetCommandAdded )
                    {
                        imperialFleetCommands++;
                        fleetCommandAdded = true;
                    }
                }
            }
        }
    }

    static const int sMaxActiveFleetCommands = 10;
    static const int sMaxActiveImperialFleetCommands = 8;
    while ( 1 )
    {
        if ( imperialFleetCommands + hostileFleetCommands >= sMaxActiveFleetCommands )
        {
            break;
        }

        if ( imperialFleetCommands >= sMaxActiveImperialFleetCommands )
        {
            break;
        }

        FleetSharedPtr pFleetToSpawn;
        if ( m_PendingImperialReinforcements.empty() == false )
        {
            pFleetToSpawn = m_PendingImperialReinforcements.front();
            m_PendingImperialReinforcements.pop_front();
            imperialFleetCommands++;
        }
        else if ( m_PendingHostileReinforcements.empty() == false )
        {
            pFleetToSpawn = m_PendingHostileReinforcements.front();
            m_PendingHostileReinforcements.pop_front();
            hostileFleetCommands++;
        }

        if ( pFleetToSpawn )
        {
            Genesis::Logger* pLogger = Genesis::FrameWork::GetLogger();
            pLogger->LogInfo( "Reinforcing sector, current state: " );
            pLogger->LogInfo( "- Imperial fleet commands: %d", imperialFleetCommands );
            pLogger->LogInfo( "- Hostile fleet commands: %d", hostileFleetCommands );
            pLogger->LogInfo( "- Imperial ships: %d", imperialShips );
            pLogger->LogInfo( "- Hostile ships: %d", hostileShips );
            m_TimeToNextReinforcements = gRand( 10.0f, 15.0f );
            bool showNotification = ( delta > 0.0f ); // don't show reinforcement notifications when entering the sector.
            ReinforceImmediate( pFleetToSpawn, nullptr, showNotification );
        }
        else
        {
            break;
        }
    }
}

void Sector::ReinforceImmediate( FleetSharedPtr pFleet, ShipVector* pSpawnedShips /* = nullptr */, bool showNotification /* = true */ )
{
    const glm::vec2 spawnPosition = GetFleetSpawnPosition( pFleet->GetFaction() );
    FleetSpawner::Spawn( pFleet, this, pSpawnedShips, spawnPosition );

    if ( showNotification )
    {
        if ( Faction::sIsEnemyOf( pFleet->GetFaction(), g_pGame->GetPlayerFaction() ) )
        {
            g_pGame->AddFleetCommandIntel( "Detected waveform collapse, an enemy fleet is entering the sector." );
        }
        else
        {
            g_pGame->AddFleetCommandIntel( "Captain, reinforcements have arrived." );
        }
    }
}

void Sector::AddShip( Ship* pShip )
{
    m_ShipList.push_back( pShip );
    m_pShipLayer->AddSceneObject( pShip, true );

    // If non-Imperial ships arrive after victory, then the victory has to be rescinded
    if ( m_IsPlayerVictorious && pShip->GetFaction() != g_pGame->GetFaction( FactionId::Empire ) )
    {
        m_IsPlayerVictorious = false;
    }

    // Flavour text for flagships
    if ( pShip->IsFlagship() )
    {
        FactionId factionId = pShip->GetFaction()->GetFactionId();
        if ( factionId == FactionId::Ascent )
        {
            g_pGame->AddFleetCommandIntel( "The Ascent flagship is here, the 'Angel of Io'. It is heavily shielded and has a devastating battery of lances, as well as two forward-mounted ion cannons. Iriani technology, we reckon." );
        }
        else if ( factionId == FactionId::Pirate )
        {
            g_pGame->AddFleetCommandIntel( "The Pirate flagship is here, Captain. The 'Northern Star' is a commandeered vessel from an unaligned sector and uses experimental lance weapons." );
        }
        else if ( factionId == FactionId::Marauders )
        {
            g_pGame->AddFleetCommandIntel( "Presence of the Marauder flagship is confirmed. The 'Ragnarokkr' is extremely heavily armoured and relies on artilleries and torpedo launchers." );
            g_pGame->AddFleetCommandIntel( "From previous battles we know it uses reactive armour, so energy weapons would be more effective against it." );
        }
        else if ( factionId == FactionId::Iriani )
        {
            // Silver Handmaiden
            // g_pGame->AddFleetCommandIntel( "" );
        }
    }
}

void Sector::RemoveShip( Ship* pShip )
{
    // Check if we already have this ship in our list of ships to remove
    const ShipList::iterator& itEnd = m_ShipsToRemove.end();
    for ( ShipList::iterator it = m_ShipsToRemove.begin(); it != itEnd; ++it )
    {
        if ( *it == pShip )
            return;
    }

    pShip->Terminate();

    m_ShipsToRemove.push_back( pShip );
}

glm::vec2 Sector::GetFleetSpawnPosition( Faction* pFleetFaction )
{
    return m_pSectorSpawner->ClaimFleetSpawnPosition( pFleetFaction );
}

void Sector::IntelStart()
{
    std::stringstream ss;

    if ( Faction::sIsEnemyOf( GetSectorInfo()->GetFaction(), g_pGame->GetPlayerFaction() ) )
    {
        ss << "Fleet entering sector " << GetSectorInfo()->GetName() << ", contact imminent.";
    }
    else
    {
        ss << "Fleet entering allied sector " << GetSectorInfo()->GetName() << ".";
    }

    g_pGame->AddIntel( GameCharacter::FleetIntelligence, ss.str(), false );

    ContextualTips::Present( ContextualTipType::EnterSector1 );
    ContextualTips::Present( ContextualTipType::EnterSector2 );

    Player* pPlayer = g_pGame->GetPlayer();
    const std::string& shipTemplate = pPlayer->GetCompanionShipTemplate();
    if ( shipTemplate == "phalanx" || shipTemplate == "ironclad" )
    {
        ContextualTips::Present( ContextualTipType::EnterSectorWithArmourRepairer );
    }

    if ( GetSectorInfo()->HasShipyard() )
    {
        ContextualTips::Present( ContextualTipType::Shipyard );
    }
}

void Sector::SetTowerBonus( Faction* pFaction, TowerBonus bonus, float bonusMagnitude )
{
    int idx = static_cast<int>( pFaction->GetFactionId() );
    m_TowerBonus[ idx ] = bonus;
    m_TowerBonusMagnitude[ idx ] = bonusMagnitude;
}

void Sector::GetTowerBonus( Faction* pFaction, TowerBonus* pBonus, float* pBonusMagnitude ) const
{
    if ( pFaction == nullptr )
    {
        if ( pBonus != nullptr )
        {
            *pBonus = TowerBonus::None;
        }

        if ( pBonusMagnitude != nullptr )
        {
            *pBonusMagnitude = 0.0f;
        }
    }
    else
    {
        int idx = static_cast<int>( pFaction->GetFactionId() );
        if ( pBonus != nullptr )
        {
            *pBonus = m_TowerBonus[ idx ];
        }

        if ( pBonusMagnitude != nullptr )
        {
            *pBonusMagnitude = m_TowerBonusMagnitude[ idx ];
        }
    }
}

void Sector::SelectBackground()
{
    const glm::ivec2& coordinates = GetSectorInfo()->GetCoordinates();
    const unsigned int seed = coordinates.x + coordinates.x * coordinates.y;
    StarInfo* pStarInfo = nullptr;
    if ( m_pSectorInfo->HasStar() )
    {
        pStarInfo = new StarInfo( seed * seed );
        pStarInfo->PrintOut();
    }

    m_pBackground = new Background( m_pSectorInfo->GetBackground(), pStarInfo );
    m_pBackgroundLayer->AddSceneObject( m_pBackground );
}

void Sector::AddFleetCommand( FleetCommandUniquePtr pFleetCommand )
{
    // Hook into the Fleet Status table when the player fleet is added.
    if ( pFleetCommand->GetLeader()->GetFaction()->GetFactionId() == FactionId::Player )
    {
        m_pFleetStatus->AddShip( pFleetCommand->GetLeader() );

        for ( Ship* pShip : pFleetCommand->GetShips() )
        {
            m_pFleetStatus->AddShip( pShip );
        }
    }

    m_FleetCommands.push_back( std::move( pFleetCommand ) );
}

} // namespace Hexterminate