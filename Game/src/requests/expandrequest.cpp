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

#include "requests/expandrequest.h"

#include <cassert>
#include <sstream>

#include "hexterminate.h"
#include "player.h"
#include "sector/sector.h"
#include <genesis.h>

namespace Hexterminate
{

ExpandRequest::ExpandRequest( RequestManager* pRequestManager, SectorInfo* pSectorInfo )
    : ImperialRequest( pRequestManager )
    , m_pSectorInfo( pSectorInfo )
    , m_Reward( 0 )
{
    SDL_assert( pSectorInfo != nullptr );
}

void ExpandRequest::OnBegin()
{
    ImperialRequest::OnBegin();

    m_Reward = m_pSectorInfo->GetConquestReward();

    std::stringstream ss;
    ss << "Claim sector: +" << m_Reward << " influence";

    m_pGoal = std::make_shared<RequestGoal>( m_pSectorInfo, ss.str() );
    AddGoal( m_pGoal );

#ifdef _DEBUG
    int x, y;
    m_pSectorInfo->GetCoordinates( x, y );
    Genesis::FrameWork::GetLogger()->LogInfo( "Starting ExpandRequest on sector %d / %d", x, y );
#endif
}

void ExpandRequest::Update( float delta )
{
    ImperialRequest::Update( delta );

    // If the player was in this sector and it is now ours, then the request is completed successfully
    Sector* pCurrentSector = g_pGame->GetCurrentSector();
    if ( pCurrentSector != nullptr && pCurrentSector->GetSectorInfo() == m_pSectorInfo && pCurrentSector->IsPlayerVictorious() )
    {
        OnSuccess();
    }
    // But if the sector now belongs to the Empire without the player interfering, then the request fails
    else if ( m_pSectorInfo->GetFaction() == g_pGame->GetFaction( FactionId::Empire ) )
    {
        OnFailure();
    }
}

void ExpandRequest::OnSuccess()
{
    ImperialRequest::OnSuccess();

    std::stringstream ss;
    ss << "Excellent, the sector is now ours. Our influence with Imperial HQ has increased by " << m_Reward << ".";
    g_pGame->AddIntel( GameCharacter::FleetIntelligence, ss.str() );

    RemoveGoal( m_pGoal );
    m_pGoal.reset();
}

void ExpandRequest::OnFailure()
{
    ImperialRequest::OnFailure();

    RemoveGoal( m_pGoal );
    m_pGoal.reset();
}

int ExpandRequest::GetConquestReward( const SectorInfo* pSectorInfo ) const
{
    return ( pSectorInfo == m_pSectorInfo ) ? ExpandRequestReward : 0;
}

} // namespace Hexterminate
