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

#include <imgui/imgui.h>
#include <logger.h>
#include <math/misc.h>

#include "faction/faction.h"
#include "hexterminate.h"
#include "player.h"
#include "sector/sector.h"
#include "sector/sectorspawner.h"
#include "ship/ship.h"

namespace Hexterminate
{

SectorSpawner::SectorSpawner()
    : m_DebugWindowOpen( true )
{
    m_Reservation.reset();
}

SectorSpawner::~SectorSpawner()
{
}

void SectorSpawner::Update()
{
    Evaluate();
    DrawDebugUI();
}

glm::vec3 SectorSpawner::SpawnFleet( Faction* pFleetFaction )
{
    SectorInfo* pSectorInfo = g_pGame->GetCurrentSector()->GetSectorInfo();

    // If the player's fleet is spawning and we are in an allied sector with a shipyard, then spawn next to it.
    if ( pSectorInfo->HasShipyard() && pSectorInfo->GetFaction()->GetFactionId() == FactionId::Empire && pFleetFaction->GetFactionId() == FactionId::Player )
    {
        glm::vec3 nearShipyardPosition( 0.0f, -200.0f, 0.0f );
        SetCellReservation( ToCellPosition( nearShipyardPosition ).value(), true );
        return nearShipyardPosition;
    }

    std::vector<glm::ivec2> freeCells;
    glm::ivec2 centreCell = ToCellPosition( glm::vec3( 0.0f, 0.0f, 0.0f ) ).value();

    // Are we spawning the player fleet? Spawn it near the centre of the sector.
    if ( pFleetFaction->GetFactionId() == FactionId::Player )
    {
        freeCells = GetFreeCellsNear( centreCell );
    }
    else
    {
        // Otherwise, try to spawn near the player.
        Ship* pPlayerShip = g_pGame->GetPlayer()->GetShip();
        if ( pPlayerShip == nullptr )
        {
            freeCells = GetFreeCellsNear( centreCell );
        }
        else
        {
            std::optional<glm::ivec2> playerCellPosition = ToCellPosition( pPlayerShip->GetTowerPosition() );
            if ( playerCellPosition.has_value() )
            {
                freeCells = GetFreeCellsNear( playerCellPosition.value() );
            }
            else
            {
                // This can happen if the player is currently outside the sector boundary.
                freeCells = GetFreeCellsNear( centreCell );
            }
        }
    }

    glm::ivec2 cell;
    if ( freeCells.empty() )
    {
        Genesis::FrameWork::GetLogger()->LogWarning( "Failed to find a free cell with SectorSpawner, using random cell instead." );
        cell = GetRandomCell();
    }
    else
    {
        size_t cellIndex = gRand( 0, static_cast<int>( freeCells.size() ) );
        cell = freeCells[cellIndex ];
    }

    SetCellReservation( cell, true );
    return ToWorldPosition( cell );
}

void SectorSpawner::DrawDebugUI()
{
    static const float sDebugWindowGridSize = 50.0f;
    static const float sWindowWidth = sDebugWindowGridSize * static_cast<float>( sNumSpawnPointsSide ) + 16.0f;
    static const float sWindowHeight = sDebugWindowGridSize * static_cast<float>( sNumSpawnPointsSide ) + 36.0f;

    ImGui::SetNextWindowSize( ImVec2( sWindowWidth, sWindowHeight ) );
    if ( ImGui::Begin( "Sector spawner", &m_DebugWindowOpen, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        const ImVec2 windowPosition = ImGui::GetCursorScreenPos();
        ImDrawList* pDrawList = ImGui::GetWindowDrawList();

        for ( int x = 0; x < sNumSpawnPointsSide; x++ )
        {
            for ( int y = 0; y < sNumSpawnPointsSide; y++ )
            {
                const ImVec2 p0 = ImVec2( windowPosition.x + sDebugWindowGridSize * static_cast<float>( x ), windowPosition.y + sDebugWindowGridSize * static_cast<float>( y ) );
                const ImVec2 p1 = ImVec2( p0.x + sDebugWindowGridSize, p0.y + sDebugWindowGridSize );
                pDrawList->AddRectFilled( p0, p1, IsCellReserved( x, y ) ? IM_COL32( 255, 0, 0, 255 ) : IM_COL32( 0, 255, 0, 255 ) );
                pDrawList->AddRect( p0, p1, IM_COL32( 255, 255, 255, 255 ) );
            }
        }

        ImGui::End();
    }
}

std::vector<glm::ivec2> SectorSpawner::GetFreeCellsNear( const glm::ivec2& originCellPosition, int range /* = 2 */ ) const
{
    std::vector<glm::ivec2> freeCells;

    for ( int x = originCellPosition.x - range; x < originCellPosition.x + range; x++ )
    {
        if ( x < 0 || x >= sNumSpawnPointsSide )
        {
            continue;
        }

        for ( int y = originCellPosition.y - range; y < originCellPosition.y + range; y++ )
        {
            if ( y < 0 || y >= sNumSpawnPointsSide )
            {
                continue;
            }

            glm::vec2 cellPosition( x, y );
            if ( !IsCellReserved( cellPosition ) )
            {
                freeCells.push_back( cellPosition );
            }
        }
    }

    return freeCells;
}

void SectorSpawner::Evaluate()
{
    m_Reservation.reset();

    const ShipList& ships = g_pGame->GetCurrentSector()->GetShipList();
    for ( Ship* pShip : ships )
    {
        const glm::vec3 shipPosition = pShip->GetTowerPosition();
        std::optional<glm::ivec2> cellPosition = ToCellPosition( shipPosition );
        if ( cellPosition.has_value() )
        {
            SetCellReservation( cellPosition.value(), true );
        }
    }
}

std::optional<glm::ivec2> SectorSpawner::ToCellPosition( const glm::vec3 worldPosition ) const
{
    const glm::vec3 positionInCellSpace = glm::vec3( worldPosition.x, -worldPosition.y, 0.0f ) / sCellWorldSize + glm::vec3( sNumSpawnPointsSide / 2.0f, sNumSpawnPointsSide / 2.0f, 0.0f );
    const glm::ivec2 cellPosition( static_cast<int>( positionInCellSpace.x ), static_cast<int>( positionInCellSpace.y ) );
    if ( cellPosition.x < 0 || cellPosition.y < 0 || cellPosition.x >= sNumSpawnPointsSide || cellPosition.y >= sNumSpawnPointsSide )
    {
        return std::nullopt;
    }
    else
    {
        return cellPosition;
    }
}

glm::vec3 SectorSpawner::ToWorldPosition( const glm::ivec2& cellPosition ) const
{
    return glm::vec3( 0.0f );
}

glm::ivec2 SectorSpawner::GetRandomCell() const
{
    return glm::ivec2( gRand( 0, sNumSpawnPointsSide ), gRand( 0, sNumSpawnPointsSide ) );
}

} // namespace Hexterminate
