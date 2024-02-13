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

#include "requests/expandrequestinfo.h"

#include "faction/faction.h"
#include "fleet/fleetbehaviour.h"
#include "hexterminate.h"
#include "misc/randomshuffle.h"
#include "requests/expandrequest.h"
#include "requests/requestmanager.h"

namespace Hexterminate
{

ImperialRequestSharedPtr ExpandRequestInfo::TryInstantiate( RequestManager* pRequestManager ) const
{
    if ( CommonInstantiationChecks( pRequestManager ) == false )
        return ImperialRequestSharedPtr();

    SectorInfo* pSectorInfo = FindSector();
    if ( pSectorInfo && IsDuplicateSector( pRequestManager, pSectorInfo ) == false )
    {
        return std::make_shared<ExpandRequest>( pRequestManager, pSectorInfo );
    }
    else
    {
        return ImperialRequestSharedPtr();
    }
}

SectorInfo* ExpandRequestInfo::FindSector() const
{
    Faction* pFaction = g_pGame->GetFaction( FactionId::Empire );
    const SectorInfoVector& controlledSectors = pFaction->GetControlledSectors();
    SectorInfoVector hostileBorderingSectors;
    for ( auto& pSectorInfo : controlledSectors )
    {
        FleetBehaviourExpansionist::sGetHostileBorderingSectors( pFaction, pSectorInfo, hostileBorderingSectors );
    }

    SectorInfoVector::iterator it = std::unique( hostileBorderingSectors.begin(), hostileBorderingSectors.end() );
    hostileBorderingSectors.resize( std::distance( hostileBorderingSectors.begin(), it ) );
    if ( hostileBorderingSectors.empty() == false )
    {
        RandomShuffle::Shuffle( hostileBorderingSectors.begin(), hostileBorderingSectors.end() );
        return hostileBorderingSectors.front();
    }
    else
    {
        return nullptr;
    }
}

} // namespace Hexterminate
