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

#include <algorithm>

#include "requests/invasionrequestinfo.h"

#include "blackboard.h"
#include "faction/faction.h"
#include "fleet/fleet.h"
#include "fleet/fleetbehaviour.h"
#include "hexterminate.h"
#include "requests/invasionrequest.h"
#include "requests/requestmanager.h"

namespace Hexterminate
{

std::string InvasionRequestInfo::GetBlackboardTag( FactionId factionId )
{
    static const std::string sBlackboardTags[ static_cast<size_t>( FactionId::Count ) ] = {
        "#invasion_neutral",
        "#invasion_player",
        "#invasion_empire",
        "#invasion_ascent",
        "#invasion_pirate",
        "#invasion_marauders",
        "#invasion_iriani",
        "#invasion_special"
    };
    return sBlackboardTags[ static_cast<size_t>( factionId ) ];
}

ImperialRequestSharedPtr InvasionRequestInfo::TryInstantiate( RequestManager* pRequestManager ) const
{
    if ( CommonInstantiationChecks( pRequestManager ) == false )
        return ImperialRequestSharedPtr();

    // Is any invasion currently active?
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    FactionId invadingFaction = FactionId::Neutral;
    int invasionCount = 0;
    std::string invasionTag;
    for ( unsigned int factionIdx = 0u; factionIdx < static_cast<unsigned int>( FactionId::Count ); factionIdx++ )
    {
        FactionId factionId = static_cast<FactionId>( factionIdx );
        invasionTag = GetBlackboardTag( factionId );
        invasionCount = pBlackboard->Get( invasionTag );

        // If we have an active invasion, we need to make sure that we aren't exceeding the number of
        // spawned invasions.
        if ( invasionCount > 0 && GetActiveInvasions( pRequestManager, factionId ) < invasionCount )
        {
            invadingFaction = factionId;
            break;
        }
    }

    // If there are no invading factions, we don't do anything.
    if ( invasionCount == 0 )
    {
        return ImperialRequestSharedPtr();
    }

    SectorInfo* pSectorInfo = FindSector();
    if ( pSectorInfo && IsDuplicateSector( pRequestManager, pSectorInfo ) == false )
    {
        return std::make_shared<InvasionRequest>( pRequestManager, pSectorInfo, invadingFaction );
    }
    else
    {
        return ImperialRequestSharedPtr();
    }
}

int InvasionRequestInfo::GetActiveInvasions( RequestManager* pRequestManager, FactionId factionId ) const
{
    int activeInvasions = 0;
    const ImperialRequestList& requests = pRequestManager->GetRequests();
    for ( auto& pRequest : requests )
    {
        if ( pRequest->GetType() == ImperialRequestType::Invasion )
        {
            InvasionRequest* pInvasionRequest = static_cast<InvasionRequest*>( pRequest.get() );
            if ( pInvasionRequest->GetInvadingFaction() == factionId )
            {
                activeInvasions++;
            }
        }
    }
    return activeInvasions;
}

SectorInfo* InvasionRequestInfo::FindSector() const
{
    Faction* pEmpireFaction = g_pGame->GetFaction( FactionId::Empire );
    const SectorInfoVector& controlledSectors = pEmpireFaction->GetControlledSectors();
    if ( controlledSectors.empty() )
    {
        return nullptr;
    }
    else
    {
        const unsigned int r = rand() % controlledSectors.size();
        return controlledSectors[ r ];
    }
}

} // namespace Hexterminate
