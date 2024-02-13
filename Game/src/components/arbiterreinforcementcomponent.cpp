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

#include <math/misc.h>

#include "components/arbiterreinforcementcomponent.h"

#include "blackboard.h"
#include "hexterminate.h"
#include "requests/campaigntags.h"
#include "sector/sector.h"
#include "ship/ship.h"
#include "ship/shipinfo.h"

namespace Hexterminate
{

IMPLEMENT_COMPONENT( ArbiterReinforcementComponent )

bool ArbiterReinforcementComponent::Initialise()
{
    SpawnArbiters();
    return true;
}

void ArbiterReinforcementComponent::SpawnArbiters()
{
    Sector* pCurrentSector = g_pGame->GetCurrentSector();

    const ShipInfo* pShipInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetFaction( FactionId::Iriani ), "special_arbiter" );
    SDL_assert( pShipInfo != nullptr );

    ShipSpawnData spawnData[ 4 ] = {
        { 0.0f, -3000.0f },
        { 0.0f, 3000.0f },
        { 3000.0f, 0.0f },
        { -3000.0f, 0.0f }
    };

    const int arbitersDestroyed = g_pGame->GetBlackboard()->Get( sArbitersDestroyed );
    const int arbitersAvailable = gClamp<int>( 4 - arbitersDestroyed, 0, 4 );
    for ( int i = 0; i < arbitersAvailable; ++i )
    {
        ShipCustomisationData customisationData( pShipInfo->GetModuleInfoHexGrid() );
        Ship* pShip = new Ship();
        pShip->SetInitialisationParameters(
            g_pGame->GetFaction( FactionId::Iriani ),
            pCurrentSector->GetRegionalFleet(),
            customisationData,
            spawnData[ i ],
            pShipInfo );

        pShip->Initialize();
        pCurrentSector->AddShip( pShip );
    }
}

} // namespace Hexterminate