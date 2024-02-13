#pragma once

#include <list>
#include <memory>
#include <vector>

namespace Hexterminate
{

// If the ShipType enum is changed, update the FleetDoctrine as well.
// The ShipType is set by calculating the points value of a ship, with ShipInfo being responsible for this.
// It is used for spawning ships of sizes in FleetSpawner.
enum class ShipType
{
    Invalid,
    Gunship,
    Battlecruiser,
    Capital,

    Count
};

enum class DockingState
{
    Undocked,
    Docking,
    Docked,
    Undocking
};

enum class QuantumState
{
    Inactive,
    White,
    Black
};

class Ship;
typedef std::list<Ship*> ShipList;

class ShipSpawnData;
typedef std::vector<ShipSpawnData> ShipSpawnDataVector;

class ShipTweaks;
using ShipTweaksUniquePtr = std::unique_ptr<ShipTweaks>;

} // namespace Hexterminate
