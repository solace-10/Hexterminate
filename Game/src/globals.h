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

namespace Hexterminate
{
static const float GalaxyMinSize = 1600.0f * 1.25f;
static const float GalaxyTimeCompression = 4.0f; // Galaxy ticks this much slower when the player is in combat
static const float TurnDuration = 30.0f; // Length of a turn, in seconds
static const int NumSectorsX = 24;
static const int NumSectorsY = 24;
static const float FleetSpeed = 0.25f; // Fleet speed in sectors per second
static const int NumPirateSectors = 6; // The pirate faction will always try to have this many sectors under their control
static const int FleetBuildTime = 3; // Number of turns it takes to build a new fleet
static const float RegionalFleetMin = 200.0f; // A regional fleet's minimum strength (non-adaptive)
static const float RegionalFleetMax = 400.0f; // A regional fleet's maximum strength (non-adaptive)
static const float RegionalFleetRegen = 0.5f; // A regional fleet regenerates this much (%) of its strength per turn
static const float StarfortRegen = 0.05f; // A starfleet regenerates this much (%) of its strength per turn;
static const float BaseModuleHealth = 1000.0f;
static const int PointsPerModule = 10; // The point value of a ship is calculated by how many modules it has, multiplied by this value
static const int MaxPointsGunship = 130; // A ship is considered a Gunship as long as its point value doesn't exceed this number
static const int MaxPointsBattlecruiser = 250; // A ship is considered a Battlecruiser as long as its point value doesn't exceed this number
static const float BaseModuleMass = 60.0f;
static const float RepairDuration = 5.0f; // Time in seconds that a ship takes to repair itself
static const float RepairArmourMultiplier = 2.0f; // How much more effective is the repairer when acting on armour modules
static const float RandomEventChance = 0.33f; // Chance of a random event happening when the player enters a sector [0-1]
static const float RegenerationRate = 0.05f; // Regenerative armour repairs this % of the module's max health per second
static const float MaxFleetSupportDistance = 2.0f; // Maximum distance (in sectors) that a fleet will go out of its way to support an allied fleet
static const float cEngineTorqueMultiplier = 3.0f; // Angular velocity tweak
static const float cEngineThrustMultiplier = 1.5f; // Linear velocity tweak
static const float RammingSpeedCooldown = 20.0f; // Time before the ramming speed ability can be used again
static const float ShipRewardThreshold = 0.20f; // Percentage of damage the player needs to deal to another ship to be elegible for a reward
static const float cWaveThresholdThree = 20.0f; // Time, in minutes, before 3 waves can spawn at a sector.
static const float cWaveThresholdFour = 45.0f; // Time, in minutes, before 4 waves can spawn at a sector.
static const float cEasyDamageMitigation = 0.3f; // Percentage of damage reduction for Imperial ships in Easy difficulty.
static const int cMaxContestingFleets = 8; // Maximum number of fleets simultaneously contesting a sector (not including temporary fleets)
} // namespace Hexterminate