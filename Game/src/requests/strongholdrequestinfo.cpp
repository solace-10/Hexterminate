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

#include "requests/strongholdrequestinfo.h"

#include "faction/faction.h"
#include "fleet/fleetbehaviour.h"
#include "hexterminate.h"
#include "requests/requestmanager.h"
#include "requests/strongholdrequest.h"

namespace Hexterminate
{

ImperialRequestSharedPtr StrongholdRequestInfo::TryInstantiate( RequestManager* pRequestManager ) const
{
    if ( CommonInstantiationChecks( pRequestManager ) == false )
        return ImperialRequestSharedPtr();

    SectorInfo* pSectorInfo = FindSector();
    if ( pSectorInfo && pSectorInfo->IsPersonal() == false && IsDuplicateSector( pRequestManager, pSectorInfo ) == false )
    {
        return std::make_shared<StrongholdRequest>( pRequestManager, pSectorInfo );
    }
    else
    {
        return ImperialRequestSharedPtr();
    }
}

SectorInfo* StrongholdRequestInfo::FindSector() const
{
    Faction* pFaction = g_pGame->GetFaction( FactionId::Neutral );
    SectorInfoVector controlledSectors = pFaction->GetControlledSectors();

    if ( controlledSectors.empty() == false )
    {
        return controlledSectors[ rand() % controlledSectors.size() ];
    }
    else
    {
        return nullptr;
    }
}

} // namespace Hexterminate
