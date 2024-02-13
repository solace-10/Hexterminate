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

#include "requests/imperialrequest.h"
#include "hexterminate.h"
#include "requests/requestmanager.h"
#include "ship/shipinfo.h"

namespace Hexterminate
{

ImperialRequest::ImperialRequest( RequestManager* pRequestManager )
    : m_pRequestManager( pRequestManager )
    , m_StartTime( 0.0f )
    , m_ThreatScore( 0 )
{
}

void ImperialRequest::OnBegin()
{
    m_StartTime = g_pGame->GetPlayedTime();
}

void ImperialRequest::OnSuccess()
{
    m_pRequestManager->Remove( shared_from_this() );
}

void ImperialRequest::OnFailure()
{
    m_pRequestManager->Remove( shared_from_this() );
}

void ImperialRequest::AddGoal( RequestGoalSharedPtr pGoal )
{
    m_Goals.push_back( pGoal );
}

void ImperialRequest::RemoveGoal( RequestGoalSharedPtr pGoal )
{
    m_Goals.remove( pGoal );
}

void ImperialRequest::RemoveGoal( const SectorInfo* pSectorInfo )
{
    for ( RequestGoalSharedPtrList::iterator it = m_Goals.begin(); it != m_Goals.end(); )
    {
        if ( ( *it )->GetSectorInfo() == pSectorInfo )
        {
            it = m_Goals.erase( it );
        }
        else
        {
            it++;
        }
    }
}

bool ImperialRequest::GoalExists( const SectorInfo* pSectorInfo ) const
{
    for ( auto& pGoal : m_Goals )
    {
        if ( pGoal->GetSectorInfo() != nullptr && pGoal->GetSectorInfo() == pSectorInfo )
        {
            return true;
        }
    }

    return false;
}

unsigned int ImperialRequest::GetShipThreatScore( const std::string& ship, FactionId factionId ) const
{
    const ShipInfo* pShipInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetFaction( factionId ), ship );
    SDL_assert( pShipInfo != nullptr );
    return pShipInfo->GetPoints();
}

} // namespace Hexterminate
