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

#include "sector/events/orbitaldefenses.h"
#include "hexterminate.h"
#include "sector/sector.h"
#include "ship/shipinfo.h"
#include <math/misc.h>

namespace Hexterminate
{

SectorEventOrbitalDefenses::SectorEventOrbitalDefenses()
{
    SetName( "OrbitalDefenses" );

    AllowForFaction( FactionId::Empire );
    AllowForFaction( FactionId::Neutral );
    AllowForFaction( FactionId::Marauders );
    AllowForFaction( FactionId::Ascent );
    AllowForFaction( FactionId::Iriani );
}

bool SectorEventOrbitalDefenses::IsAvailableAt( const SectorInfo* pSectorInfo ) const
{
    bool allowed = SectorEvent::IsAvailableAt( pSectorInfo );
    if ( allowed == false )
    {
        return false;
    }

    return g_pGame->GetGameMode() != GameMode::InfiniteWar;
}

void SectorEventOrbitalDefenses::OnPlayerEnterSector()
{
    Sector* pCurrentSector = g_pGame->GetCurrentSector();
    if ( pCurrentSector->GetSectorInfo()->GetFaction()->GetFactionId() == FactionId::Empire )
    {
        g_pGame->AddFleetCommandIntel(
            "Captain, allied orbital defenses are present in this sector. "
            "They should add considerable firepower to our fleets." );
    }
    else
    {
        g_pGame->AddFleetCommandIntel(
            "Captain, the enemy has orbital defenses present in this sector. "
            "Orbital defenses are heavily armed and will assist the regional fleet." );
    }

    const ShipInfo* pOrbitalDefenseInfo = g_pGame->GetShipInfoManager()->Get( pCurrentSector->GetSectorInfo()->GetFaction(), "special_orbital_defense" );
    SDL_assert( pOrbitalDefenseInfo != nullptr );

    int defensesToSpawn = rand() % 2 + 1;
    for ( int i = 0; i < defensesToSpawn; ++i )
    {
        ShipSpawnData spawnData;
        spawnData.m_PositionX = 0.0f;
        spawnData.m_PositionY = ( i == 0 ) ? -600.0f : 600.0f;

        ShipCustomisationData customisationData;
        customisationData.m_CaptainName = "";
        customisationData.m_ShipName = "";
        customisationData.m_pModuleInfoHexGrid = pOrbitalDefenseInfo->GetModuleInfoHexGrid();

        Ship* pShip = new Ship();
        pShip->SetInitialisationParameters(
            pCurrentSector->GetSectorInfo()->GetFaction(),
            pCurrentSector->GetRegionalFleet(),
            customisationData,
            spawnData,
            pOrbitalDefenseInfo );

        pShip->Initialize();

        pCurrentSector->AddShip( pShip );
    }
}

void SectorEventOrbitalDefenses::OnShipDestroyed( Ship* pShip )
{
}

} // namespace Hexterminate