// Copyright 2020 Pedro Nunes
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

#include <sstream>

#include <math/constants.h>

#include "requests/infinitewarrequest.h"

#include "blackboard.h"
#include "faction/faction.h"
#include "fleet/fleet.h"
#include "hexterminate.h"
#include "requests/invasionrequestinfo.h"
#include "sector/galaxy.h"
#include "sector/sector.h"
#include "sector/sectorinfo.h"
#include "ship/ship.h"
#include "ship/shipinfo.h"
#include "stringaux.h"

namespace Hexterminate
{

InfiniteWarRequest::InfiniteWarRequest( RequestManager* pRequestManager )
    : ImperialRequest( pRequestManager )
    , m_UpdateInterval( 5.0f )
{
}

void InfiniteWarRequest::OnBegin()
{
    AddHomeworldGoal( FactionId::Pirate );
    AddHomeworldGoal( FactionId::Marauders );
    AddHomeworldGoal( FactionId::Ascent );
    AddHomeworldGoal( FactionId::Iriani );
    AddHomeworldGoal( FactionId::Hegemon );
}

void InfiniteWarRequest::OnSuccess()
{
}

void InfiniteWarRequest::Update( float delta )
{
    ImperialRequest::Update( delta );

    if ( m_UpdateInterval <= 0.0f )
    {
        UpdateHomeworldsArc( delta );

        m_UpdateInterval = 1.0f;
    }
    else
    {
        m_UpdateInterval = 0.0f;
    }
}

void InfiniteWarRequest::AddHomeworldGoal( FactionId factionId )
{
    Faction* pFaction = g_pGame->GetFaction( factionId );
    SectorInfo* pHomeworld = pFaction->GetHomeworld();
    if ( pHomeworld != nullptr )
    {
        std::stringstream ss;
        ss << "Conquer homeworld (" << ToString( factionId ) << ")";
        RequestGoalSharedPtr pGoal = std::make_shared<RequestGoal>( pHomeworld, ss.str(), Genesis::Color( 0.0f, 0.0f, 0.6f, 0.6f ) );
        AddGoal( pGoal );
    }
}

void InfiniteWarRequest::UpdateHomeworldsArc( float delta )
{
    RequestGoalSharedPtrList goals = GetGoals();
    for ( RequestGoalSharedPtr pGoal : goals )
    {
        if ( pGoal->GetSectorInfo()->GetFaction()->GetFactionId() == FactionId::Empire )
        {
            RemoveGoal( pGoal );
            break;
        }
    }

    if ( goals.empty() )
    {
        BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
        pBlackboard->Add( "#infinity_war_finished" );
    }
}

} // namespace Hexterminate
