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

#include <glm/gtc/matrix_access.hpp>

#include <physics/rigidbody.h>

#include "fleet/fleetcommand.h"
#include "hexterminate.h"
#include "sector/sector.h"
#include "ship/ship.h"

namespace Hexterminate
{

FleetCommand::FleetCommand()
    : m_pLeader( nullptr )
{
}

void FleetCommand::AssignShip( Ship* pShip )
{
    m_Ships.push_back( pShip );
}

void FleetCommand::AssignLeader( Ship* pShip )
{
    m_pLeader = pShip;
}

// Setup the spatial relationship between each ship and the fleet's leader.
void FleetCommand::SetupRelationships()
{
    SDL_assert( HasLeader() );
    if ( HasLeader() )
    {
        glm::vec3 leaderPosition = GetLeader()->GetRigidBody()->GetPosition();
        for ( auto& pShip : m_Ships )
        {
            glm::vec3 shipPosition = pShip->GetRigidBody()->GetPosition();
            glm::vec3 diff = shipPosition - leaderPosition;
            m_Relationships.push_back( glm::vec2( diff.x, diff.y ) );
        }
    }
}

// Decide what the fleet should do.
// The fleet leader will either want to attack enemies or patrol if there are no enemies in the sector.
// The other ships will follow the leader, staying in formation until the enemy is close by. At that point,
// they break formation and engage on their own.
// If the leader has been destroyed, then all ships are set to free engagement mode.
void FleetCommand::Update()
{
    SDL_assert( HasLeader() );

    if ( GetLeader()->IsDestroyed() )
    {
        for ( auto& pShip : m_Ships )
        {
            pShip->SetFleetCommandOrder( FleetCommandOrder::Engage );
        }
    }
    else
    {
        const ScanForEnemiesResult result = ScanForEnemies();
        if ( result == ScanForEnemiesResult::EnemiesInRange || result == ScanForEnemiesResult::EnemiesOutOfRange )
        {
            GetLeader()->SetFleetCommandOrder( FleetCommandOrder::Engage );
        }
        else
        {
            GetLeader()->SetFleetCommandOrder( FleetCommandOrder::Patrol );
        }

        if ( GetLeader()->GetFaction()->GetUsesFormations() )
        {
            const bool isLeaderDocked = ( GetLeader()->GetDockingState() != DockingState::Undocked );
            const glm::mat4x4& tr = GetLeader()->GetTransform();
            const glm::vec2 fwd( glm::column( tr, 1 ) );
            const glm::vec2 rgt( glm::column( tr, 0 ) );
            const glm::vec2 leaderPosition( glm::column( tr, 3 ) );

            SDL_assert( m_Ships.size() == m_Relationships.size() );
            for ( size_t i = 0, s = m_Ships.size(); i < s; ++i )
            {
                Ship* pShip = m_Ships[ i ];
                if ( result == ScanForEnemiesResult::EnemiesInRange || isLeaderDocked )
                {
                    pShip->SetFleetCommandOrder( FleetCommandOrder::Engage );
                }
                else if ( result == ScanForEnemiesResult::EnemiesOutOfRange || result == ScanForEnemiesResult::NoEnemies )
                {
                    pShip->SetFleetCommandOrder( FleetCommandOrder::StickToFormation );
                    pShip->SetFormationPosition( leaderPosition + rgt * m_Relationships[ i ].x + fwd * m_Relationships[ i ].y );
                    pShip->SetFormationDirection( fwd );
                }
            }
        }
        else
        {
            for ( auto& pShip : m_Ships )
            {
                pShip->SetFleetCommandOrder( FleetCommandOrder::Engage );
            }
        }
    }
}

ScanForEnemiesResult FleetCommand::ScanForEnemies() const
{
    const float searchRange = 1000.0f;
    const glm::vec3& leaderPosition = GetLeader()->GetTowerPosition();
    const ShipList& ships = g_pGame->GetCurrentSector()->GetShipList();
    unsigned int enemiesDetected = 0;
    for ( auto& pShip : ships )
    {
        if ( Faction::sIsEnemyOf( GetLeader()->GetFaction(), pShip->GetFaction() ) && pShip->IsDestroyed() == false )
        {
            enemiesDetected++;

            const glm::vec3& shipPosition = pShip->GetTowerPosition();
            if ( glm::distance( shipPosition, leaderPosition ) < searchRange )
            {
                return ScanForEnemiesResult::EnemiesInRange;
            }
        }
    }

    if ( enemiesDetected == 0 )
    {
        return ScanForEnemiesResult::NoEnemies;
    }
    else
    {
        return ScanForEnemiesResult::EnemiesOutOfRange;
    }
}

} // namespace Hexterminate
