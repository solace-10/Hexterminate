// Copyright 2018 Pedro Nunes
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

#include "requests/arbiterrequestinfo.h"

#include "faction/faction.h"
#include "fleet/fleet.h"
#include "fleet/fleetbehaviour.h"
#include "hexterminate.h"
#include "requests/arbiterrequest.h"
#include "requests/campaigntags.h"
#include "requests/requestmanager.h"

namespace Hexterminate
{

ImperialRequestSharedPtr ArbiterRequestInfo::TryInstantiate( RequestManager* pRequestManager ) const
{
    if ( CommonInstantiationChecks( pRequestManager ) == false )
    {
        return ImperialRequestSharedPtr();
    }
    else if ( g_pGame->GetBlackboard()->Get( sArbitersDestroyed ) >= 4 )
    {
        return ImperialRequestSharedPtr();
    }

    // Do not spawn this request unless the player has begun the Iriani arc,
    // as the player can't enter Iriani space anyway due to the hyperspace inhibitors.
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    if ( pBlackboard->Exists( sConquerIrianiPrime ) == false )
    {
        return ImperialRequestSharedPtr();
    }

    SectorInfo* pSectorInfo = FindSector();
    if ( pSectorInfo && IsDuplicateSector( pRequestManager, pSectorInfo ) == false )
    {
        return std::make_shared<ArbiterRequest>( pRequestManager, pSectorInfo );
    }
    else
    {
        return ImperialRequestSharedPtr();
    }
}

SectorInfo* ArbiterRequestInfo::FindSector() const
{
    Faction* pIrianiFaction = g_pGame->GetFaction( FactionId::Iriani );
    const SectorInfoVector& controlledSectors = pIrianiFaction->GetControlledSectors();
    SectorInfoVector possibleSectors( controlledSectors.size() );

    std::copy_if( controlledSectors.begin(), controlledSectors.end(), possibleSectors.begin(), []( SectorInfo* pSectorInfo ) { return !pSectorInfo->IsPersonal(); } );
    const size_t numPossibleSectors = possibleSectors.size();
    if ( numPossibleSectors > 0 )
    {
        return ( possibleSectors[ rand() % numPossibleSectors ] );
    }
    else
    {
        return nullptr;
    }
}

} // namespace Hexterminate
