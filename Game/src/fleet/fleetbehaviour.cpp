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
#include <list>
#include <string>
#include <vector>

#include "fleet/fleet.h"
#include "fleet/fleetbehaviour.h"
#include "globals.h"
#include "hexterminate.h"
#include "misc/randomshuffle.h"
#include "sector/galaxy.h"
#include "sector/sector.h"
#include "sector/sectorinfo.h"
#include <genesis.h>
#include <logger.h>
#include <math/misc.h>

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// FleetBehaviour
///////////////////////////////////////////////////////////////////////////////

FleetBehaviour::FleetBehaviour( Fleet* pFleet )
    : m_pFleet( pFleet )
    , m_ClaimsSectors( false )
    , m_AssistsFriendlies( true )
    , m_JoinsTheFray( false )
{
}

bool FleetBehaviour::ProcessTurn()
{
    // Do not claim any further sectors if the faction has started collapsing.
    if ( m_ClaimsSectors && m_pFleet->GetFaction()->IsCollapsing() )
    {
        m_ClaimsSectors = false;
    }

    if ( m_pFleet->IsEngaged() )
    {
        return false;
    }
    else if ( m_pFleet->HasArrived() )
    {
        SectorInfo* pCurrentSector = m_pFleet->GetCurrentSector();
        if ( pCurrentSector == nullptr )
        {
            return false;
        }
        else if ( pCurrentSector->HasHyperspaceInhibitor() == false && pCurrentSector->IsPersonal() == false && pCurrentSector->GetContestedFleets().size() < cMaxContestingFleets )
        {
            // Contest sector if it is not ours
            if ( Faction::sIsEnemyOf( pCurrentSector->GetFaction(), m_pFleet->GetFaction() ) )
            {
                pCurrentSector->Contest();
                return false;
            }
            else // Or if there is an enemy fleet present
            {
                FleetWeakPtrList contestedFleets = pCurrentSector->GetContestedFleets();
                for ( auto& pContestingFleetWeakPtr : contestedFleets )
                {
                    FleetSharedPtr pContestingFleet = pContestingFleetWeakPtr.lock();
                    if ( pContestingFleet != nullptr && Faction::sIsEnemyOf( pContestingFleet->GetFaction(), m_pFleet->GetFaction() ) )
                    {
                        pCurrentSector->Contest();
                        return false;
                    }
                }

                // Otherwise, there's nothing to do here and we go back to being idle
                m_pFleet->SetState( FleetState::Idle );
                return true;
            }
        }
        else
        {
            m_pFleet->SetState( FleetState::Idle );
        }
    }

    SectorInfo* pSectorToAssist = GetSectorToAssist();
    if ( pSectorToAssist != nullptr )
    {
#ifdef _DEBUG
        int x, y;
        pSectorToAssist->GetCoordinates( x, y );
        Genesis::FrameWork::GetLogger()->LogInfo(
            "[%s] Fleet diverted to assist sector [%d,%d]. %d fleets now assisting the sector.",
            m_pFleet->GetFaction()->GetName().c_str(),
            x,
            y,
            GetAssistingFleetsCount( pSectorToAssist ) );
#endif

        NotifyAssist( pSectorToAssist );

        if ( pSectorToAssist != m_pFleet->GetCurrentSector() )
        {
            m_pFleet->SetDestinationSector( pSectorToAssist );
        }

        return false;
    }

    return true;
}

void FleetBehaviour::NotifyBattleWon()
{
    if ( m_ClaimsSectors )
    {
        SectorInfo* pCurrentSector = m_pFleet->GetCurrentSector();
        if ( pCurrentSector != nullptr )
        {
            pCurrentSector->SetFaction( m_pFleet->GetFaction(), false, false );
        }
    }
}

void FleetBehaviour::NotifyAssist( SectorInfo* pSectorToAssist )
{
    // If the player isn't in the sector this fleet will move to, then there is no need to notify the player
    if ( g_pGame->GetCurrentSector() == nullptr || g_pGame->GetCurrentSector()->GetSectorInfo() != pSectorToAssist )
    {
        return;
    }

    if ( Faction::sIsEnemyOf( m_pFleet->GetFaction(), g_pGame->GetPlayerFaction() ) )
    {
        g_pGame->AddFleetCommandIntel( "We have detected a quantum wavefront, an unknown fleet will soon emerge from hyperspace." );
    }
    else
    {
        g_pGame->AddFleetCommandIntel( "Captain, an Imperial fleet is on its way to join forces with us." );
    }
}

SectorInfo* FleetBehaviour::GetSectorToAssist() const
{
    // A fleet should only go out of its way if it is flagged as either capable of assisting friendly fleets
    // or as wishing to join on-going battles.
    if ( !m_AssistsFriendlies && !m_JoinsTheFray )
    {
        return nullptr;
    }

    for ( int i = 0; i < (int)FactionId::Count; ++i )
    {
        Faction* pFaction = g_pGame->GetFaction( static_cast<FactionId>( i ) );

        // If we only assist friendlies, disregard any factions we are not allied with
        if ( m_AssistsFriendlies && Faction::sIsEnemyOf( m_pFleet->GetFaction(), pFaction ) )
        {
            continue;
        }

        const FleetList& fleets = pFaction->GetFleets();
        for ( auto& pOtherFleet : fleets )
        {
            if ( pOtherFleet == nullptr || pOtherFleet.get() == m_pFleet || !pOtherFleet->IsEngaged() || pOtherFleet->IsRegionalFleet() )
            {
                continue;
            }

            if ( m_pFleet->IsInRangeOf( pOtherFleet ) )
            {
                SectorInfo* pSectorToAssist = pOtherFleet->GetCurrentSector();

                // Do not allow more than 8 fleets in the same sector.
                if ( pSectorToAssist->GetContestedFleets().size() >= cMaxContestingFleets )
                {
                    continue;
                }
                // Limit the number of how many simultaneous fleets we can have assisting a sector to avoid having them bunch up.
                if ( GetAssistingFleetsCount( pSectorToAssist ) >= 3 )
                {
                    continue;
                }
                // We are in range of the other fleet, but does that fleet actually need help?
                else if ( Faction::sIsEnemyOf( pSectorToAssist->GetFaction(), m_pFleet->GetFaction() ) )
                {
                    return pSectorToAssist;
                }
                else
                {
                    // If the sector does belong to our faction already then we need to check if it is being contested by hostile forces
                    FleetWeakPtrList contestedFleets = pSectorToAssist->GetContestedFleets();
                    for ( auto& pContestingFleetWeakPtr : contestedFleets )
                    {
                        FleetSharedPtr pContestingFleet = pContestingFleetWeakPtr.lock();
                        if ( pContestingFleet != nullptr && Faction::sIsEnemyOf( pContestingFleet->GetFaction(), m_pFleet->GetFaction() ) )
                        {
                            return pSectorToAssist;
                        }
                    }
                }
            }
        }
    }

    return nullptr;
}

bool FleetBehaviour::CanAttackSector( SectorInfo* pSectorInfo ) const
{
    if ( pSectorInfo->HasHyperspaceInhibitor() && pSectorInfo->GetFaction() != m_pFleet->GetFaction() && pSectorInfo->GetContestedFleets().size() < cMaxContestingFleets && pSectorInfo->IsPersonal() == false )
    {
        return false;
    }
    else if ( m_pFleet->GetFaction()->GetFactionId() == FactionId::Empire )
    {
        return true;
    }
    else
    {
        // For 45 minutes after the game starts, Imperial space should be left alone by hostiles
        const float imperialSafeTime = 45.0f * 60.0f;
        if ( pSectorInfo->GetFaction()->GetFactionId() == FactionId::Empire && g_pGame->GetPlayedTime() < imperialSafeTime )
        {
            return false;
        }
        else
        {
            return true;
        }
    }
}

// Find out how many fleets are already in the process of assisting a particular sector
int FleetBehaviour::GetAssistingFleetsCount( SectorInfo* pSectorToAssist ) const
{
    int count = 0;
    const FleetList& fleets = m_pFleet->GetFaction()->GetFleets();
    for ( auto& pFleet : fleets )
    {
        if ( pFleet->GetDestinationSector() == pSectorToAssist )
        {
            count++;
        }
    }
    return count;
}

///////////////////////////////////////////////////////////////////////////////
// FleetBehaviourRoaming
///////////////////////////////////////////////////////////////////////////////

FleetBehaviourRoaming::FleetBehaviourRoaming( Fleet* pFleet, const SectorInfo* pBaseSector )
    : FleetBehaviour( pFleet )
    , m_pBaseSector( pBaseSector )
{
    m_ClaimsSectors = false;
    m_AssistsFriendlies = true;
    m_JoinsTheFray = true;

    SDL_assert( m_pBaseSector != nullptr );
}

bool FleetBehaviourRoaming::HasValidBaseSector() const
{
    return m_pBaseSector->GetFaction() == m_pFleet->GetFaction();
}

void FleetBehaviourRoaming::RelocateBaseSector()
{
    // Attempt to relocate to a sector controlled by this fleet's faction.
    const SectorInfoVector& controlledSectors = m_pFleet->GetFaction()->GetControlledSectors();
    if ( controlledSectors.empty() == false )
    {
        m_pBaseSector = controlledSectors[ rand() % controlledSectors.size() ];
    }
    else
    {
        // If that fails (because the faction has been conquered), go wreck some havok in Imperial space.
        const SectorInfoVector& empireControlledSectors = g_pGame->GetFaction( FactionId::Empire )->GetControlledSectors();
        if ( empireControlledSectors.empty() == false )
        {
            m_pBaseSector = empireControlledSectors[ rand() % empireControlledSectors.size() ];
        }
    }
}

bool FleetBehaviourRoaming::TryAttackSector( SectorInfo* pSector )
{
    if ( CanAttackSector( pSector ) )
    {
        int x, y;
        pSector->GetCoordinates( x, y );

        m_pFleet->SetDestination( ( static_cast<float>( x ) + 0.5f ) / NumSectorsX, ( static_cast<float>( y ) + 0.5f ) / NumSectorsY );
        return true;
    }
    else
    {
        return false;
    }
}

bool FleetBehaviourRoaming::ProcessTurn()
{
    if ( FleetBehaviour::ProcessTurn() == false )
    {
        return false;
    }

    if ( m_pFleet->GetState() == FleetState::Idle )
    {
        // Check if our base sector is still valid. As the game progresses,
        // it is possible that the base sector gets captured by a different
        // faction.
        if ( HasValidBaseSector() == false )
        {
            RelocateBaseSector();
        }

        int x, y;
        m_pBaseSector->GetCoordinates( x, y );

        const int maximumRoamingDistance = 6 + rand() % 3;
        const int roamingDistanceX = rand() % maximumRoamingDistance;
        const int roamingDistanceY = rand() % maximumRoamingDistance;
        x = gClamp<int>( x - roamingDistanceX / 2 + roamingDistanceX, 0, NumSectorsX - 1 );
        y = gClamp<int>( y - roamingDistanceY / 2 + roamingDistanceY, 0, NumSectorsY - 1 );

        // Attempt to attack a sector near our base sector.
        // If that doesn't work, try a random imperial sector.
        if ( TryAttackSector( g_pGame->GetGalaxy()->GetSectorInfo( x, y ) ) == false )
        {
            const SectorInfoVector& imperialSectors = g_pGame->GetFaction( FactionId::Empire )->GetControlledSectors();
            if ( imperialSectors.empty() == false )
            {
                TryAttackSector( imperialSectors[ rand() % imperialSectors.size() ] );
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// FleetBehaviourRaiding
///////////////////////////////////////////////////////////////////////////////

FleetBehaviourRaiding::FleetBehaviourRaiding( Fleet* pFleet )
    : FleetBehaviour( pFleet )
{
    m_ClaimsSectors = true;
    m_AssistsFriendlies = true;
    m_JoinsTheFray = true;
}

bool FleetBehaviourRaiding::ProcessTurn()
{
    if ( FleetBehaviour::ProcessTurn() == false )
    {
        return false;
    }

    if ( m_pFleet->GetState() == FleetState::Idle )
    {
        const unsigned int x = rand() % NumSectorsX;
        const unsigned int y = rand() % NumSectorsY;
        SectorInfo* pSector = g_pGame->GetGalaxy()->GetSectorInfo( x, y );

        if ( CanAttackSector( pSector ) )
        {
            const float coordX = ( static_cast<float>( x ) + 0.5f ) / NumSectorsX;
            const float coordY = ( static_cast<float>( y ) + 0.5f ) / NumSectorsY;
            m_pFleet->SetDestination( coordX, coordY );
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// FleetBehaviourExpansionist
///////////////////////////////////////////////////////////////////////////////

FleetBehaviourExpansionist::FleetBehaviourExpansionist( Fleet* pFleet )
    : FleetBehaviour( pFleet )
{
    m_ClaimsSectors = true;
    m_AssistsFriendlies = true;
    m_JoinsTheFray = false;
}

bool FleetBehaviourExpansionist::ProcessTurn()
{
    if ( FleetBehaviour::ProcessTurn() == false )
    {
        return false;
    }

    const SectorInfoVector& controlledSectors = m_pFleet->GetFaction()->GetControlledSectors();
    SectorInfoVector hostileBorderingSectors;
    Faction* pFleetFaction = m_pFleet->GetFaction();
    for ( auto& sector : controlledSectors )
    {
        sGetHostileBorderingSectors( pFleetFaction, sector, hostileBorderingSectors );
    }

    SectorInfoVector::iterator it = std::unique( hostileBorderingSectors.begin(), hostileBorderingSectors.end() );
    hostileBorderingSectors.resize( std::distance( hostileBorderingSectors.begin(), it ) );
    if ( hostileBorderingSectors.empty() == false )
    {
        RandomShuffle::Shuffle( hostileBorderingSectors.begin(), hostileBorderingSectors.end() );
        SectorInfo* pDestinationSector = hostileBorderingSectors.front();

        if ( CanAttackSector( pDestinationSector ) )
        {
            int sectorX, sectorY;
            pDestinationSector->GetCoordinates( sectorX, sectorY );
            m_pFleet->SetDestination( ( static_cast<float>( sectorX ) + 0.5f ) / NumSectorsX, ( static_cast<float>( sectorY ) + 0.5f ) / NumSectorsY );

#if _DEBUG
            Genesis::FrameWork::GetLogger()->LogInfo(
                "[%s] Fleet expanding to sector [%d,%d]",
                m_pFleet->GetFaction()->GetName().c_str(),
                sectorX,
                sectorY );
#endif
        }
    }

    return true;
}

void FleetBehaviourExpansionist::sGetHostileBorderingSectors( Faction* pFaction, const SectorInfo* pAroundSector, SectorInfoVector& hostileSectors )
{
    int x, y;
    pAroundSector->GetCoordinates( x, y );
    for ( int x2 = x - 1; x2 <= x + 1; ++x2 )
    {
        for ( int y2 = y - 1; y2 <= y + 1; ++y2 )
        {
            if ( x2 < 0 || x2 >= NumSectorsX || y2 < 0 || y2 >= NumSectorsY )
            {
                continue;
            }
            else if ( x == x2 && y == y2 )
            {
                continue;
            }

            SectorInfo* pSectorInfo = g_pGame->GetGalaxy()->GetSectorInfo( x2, y2 );
            if ( Faction::sIsEnemyOf( pFaction, pSectorInfo->GetFaction() ) )
            {
                hostileSectors.push_back( pSectorInfo );
            }
        }
    }
}

} // namespace Hexterminate