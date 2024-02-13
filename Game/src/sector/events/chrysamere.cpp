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

#include <math/misc.h>

#include "achievements.h"
#include "blackboard.h"
#include "hexterminate.h"
#include "player.h"
#include "requests/campaigntags.h"
#include "sector/sector.h"
#include "ship/inventory.h"
#include "ship/shipinfo.h"

#include "sector/events/chrysamere.h"

namespace Hexterminate
{

static const std::string sChrysamereDestroyed( "#chrysamere_destroyed" );

SectorEventChrysamere::SectorEventChrysamere()
    : m_pChrysamere( nullptr )
{
    SetName( "Chrysamere" );
    SetMinimumPlayTime( 60 );

    AllowForFaction( FactionId::Neutral );
    AllowForFaction( FactionId::Iriani );
    AllowForFaction( FactionId::Marauders );
    AllowForFaction( FactionId::Ascent );
    AllowForFaction( FactionId::Pirate );
}

bool SectorEventChrysamere::IsAvailableAt( const SectorInfo* pSectorInfo ) const
{
    if ( SectorEvent::IsAvailableAt( pSectorInfo ) == false )
    {
        return false;
    }
    else
    {
        // If the Chrysamere has been previously destroyed then this event can no longer spawn.
        BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
        return !pBlackboard->Exists( sChrysamereDestroyed ) && !pBlackboard->Exists( sFinalChrysamereDestroyed );
    }
}

void SectorEventChrysamere::OnPlayerEnterSector()
{
    Sector* pCurrentSector = g_pGame->GetCurrentSector();

    g_pGame->AddIntel(
        GameCharacter::FleetIntelligence,
        "We've detected the signature of the rogue battleship Chrysamere. It is a ghost ship, millenia old... I recommend we leave it alone." );

    g_pGame->AddIntel(
        GameCharacter::Chrysamere,
        "Rise and fall, the sea changed blood red... ashes and pain, that is all your tortured Empire brings." );

    const ShipInfo* pShipInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetFaction( FactionId::Special ), "special_chrysamere" );
    SDL_assert( pShipInfo != nullptr );

    ShipSpawnData spawnData;
    spawnData.m_PositionX = ( rand() % 2 == 0 ) ? -3000.0f : 3000.0f;
    spawnData.m_PositionY = ( rand() % 2 == 0 ) ? -3000.0f : 3000.0f;

    ShipCustomisationData customisationData;
    customisationData.m_CaptainName = "Elise Delacroix";
    customisationData.m_ShipName = "Chrysamere";
    customisationData.m_pModuleInfoHexGrid = pShipInfo->GetModuleInfoHexGrid();

    m_pChrysamere = new Ship();
    m_pChrysamere->SetInitialisationParameters(
        g_pGame->GetFaction( FactionId::Special ),
        pCurrentSector->GetRegionalFleet(),
        customisationData,
        spawnData,
        pShipInfo );

    m_pChrysamere->Initialize();
    pCurrentSector->AddShip( m_pChrysamere );
}

void SectorEventChrysamere::OnShipDestroyed( Ship* pShip )
{
    if ( pShip == m_pChrysamere )
    {
        // Give the player the Mjolnir artillery and a high regeneration reactor.
        // These items are part of the Chrysamere's ship design.
        Inventory* pInventory = g_pGame->GetPlayer()->GetInventory();
        const int cachedQuantity = ( g_pGame->GetDifficulty() == Difficulty::Normal ) ? 1 : 0;
        pInventory->AddModule( "LegendaryArtillery1", 1, cachedQuantity, true );
        pInventory->AddModule( "ArtifactReactor2", 1, cachedQuantity, true );

        g_pGame->GetBlackboard()->Add( sChrysamereDestroyed );
        g_pGame->GetAchievementsManager()->UnlockAchievement( ACH_THIS_ISNT_EVEN_MY_FINAL_FORM );
    }
}

} // namespace Hexterminate