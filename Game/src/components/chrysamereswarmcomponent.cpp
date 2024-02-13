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

#include "components/chrysamereswarmcomponent.h"
#include "blackboard.h"
#include "fleet/fleet.h"
#include "hexterminate.h"
#include "requests/campaigntags.h"
#include "sector/sector.h"
#include "ship/shipinfo.h"

namespace Hexterminate
{

IMPLEMENT_COMPONENT( ChrysamereSwarmComponent )

ChrysamereSwarmComponent::ChrysamereSwarmComponent()
    : m_Wave( 0u )
    , m_Timer( 5.0f )
{
}

bool ChrysamereSwarmComponent::Initialise()
{
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    if ( pBlackboard->Exists( sFinalChrysamereDestroyed ) == false )
    {
        g_pGame->AddIntel( GameCharacter::Chrysamere,
            "You tread beyond where you're permitted to." );

        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "Multiple hyperspace wavefronts detected. This system appears to be receiving reinforcements from an automated shipyard outside of realspace. We need to be fast, Captain, or they will grind us down." );

        g_pGame->AddIntel( GameCharacter::Chrysamere,
            "How many will it take to destroy you? Five? Ten? A hundred?" );

        return true;
    }
    else
    {
        return false;
    }
}

void ChrysamereSwarmComponent::Update( float delta )
{
    m_Timer -= delta;
    if ( m_Timer <= 0.0f )
    {
        Sector* pCurrentSector = g_pGame->GetCurrentSector();

        m_Timer = 45.0f;
        m_Wave++;

        FleetSharedPtr temporaryFleet = std::make_shared<Fleet>();
        temporaryFleet->Initialise( g_pGame->GetFaction( FactionId::Special ), pCurrentSector->GetSectorInfo() );

        const ShipInfo* pShipInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetFaction( FactionId::Special ), "special_chrysamere_drone" );

        const int numShips = 2 + m_Wave * 2;
        for ( int i = 0; i < numShips; ++i )
        {
            temporaryFleet->AddShip( pShipInfo );
        }

        if ( pCurrentSector->Reinforce( temporaryFleet ) )
        {
            m_TemporaryFleets.push_back( temporaryFleet );
        }
    }
}

} // namespace Hexterminate