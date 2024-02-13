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

#include "hexterminate.h"
#include "sector/sector.h"
#include "ship/shipinfo.h"
#include <math/misc.h>

#include "sector/events/corsairfleet.h"

namespace Hexterminate
{

SectorEventCorsairFleet::SectorEventCorsairFleet()
{
    SetName( "CorsairFleet" );
    SetMinimumPlayTime( 30 );

    AllowForFaction( FactionId::Neutral );
    AllowForFaction( FactionId::Marauders );
    AllowForFaction( FactionId::Ascent );
    AllowForFaction( FactionId::Pirate );
}

void SectorEventCorsairFleet::OnPlayerEnterSector()
{
    Sector* pCurrentSector = g_pGame->GetCurrentSector();

    g_pGame->AddFleetCommandIntel( "Captain, we've detected a Corsair wing at long range. They're a small group of pirates flying heavily shielded gunships." );
    g_pGame->AddFleetCommandIntel( "These ships have covariant shield arrays, essentially generators overtuned past safety limits." );

    const ShipInfo* pShipInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetFaction( FactionId::Pirate ), "special_corsair" );
    SDL_assert( pShipInfo != nullptr );

    ShipCustomisationData customisationData;
    customisationData.m_CaptainName = "";
    customisationData.m_ShipName = "";
    customisationData.m_pModuleInfoHexGrid = pShipInfo->GetModuleInfoHexGrid();

    const int numShips = 3;
    const float spawnX = ( rand() % 2 == 0 ) ? -4000.0f : 4000.0f;
    const float spawnY = ( rand() % 2 == 0 ) ? -4000.0f : 4000.0f;
    const float xOffset[ numShips ] = { -150.0f, 0.0f, 150.0f };
    const float yOffset[ numShips ] = { 100.0f, -100.0f, 100.0f };

    for ( int i = 0; i < numShips; ++i )
    {
        ShipSpawnData spawnData;
        spawnData.m_PositionX = spawnX + xOffset[ i ];
        spawnData.m_PositionY = spawnY + yOffset[ i ];

        Ship* pShip = new Ship();
        pShip->SetInitialisationParameters(
            g_pGame->GetFaction( FactionId::Pirate ),
            pCurrentSector->GetRegionalFleet(),
            customisationData,
            spawnData,
            pShipInfo );

        pShip->Initialize();
        pCurrentSector->AddShip( pShip );
    }
}

} // namespace Hexterminate