// Copyright 2018 Pedro Nunes
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

#include "requests/arbiterrequest.h"

#include <cassert>
#include <sstream>

#include <genesis.h>

#include "blackboard.h"
#include "fleet/fleet.h"
#include "hexterminate.h"
#include "player.h"
#include "requests/campaigntags.h"
#include "sector/sector.h"
#include "ship/shipinfo.h"

namespace Hexterminate
{

ArbiterRequest::ArbiterRequest( RequestManager* pRequestManager, SectorInfo* pSectorInfo )
    : ImperialRequest( pRequestManager )
    , m_pSectorInfo( pSectorInfo )
    , m_Reward( 0 )
    , m_pArbiter( nullptr )
{
    SDL_assert( pSectorInfo != nullptr );
}

void ArbiterRequest::OnBegin()
{
    ImperialRequest::OnBegin();

    m_Reward = m_pSectorInfo->GetConquestReward();

    std::stringstream ss;
    ss << "Destroy Arbiter: +" << m_Reward << " influence";

    m_pGoal = std::make_shared<RequestGoal>( m_pSectorInfo, ss.str() );
    AddGoal( m_pGoal );
}

void ArbiterRequest::OnPlayerEnterSector()
{
    const ShipInfo* pArbiterShipInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetFaction( FactionId::Iriani ), "special_arbiter" );
    SDL_assert( pArbiterShipInfo != nullptr );

    const ShipInfo* pMaidenShipInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetFaction( FactionId::Iriani ), "special_maiden" );
    SDL_assert( pMaidenShipInfo != nullptr );

    m_pTemporaryFleet = std::make_shared<Fleet>();
    m_pTemporaryFleet->Initialise( g_pGame->GetFaction( FactionId::Iriani ), m_pSectorInfo );

    m_pTemporaryFleet->AddShip( pMaidenShipInfo );
    m_pTemporaryFleet->AddShip( pMaidenShipInfo );
    m_pTemporaryFleet->AddShip( pArbiterShipInfo );
    m_pTemporaryFleet->AddShip( pMaidenShipInfo );
    m_pTemporaryFleet->AddShip( pMaidenShipInfo );

    ShipVector spawnedShips;
    if ( g_pGame->GetCurrentSector()->Reinforce( m_pTemporaryFleet, &spawnedShips ) )
    {
        for ( Ship* pShip : spawnedShips )
        {
            if ( pShip->GetShipInfo() == pArbiterShipInfo )
            {
                m_pArbiter = pShip;
                break;
            }
        }

        BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
        if ( pBlackboard->Exists( sArbitersIntro ) == false )
        {
            g_pGame->AddIntel(
                GameCharacter::AeliseGloriam,
                "I see you have found one of our Arbiters. Let us see how you fare against one of the guardians of the Iriani." );

            pBlackboard->Add( sArbitersIntro );
        }
    }
    else
    {
        m_pTemporaryFleet = nullptr;
        OnFailure();
    }
}

void ArbiterRequest::Update( float delta )
{
    ImperialRequest::Update( delta );

    // The request is completed successfully if the player destroys the Arbiter.
    Sector* pCurrentSector = g_pGame->GetCurrentSector();
    if ( pCurrentSector != nullptr && pCurrentSector->GetSectorInfo() == m_pSectorInfo )
    {
        if ( m_pArbiter != nullptr && m_pArbiter->IsDestroyed() )
        {
            BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
            int arbitersDestroyed = pBlackboard->Get( sArbitersDestroyed ) + 1;
            pBlackboard->Add( sArbitersDestroyed, arbitersDestroyed );
            OnSuccess();
        }
    }
    // But if the sector no longer belongs to the Iriani, then the request fails and will be re-spawned elsewhere.
    else if ( m_pSectorInfo->GetFaction() != g_pGame->GetFaction( FactionId::Iriani ) )
    {
        OnFailure();
    }
}

void ArbiterRequest::OnSuccess()
{
    ImperialRequest::OnSuccess();

    std::stringstream ss;
    ss << "The Arbiter has been destroyed - one less reinforcement for Iriani Prime. Our influence with Imperial HQ has increased by " << m_Reward << ".";
    g_pGame->AddIntel( GameCharacter::FleetIntelligence, ss.str() );

    RemoveGoal( m_pGoal );
    m_pGoal.reset();
}

void ArbiterRequest::OnFailure()
{
    ImperialRequest::OnFailure();

    RemoveGoal( m_pGoal );
    m_pGoal.reset();
}

int ArbiterRequest::GetConquestReward( const SectorInfo* pSectorInfo ) const
{
    return ( pSectorInfo == m_pSectorInfo ) ? sArbiterRequestReward : 0;
}

} // namespace Hexterminate
