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

#include "requests/defendrequest.h"

#include <cassert>
#include <sstream>

#include "fleet/fleet.h"
#include "hexterminate.h"
#include "player.h"
#include "sector/sector.h"
#include <genesis.h>

namespace Hexterminate
{

DefendRequest::DefendRequest( RequestManager* pRequestManager, SectorInfo* pSectorInfo )
    : ImperialRequest( pRequestManager )
    , m_pSectorInfo( pSectorInfo )
    , m_Reward( 0 )
{
    SDL_assert( pSectorInfo != nullptr );
}

void DefendRequest::OnBegin()
{
    ImperialRequest::OnBegin();

    m_Reward = m_pSectorInfo->GetConquestReward();

    std::stringstream ss;
    ss << "Defend sector: +" << m_Reward << " influence";

    m_pGoal = std::make_shared<RequestGoal>( m_pSectorInfo, ss.str() );
    AddGoal( m_pGoal );

#ifdef _DEBUG
    int x, y;
    m_pSectorInfo->GetCoordinates( x, y );
    Genesis::FrameWork::GetLogger()->LogInfo( "Starting DefendRequest on sector %d / %d", x, y );
#endif
}

void DefendRequest::Update( float delta )
{
    ImperialRequest::Update( delta );

    // If the player was in this sector and there are no hostile contesting fleets, then the request is completed successfully
    Sector* pCurrentSector = g_pGame->GetCurrentSector();
    if ( pCurrentSector != nullptr && pCurrentSector->GetSectorInfo() == m_pSectorInfo )
    {
        Faction* pEmpireFaction = g_pGame->GetFaction( FactionId::Empire );
        auto contestedFleets = m_pSectorInfo->GetContestedFleets();
        for ( auto& contestedFleet : contestedFleets )
        {
            FleetSharedPtr pFleet = contestedFleet.lock();
            if ( pFleet != nullptr && !pFleet->IsEngaged() && Faction::sIsEnemyOf( pFleet->GetFaction(), pEmpireFaction ) )
            {
                OnSuccess();
                break;
            }
        }
    }
    // But if the sector no longer belongs to the Empire, then the request fails
    else if ( m_pSectorInfo->GetFaction() != g_pGame->GetFaction( FactionId::Empire ) )
    {
        OnFailure();
    }
    // We also "fail" if the sector no longer has any contesting fleets.
    // This can happen if a faction that doesn't claim sectors was contesting it but leaves.
    else if ( m_pSectorInfo->IsContested() == false )
    {
        OnFailure();
    }
}

void DefendRequest::OnSuccess()
{
    ImperialRequest::OnSuccess();

    std::stringstream ss;
    ss << "The sector has been successfully defended. Our influence with Imperial HQ has increased by " << m_Reward << ".";
    g_pGame->AddIntel( GameCharacter::FleetIntelligence, ss.str() );

    RemoveGoal( m_pGoal );
    m_pGoal.reset();
}

void DefendRequest::OnFailure()
{
    ImperialRequest::OnFailure();

    RemoveGoal( m_pGoal );
    m_pGoal.reset();
}

int DefendRequest::GetConquestReward( const SectorInfo* pSectorInfo ) const
{
    return ( pSectorInfo == m_pSectorInfo ) ? DefendRequestReward : 0;
}

} // namespace Hexterminate
