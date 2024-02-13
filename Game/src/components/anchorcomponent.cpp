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

#include "components/anchorcomponent.h"

#include "blackboard.h"
#include "hexterminate.h"
#include "requests/campaigntags.h"
#include "sector/sector.h"
#include "ship/ship.h"
#include "ship/shipinfo.h"

namespace Hexterminate
{

IMPLEMENT_COMPONENT( AnchorComponent )

AnchorComponent::AnchorComponent()
    : m_pAnchor( nullptr )
{
}

bool AnchorComponent::Initialise()
{
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    if ( pBlackboard->Exists( sAnchorDestroyed ) == false )
    {
        SpawnAnchor();
        SpawnAnchorDefenses();

        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "This is the where the seat of power of the Ascent is - they call it the Anchor." );

        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "Destroying it will wipe out their leadership and we should be able to find the location of Orion's Sword afterwards." );
    }

    return true;
}

void AnchorComponent::Update( float delta )
{
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    if ( pBlackboard->Exists( sAnchorDestroyed ) == false && m_pAnchor != nullptr && m_pAnchor->IsDestroyed() )
    {
        pBlackboard->Add( sAnchorDestroyed );
        pBlackboard->Add( sConquerSolarisSecundusCompleted );
    }
}

void AnchorComponent::SpawnAnchor()
{
    Sector* pCurrentSector = g_pGame->GetCurrentSector();

    const ShipInfo* pShipInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetFaction( FactionId::Ascent ), "special_anchor" );
    SDL_assert( pShipInfo != nullptr );

    ShipSpawnData spawnData( 0.0f, -3000.0f );
    ShipCustomisationData customisationData( pShipInfo->GetModuleInfoHexGrid() );

    m_pAnchor = new Ship();
    m_pAnchor->SetInitialisationParameters(
        g_pGame->GetFaction( FactionId::Ascent ),
        pCurrentSector->GetRegionalFleet(),
        customisationData,
        spawnData,
        pShipInfo );

    m_pAnchor->Initialize();
    pCurrentSector->AddShip( m_pAnchor );
}

void AnchorComponent::SpawnAnchorDefenses()
{
    Sector* pCurrentSector = g_pGame->GetCurrentSector();

    const ShipInfo* pShipInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetFaction( FactionId::Ascent ), "special_anchor_defense" );
    SDL_assert( pShipInfo != nullptr );

    ShipCustomisationData customisationData( pShipInfo->GetModuleInfoHexGrid() );
    ShipSpawnData spawnData[ 4 ] = {
        { 0.0f, -3500.0f },
        { 0.0f, -2500.0f },
        { -500, -3000.0f },
        { 500, -3000.0f }
    };

    for ( int i = 0; i < 4; ++i )
    {
        Ship* pAnchorDefense = new Ship();
        pAnchorDefense->SetInitialisationParameters(
            g_pGame->GetFaction( FactionId::Ascent ),
            pCurrentSector->GetRegionalFleet(),
            customisationData,
            spawnData[ i ],
            pShipInfo );

        pAnchorDefense->Initialize();
        pCurrentSector->AddShip( pAnchorDefense );
    }
}

} // namespace Hexterminate