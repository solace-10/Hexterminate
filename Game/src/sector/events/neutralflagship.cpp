// Copyright 2014 Pedro Nunes
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

#include "sector/events/neutralflagship.h"

namespace Hexterminate
{

SectorEventNeutralFlagship::SectorEventNeutralFlagship()
{
    SetName( "NeutralFlagship" );

    AllowForFaction( FactionId::Neutral );
}

void SectorEventNeutralFlagship::OnPlayerEnterSector()
{
    Sector* pCurrentSector = g_pGame->GetCurrentSector();

    g_pGame->AddFleetCommandIntel( "Captain, there is an enemy battleship present in this sector." );

    std::string flagships[ 4 ] = {
        "special_flagship",
        "special_flagship_2",
        "special_flagship_3",
        "special_flagship_4"
    };
    int idx = rand() % 4;

    const ShipInfo* pShipInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetFaction( FactionId::Neutral ), flagships[ idx ] );
    SDL_assert( pShipInfo != nullptr );

    ShipSpawnData spawnData;
    spawnData.m_PositionX = gRand( -1000.0f, 1000.0f );
    spawnData.m_PositionY = gRand( -1000.0f, 1000.0f );

    Ship* pShip = new Ship();
    pShip->SetInitialisationParameters(
        pCurrentSector->GetSectorInfo()->GetFaction(),
        pCurrentSector->GetRegionalFleet(),
        ShipCustomisationData( pShipInfo->GetModuleInfoHexGrid() ),
        spawnData,
        pShipInfo );

    pShip->Initialize();
    pCurrentSector->AddShip( pShip );
}

} // namespace Hexterminate