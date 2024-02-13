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

#include "components/finalchrysamerecomponent.h"

#include "achievements.h"
#include "blackboard.h"
#include "hexterminate.h"
#include "requests/campaigntags.h"
#include "sector/sector.h"
#include "ship/ship.h"
#include "ship/shipinfo.h"

namespace Hexterminate
{

IMPLEMENT_COMPONENT( FinalChrysamereComponent )

FinalChrysamereComponent::FinalChrysamereComponent()
    : m_pChrysamere( nullptr )
    , m_Timer( 30.0f )
{
}

bool FinalChrysamereComponent::Initialise()
{
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    return ( pBlackboard->Exists( sFinalChrysamereDestroyed ) == false && pBlackboard->Exists( sEnterCradle ) );
}

void FinalChrysamereComponent::Update( float delta )
{
    m_Timer -= delta;
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    if ( m_pChrysamere == nullptr && m_Timer < 0.0f )
    {
        Sector* pCurrentSector = g_pGame->GetCurrentSector();

        const ShipInfo* pShipInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetFaction( FactionId::Special ), "special_chrysamere_2" );
        SDL_assert( pShipInfo != nullptr );

        ShipSpawnData spawnData( 0.0f, -3500.0f );
        ShipCustomisationData customisationData( pShipInfo->GetModuleInfoHexGrid() );

        m_pChrysamere = new Ship();
        m_pChrysamere->SetInitialisationParameters(
            g_pGame->GetFaction( FactionId::Special ),
            pCurrentSector->GetRegionalFleet(),
            customisationData,
            spawnData,
            pShipInfo );

        m_pChrysamere->Initialize();
        pCurrentSector->AddShip( m_pChrysamere );

        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "Captain, massive hyperspace wavefront detected. It's... the Chrysamere again but it has received considerable upgrades. Unknown armour, reactor and weapon signatures. This... is not a battle we will win, Captain. I strongly recommend we jump out." );

        g_pGame->AddIntel( GameCharacter::Chrysamere,
            "The Emperor thinks the galaxy is his. He is wrong. This galaxy... is mine." );
    }
    else if ( m_pChrysamere != nullptr && m_pChrysamere->IsDestroyed() && pBlackboard->Exists( sFinalChrysamereDestroyed ) == false )
    {
        pBlackboard->Add( sFinalChrysamereDestroyed );

        // Unlock both achievements, in case the player has not fought & destroyed the original Chrysamere before.
        g_pGame->GetAchievementsManager()->UnlockAchievement( ACH_THIS_ISNT_EVEN_MY_FINAL_FORM );
        g_pGame->GetAchievementsManager()->UnlockAchievement( ACH_IMMORTAL_NO_MORE );

        g_pGame->AddIntel( GameCharacter::Chrysamere,
            "***static*** You ***static*** no idea ***static*** have done. ***static***" );
    }
}

} // namespace Hexterminate