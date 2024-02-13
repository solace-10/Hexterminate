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

#include "fleet/fleetdoctrine.h"
#include "loot/lootprobability.h"
#include "sector/sectorinfo.h"
#include "serialisable.h"
#include "ship/ship.fwd.h"
#include <list>
#include <rendersystem.h>
#include <string>
#include <vector>

namespace Hexterminate
{

class Fleet;
class ShipInfo;
typedef std::list<int> TurnQueue;
typedef std::vector<std::string> StringVector;
typedef std::vector<const ShipInfo*> ShipInfoVector;

enum class FactionId
{
    Neutral = 0,
    Player,
    Empire,
    Ascent,
    Pirate,
    Marauders,
    Iriani,
    Special,
    Hegemon,

    Count
};

enum class FactionColourId
{
    Base = 0, // Colour by which this faction is represented in the Sector view and possibly in text.
    Primary, // Main colour the ship's modules are painted with.
    Secondary, // Secondary colour for small details on the ship's modules.
    PrimaryFlagship, // Primary colour for flagships.
    SecondaryFlagship, // Secondary colour for flagships.
    Glow, // Used by engine trails.
    GlowFlagship, // Flagship variant.
    FleetChevron, // Fleet chevron in the galaxy view.

    Count
};

enum class FactionPresence
{
    None,
    Light,
    Standard,
    Heavy
};

struct FactionInfo
{
    std::string m_Name;
    Genesis::Color m_Colours[ static_cast<unsigned int>( FactionColourId::Count ) ];
    int m_BaseFleetPoints; // Initial fleet size (in points) when the game starts.
    float m_SectorToShipyardRatio; // Optimal number of shipyards for the amount of sectors controlled by this faction.
    FleetDoctrine m_Doctrine;
    unsigned int m_CollisionLayerShip;
    unsigned int m_CollisionLayerAmmo;
    LootProbability m_LootProbability; // When a ship gets destroyed, the quality of the item it drops is given by this LootProbability
    LootProbability m_LootProbabilityFlagship; // Flagships use a different LootProbability, usually with a higher chance of better loot than regular ships
    LootProbability m_LootProbabilityFlagshipHc; // Flagship drop chances when playing in Hardcore mode
    bool m_HasFlagships; // Does this faction have a flagship?
    float m_ThreatValueMultiplier; // Overall difficulty of this faction for threat rating evaluation
    int m_ConquestReward; // How much influence is the player awarded with when he conquers a sector of this faction
    std::string m_CollapseTag; // If the collapse tag exists in the blackboard, this faction will slowly start losing sectors
    bool m_UsesFormations; // Fleets created by this faction are capable of using formations
    StringVector m_FlagshipFleetShips; // If this faction's flagship fleet is spawned, it will will contain these ships
    float m_RegionalFleetMultiplier; // Multiplier for the regional fleet's strength and any additional waves.
    bool m_SpawnsFlagshipInGalaxy;
};

class Faction : public Serialisable
{
public:
    Faction( const FactionInfo& info, FactionId id );
    virtual ~Faction() {}

    inline const std::string& GetName() const { return m_Info.m_Name; }
    inline const Genesis::Color& GetColour( FactionColourId id ) const { return m_Info.m_Colours[ (int)id ]; }
    virtual void AddControlledSector( SectorInfo* pSector, bool immediate, bool takenByPlayer );
    void RemoveControlledSector( SectorInfo* pSector, bool immediate = false );
    inline const SectorInfoVector& GetControlledSectors() const { return m_ControlledSectors; }
    inline const FleetList& GetFleets() const { return m_Fleets; }
    inline const TurnQueue& GetFleetsInConstruction() const { return m_FleetsInConstruction; }
    inline const FleetDoctrine& GetFleetDoctrine() const { return m_Info.m_Doctrine; }
    inline FactionId GetFactionId() const { return m_FactionId; }
    FleetWeakPtr BuildFleet( SectorInfo* pInitialSector );
    void DestroyFleet( FleetWeakPtr pFleet );
    unsigned int GetCollisionLayerShip() const { return m_Info.m_CollisionLayerShip; }
    unsigned int GetCollisionLayerAmmo() const { return m_Info.m_CollisionLayerAmmo; }
    const LootProbability& GetLootProbability( bool isFlagship ) const;
    inline float GetThreatValueMultiplier() const { return m_Info.m_ThreatValueMultiplier; }
    inline float GetRegionalFleetStrengthMultiplier() const { return m_Info.m_RegionalFleetMultiplier; }
    int GetConquestReward( const SectorInfo* pSectorInfo, bool* pBonusApplied = nullptr ) const;
    bool IsCollapsing() const;
    inline bool GetUsesFormations() const { return m_Info.m_UsesFormations; }
    const ShipInfoVector& GetFlagshipFleetShips() const;
    inline bool HasFlagship() const { return m_Info.m_FlagshipFleetShips.empty() == false; }
    SectorInfo* GetHomeworld() const { return m_pHomeworld; }
    void SetHomeworld( SectorInfo* pSectorInfo );
    bool HasStartedWithHomeworld() const { return m_StartedWithHomeworld; }
    void SetInitialPresence( FactionPresence presence );
    FactionPresence GetInitialPresence() const;

    void ForceNextTurn();

    virtual void Update( float delta );
    virtual void PostUpdate();

    static bool sIsEnemyOf( Faction* pFactionA, Faction* pFactionB );

    // Serialisable
    virtual bool Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement ) override;
    virtual bool Read( tinyxml2::XMLElement* pRootElement ) override;
    virtual int GetVersion() const override { return 1; }
    virtual void UpgradeFromVersion( int version ) override {}

protected:
    virtual void ProcessTurn();
    bool IsFlagshipSpawned() const;
    bool IsFlagshipDestroyed() const;

private:
    void BuildFleets( const SectorInfoVector& shipyards );
    void ProcessTurnFleets();
    void ProcessCollapse();

    FactionInfo m_Info;
    FactionId m_FactionId;
    int m_Turn;
    float m_NextTurnTimer;
    SectorInfoVector m_ControlledSectors;
    SectorInfoVector m_ControlledSectorsToAdd;
    SectorInfoVector m_ControlledSectorsToRemove;
    FleetList m_Fleets;
    TurnQueue m_FleetsInConstruction;
    FleetList m_FleetsToDestroy;
    mutable ShipInfoVector m_FlagshipFleetShips;
    SectorInfo* m_pHomeworld;
    bool m_StartedWithHomeworld;
    FactionPresence m_InitialPresence;
};

} // namespace Hexterminate