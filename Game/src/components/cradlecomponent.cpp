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

#include "components/cradlecomponent.h"

#include "blackboard.h"
#include "hexterminate.h"
#include "requests/campaigntags.h"
#include "sector/sector.h"
#include "ship/ship.h"
#include "ship/shipinfo.h"

namespace Hexterminate
{

IMPLEMENT_COMPONENT( CradleComponent )

CradleComponent::CradleComponent()
    : m_pCradle( nullptr )
{
}

bool CradleComponent::Initialise()
{
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    if ( pBlackboard->Exists( sEnterCradle ) && pBlackboard->Exists( sEnterCradleCompleted ) == false )
    {
        Sector* pCurrentSector = g_pGame->GetCurrentSector();

        const ShipInfo* pShipInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetFaction( FactionId::Special ), "special_cradle" );
        SDL_assert( pShipInfo != nullptr );

        ShipSpawnData spawnData( 0.0f, -3000.0f );
        ShipCustomisationData customisationData( pShipInfo->GetModuleInfoHexGrid() );

        m_pCradle = new Ship();
        m_pCradle->SetInitialisationParameters(
            g_pGame->GetFaction( FactionId::Special ),
            pCurrentSector->GetRegionalFleet(),
            customisationData,
            spawnData,
            pShipInfo );

        m_pCradle->Initialize();
        pCurrentSector->AddShip( m_pCradle );
    }

    return true;
}

void CradleComponent::Update( float delta )
{
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    if ( m_pCradle != nullptr && m_pCradle->IsDestroyed() && pBlackboard->Exists( sPlayerHasOrionsSword ) == false )
    {
        pBlackboard->Add( sEnterCradleCompleted );
        pBlackboard->Add( sPlayerHasOrionsSword );

        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "We've retrieved the Orion's sword. We'll connect it to our hyperspace core momentarily, which will allow us to begin the invasion of the Iriani space." );

        // Disable the hyperspace inhibitors for all Iriani sectors
        for ( auto& pSectorInfo : g_pGame->GetFaction( FactionId::Iriani )->GetControlledSectors() )
        {
            pSectorInfo->SetHyperspaceInhibitor( false );
        }
    }
}

} // namespace Hexterminate