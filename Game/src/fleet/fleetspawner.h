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

#pragma once

#include "fleet/fleet.fwd.h"
#include "ship/ship.fwd.h"

namespace Hexterminate
{

class Faction;
class Fleet;
class Sector;
class ShipInfo;

using ShipInfoVector = std::vector<const ShipInfo*>;
using ShipVector = std::vector<Ship*>;

enum class FleetFormation
{

    Scattered, // A scattered fleet is individually dispersed over a rather large distance
    StrikeGroups, // Scattered but split into groups according to ship type
    Cohesive, // A strong wall formation with battlecruisers at the front, gunships at the flanks and battleships at the core
    Escort, // Used by the player's allies, placing each ship besides the player's

    Count
};

///////////////////////////////////////////////////////////////////////////////
// Spawns a fleet to a given sector
// It will attempt to use as many points as possible as the fleet can contain
// but it is limited by the templates loaded by the ShipInfoManager at the
// beginning of the game.
///////////////////////////////////////////////////////////////////////////////

class FleetSpawner
{
public:
    static bool Spawn( FleetSharedPtr pFleet, Sector* pSector, ShipVector* pSpawnedShips, float x, float y );

private:
    static bool SpawnFleetAI( FleetSharedPtr pFleet, Sector* pSector, ShipVector* pSpawnedShips, float x, float y, FleetFormation formation );
    static bool SpawnFleetPlayer( FleetSharedPtr pFleet, Sector* pSector, ShipVector* pSpawnedShips, float x, float y );
    static void GetSpawnData( ShipInfoVector shipsToSpawn, float spawnPointX, float spawnPointY, ShipSpawnDataVector& fleetSpawnData, FleetFormation formation );
    static Ship* SpawnShip( Sector* pSector, FleetSharedPtr pFleet, const ShipInfo* pShipInfo, const ShipSpawnData& spawnData );
    static void CheckFirstEncounter( Faction* pFaction );
    static void CheckFlagshipEncounter( Faction* pFaction );
};

} // namespace Hexterminate
