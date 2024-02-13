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

#include "requests/imperialrequestinfo.h"

#include "hexterminate.h"
#include "requests/imperialrequest.h"
#include "requests/requestmanager.h"

#include <genesis.h>

namespace Hexterminate
{

bool ImperialRequestInfo::CommonInstantiationChecks( RequestManager* pRequestManager ) const
{
    if ( g_pGame->GetPlayedTime() <= GetMinimumTimeBeforeRequest() )
    {
        return false;
    }
    else if ( pRequestManager->GetActiveCount( GetType() ) >= GetMaximumRequests() )
    {
        return false;
    }
    else if ( g_pGame->GetPlayedTime() <= pRequestManager->GetPreviousStartTime( GetType() ) + GetMinimumTimeBetweenRequests() )
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool ImperialRequestInfo::IsDuplicateSector( RequestManager* pRequestManager, SectorInfo* pSectorInfo ) const
{
    const ImperialRequestList& requests = pRequestManager->GetRequests();
    for ( auto& pRequest : requests )
    {
        ImperialRequest* pOtherRequest = pRequest.get();
        if ( pOtherRequest->GoalExists( pSectorInfo ) )
        {
            return true;
        }
    }

    return false;
}

} // namespace Hexterminate
