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

#include "components/homeworldcomponent.h"

#include <fstream>
#include <sstream>

#include <genesis.h>

#include "fleet/fleet.h"
#include "hexterminate.h"
#include "sector/sector.h"
#include "ship/shipinfo.h"

namespace Hexterminate
{

IMPLEMENT_COMPONENT( HomeworldComponent )

bool HomeworldComponent::Initialise()
{

    Sector* pSector = g_pGame->GetCurrentSector();
    if ( pSector->GetSectorInfo()->IsHomeworld() == false )
    {
        return false;
    }

    FactionId factionId = pSector->GetSectorInfo()->GetFaction()->GetFactionId();
    StringVector shipNames;

    if ( factionId == FactionId::Pirate )
    {
        shipNames = {
            "pirate_gunship_3",
            "pirate_gunship_4",
            "pirate_battlecruiser_2",
            "special_flagship",
            "pirate_battlecruiser_2",
            "pirate_gunship_4",
            "pirate_gunship_3"
        };
    }
    else if ( factionId == FactionId::Marauders )
    {
        shipNames = {
            "marauder_capital_2",
            "marauder_capital_2",
            "marauder_capital_2",
            "special_flagship",
            "marauder_capital_2",
            "marauder_capital_2",
            "marauder_capital_2"
        };
    }
    else if ( factionId == FactionId::Ascent )
    {
        shipNames = {
            "capital3",
            "capital3",
            "special_flagship",
            "special_flagship",
            "capital3",
            "capital3"
        };
    }
    else if ( factionId == FactionId::Iriani )
    {
        shipNames = {
            "special_arbiter",
            "special_arbiter",
            "special_flagship",
            "special_flagship",
            "special_arbiter",
            "special_arbiter"
        };
    }
    else if ( factionId == FactionId::Hegemon )
    {
        shipNames = {
            "hegemon_capital_3",
            "hegemon_capital_3",
            "special_flagship",
            "hegemon_capital_3",
            "hegemon_capital_3"
        };
    }

    if ( shipNames.empty() == false )
    {
        SpawnReinforcements( shipNames );
    }

    return true;
}

void HomeworldComponent::SpawnReinforcements( const StringVector& shipNames )
{
    Sector* pSector = g_pGame->GetCurrentSector();
    Faction* pFaction = pSector->GetSectorInfo()->GetFaction();
    FleetSharedPtr pTemporaryFleet = std::make_shared<Fleet>();
    pTemporaryFleet->Initialise( pFaction, pSector->GetSectorInfo() );

    for ( const std::string& shipName : shipNames )
    {
        const ShipInfo* pShipInfo = g_pGame->GetShipInfoManager()->Get( pFaction, shipName );
        SDL_assert( pShipInfo != nullptr );
        pTemporaryFleet->AddShip( pShipInfo );
    }

    pSector->Reinforce( pTemporaryFleet );
}

} // namespace Hexterminate