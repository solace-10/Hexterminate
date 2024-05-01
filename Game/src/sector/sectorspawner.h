// Copyright 2024 Pedro Nunes
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

#include <bitset>
#include <optional>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Hexterminate
{

class Faction;

class SectorSpawner
{
public:
    SectorSpawner();
    ~SectorSpawner();

    void Update();

    glm::vec3 ClaimFleetSpawnPosition( Faction* pFleetFaction );

private:
    static constexpr int sNumSpawnPointsSide = 10;
    static constexpr float sCellWorldSize = 1000.0f;

    void DrawDebugUI();

    void SetCellReservation( const glm::ivec2& cellPosition, bool isReserved );
    void SetCellReservation( int cellX, int cellY, bool isReserved );
    bool IsCellReserved( const glm::ivec2& cellPosition ) const;
    bool IsCellReserved( int cellX, int cellY ) const;

    std::vector<glm::ivec2> GetFreeCellsNear( const glm::ivec2& cellPosition, int range = 2 ) const;
    glm::ivec2 GetRandomCell() const;

    void Evaluate();

    std::optional<glm::ivec2> ToCellPosition( const glm::vec3 worldPosition ) const;
    glm::vec3 ToWorldPosition( const glm::ivec2& cellPosition ) const;

    using Reservation = std::bitset<sNumSpawnPointsSide * sNumSpawnPointsSide>;
    Reservation m_Reservation;
    bool m_DebugUIOpen;
};

inline void SectorSpawner::SetCellReservation( const glm::ivec2& cellPosition, bool isReserved )
{
    SetCellReservation( cellPosition.x, cellPosition.y, isReserved );
}

inline void SectorSpawner::SetCellReservation( int cellX, int cellY, bool isReserved )
{
    m_Reservation[ cellX + cellY * sNumSpawnPointsSide ] = isReserved;
}

inline bool SectorSpawner::IsCellReserved( const glm::ivec2& cellPosition ) const
{
    return IsCellReserved( cellPosition.x, cellPosition.y );
}

inline bool SectorSpawner::IsCellReserved( int cellX, int cellY ) const
{
    return m_Reservation[ cellX + cellY * sNumSpawnPointsSide ];
}

} // namespace Hexterminate
