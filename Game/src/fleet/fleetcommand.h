// Copyright 2016 Pedro Nunes
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

#include <memory>
#include <vector>

#include "glm/vec2.hpp"

namespace Hexterminate
{

class Ship;
typedef std::vector<Ship*> ShipVector;
typedef std::vector<glm::vec2> RelationshipVector;

enum class FleetCommandOrder
{
    StickToFormation,
    Engage,
    Patrol
};

enum class ScanForEnemiesResult
{
    EnemiesInRange,
    EnemiesOutOfRange,
    NoEnemies
};

class FleetCommand
{
public:
    FleetCommand();

    void AssignShip( Ship* pShip );
    void AssignLeader( Ship* pShip );
    Ship* GetLeader() const;
    bool HasLeader() const;
    const ShipVector& GetShips() const;
    void SetupRelationships();

    void Update();

private:
    ScanForEnemiesResult ScanForEnemies() const;

    ShipVector m_Ships;
    RelationshipVector m_Relationships;
    Ship* m_pLeader;
};

inline Ship* FleetCommand::GetLeader() const
{
    return m_pLeader;
}

inline bool FleetCommand::HasLeader() const
{
    return ( m_pLeader != nullptr );
}

inline const ShipVector& FleetCommand::GetShips() const
{
    return m_Ships;
}

typedef std::unique_ptr<FleetCommand> FleetCommandUniquePtr;

} // namespace Hexterminate
