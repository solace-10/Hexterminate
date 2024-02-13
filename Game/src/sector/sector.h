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

#include "faction/faction.h"
#include "ship/moduleinfo.h"
#include "ship/ship.fwd.h"
#include <component.h>
#include <list>
#include <memory>
#include <scene/layer.h>
#include <vector>

#include "fleet/fleet.fwd.h"

namespace Genesis
{
class Layer;
}

namespace Hexterminate
{

class Background;
class AmmoManager;
class MuzzleflashManager;
class MuzzleflashManagerRep;
class LaserManager;
class SpriteManager;
class SectorCamera;
class Hotbar;
class Dust;
class ShipInfo;
class Faction;
class Fleet;
class FleetStatus;
class SectorInfo;
class Shipyard;
class HyperspaceMenu;
class TrailManager;
class TrailManagerRep;
class Radar;
class ParticleManager;
class ParticleManagerRep;
class DeathMenu;
class SectorEvent;
class LootWindow;
class FleetCommand;
class Boundary;

using HotbarUniquePtr = std::unique_ptr<Hotbar>;
using FleetStatusUniquePtr = std::unique_ptr<FleetStatus>;
using FleetCommandUniquePtr = std::unique_ptr<FleetCommand>;
using FleetCommandVector = std::vector<FleetCommandUniquePtr>;

static const int LAYER_BACKGROUND = 1;
static const int LAYER_SHIP = 1 << 1;
static const int LAYER_AMMO = 1 << 2;
static const int LAYER_FX = 1 << 3;
static const int LAYER_PHYSICS = 1 << 4;
static const int LAYER_GALAXY = 1 << 5;
static const int LAYER_ALL = 0xFFFFFFFF;

static const float sSpawnPointSize = 1000.0f;
static const int sSectorSpawnPoints = 9;

using IntVector = std::vector<int>;
using ShipVector = std::vector<Ship*>;

class Sector
{
public:
    Sector( SectorInfo* pSectorInfo );
    virtual ~Sector();
    virtual void Update( float fDelta );

    bool Initialise();

    SectorInfo* GetSectorInfo() const;
    AmmoManager* GetAmmoManager() const;
    LaserManager* GetLaserManager() const;
    SpriteManager* GetSpriteManager() const;
    ParticleManager* GetParticleManager() const;
    MuzzleflashManager* GetMuzzleflashManager() const;
    const ShipList& GetShipList() const;
    TrailManager* GetTrailManager() const;
    Background* GetBackground() const;
    ShipTweaks* GetShipTweaks() const;
    SectorCamera* GetCamera() const;

    void AddShip( Ship* pShip );
    void RemoveShip( Ship* pShip );

    Shipyard* GetShipyard() const;

    void SetTowerBonus( Faction* pFaction, TowerBonus bonus, float bonusMagnitude );
    void GetTowerBonus( Faction* pFaction, TowerBonus* pBonus, float* pBonusMagnitude ) const;

    FleetWeakPtr GetRegionalFleet() const;

    bool Reinforce( FleetSharedPtr pFleet, ShipVector* pSpawnedShips = nullptr );
    bool IsPlayerVictorious() const;
    void AddFleetCommand( FleetCommandUniquePtr pFleetCommand );

protected:
    void SpawnContestingFleets();
    void SpawnRegionalFleet();
    void SpawnStarfort();
    void DeleteRemovedShips();
    bool GetFleetSpawnPosition( Faction* pFaction, float& x, float& y );
    void GetFleetSpawnPositionAtPoint( int idx, float& x, float& y );
    void DebugDrawFleetSpawnPositions();
    void IntelStart();
    bool SelectFixedEvent();
    void SelectRandomEvent();
    void UpdateSectorResolution();
    void SelectBackground();
    void SelectPlaylist();
    void InitialiseComponents();
    void UpdateComponents( float delta );

    SectorInfo* m_pSectorInfo;

    Genesis::LayerSharedPtr m_pBackgroundLayer;
    Genesis::LayerSharedPtr m_pShipLayer;
    Genesis::LayerSharedPtr m_pFxLayer;
    Genesis::LayerSharedPtr m_pAmmoLayer;
    Genesis::LayerSharedPtr m_pPhysicsLayer;
    Background* m_pBackground;
    Dust* m_pDust;
    Boundary* m_pBoundary;
    ShipList m_ShipList;
    ShipList m_ShipsToRemove;
    ParticleManager* m_pParticleManager;
    ParticleManagerRep* m_pParticleManagerRep;
    MuzzleflashManager* m_pMuzzleflashManager;
    MuzzleflashManagerRep* m_pMuzzleflashManagerRep;
    AmmoManager* m_pAmmoManager;
    LaserManager* m_pLaserManager;
    SpriteManager* m_pSpriteManager;
    TrailManager* m_pTrailManager;
    TrailManagerRep* m_pTrailManagerRep;
    SectorCamera* m_pCamera;
    Radar* m_pRadar;

    HotbarUniquePtr m_pHotbar;
    FleetStatusUniquePtr m_pFleetStatus;

    FleetSharedPtr m_pRegionalFleet;
    IntVector m_AvailableSpawnPoints;

    ShipSpawnDataVector m_DebugShipSpawnData;

    Shipyard* m_pShipyard;
    HyperspaceMenu* m_pHyperspaceMenu;
    DeathMenu* m_pDeathMenu;

    SectorEvent* m_pSectorEvent;

    bool m_IsPlayerVictorious;

    TowerBonus m_TowerBonus[ static_cast<unsigned int>( FactionId::Count ) ];
    float m_TowerBonusMagnitude[ static_cast<unsigned int>( FactionId::Count ) ];

    LootWindow* m_pLootWindow;
    FleetCommandVector m_FleetCommands;

    unsigned int m_AdditionalWaves;
    unsigned int m_AdditionalWavesSpawned;
    FleetList m_TemporaryFleets;

    Genesis::ComponentContainer m_Components;
    ShipTweaksUniquePtr m_pShipTweaks;
};

inline AmmoManager* Sector::GetAmmoManager() const
{
    return m_pAmmoManager;
}

inline LaserManager* Sector::GetLaserManager() const
{
    return m_pLaserManager;
}

inline SpriteManager* Sector::GetSpriteManager() const
{
    return m_pSpriteManager;
}

inline const ShipList& Sector::GetShipList() const
{
    return m_ShipList;
}

inline SectorInfo* Sector::GetSectorInfo() const
{
    return m_pSectorInfo;
}

inline Shipyard* Sector::GetShipyard() const
{
    return m_pShipyard;
}

inline TrailManager* Sector::GetTrailManager() const
{
    return m_pTrailManager;
}

inline ParticleManager* Sector::GetParticleManager() const
{
    return m_pParticleManager;
}

inline MuzzleflashManager* Sector::GetMuzzleflashManager() const
{
    return m_pMuzzleflashManager;
}

inline FleetWeakPtr Sector::GetRegionalFleet() const
{
    return m_pRegionalFleet;
}

inline bool Sector::IsPlayerVictorious() const
{
    return m_IsPlayerVictorious;
}

inline Background* Sector::GetBackground() const
{
    return m_pBackground;
}

inline ShipTweaks* Sector::GetShipTweaks() const
{
    return m_pShipTweaks.get();
}

inline SectorCamera* Sector::GetCamera() const
{
    return m_pCamera;
}

} // namespace Hexterminate