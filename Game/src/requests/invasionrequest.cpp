// Copyright 2016 Pedro Nunes
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

#include <cassert>
#include <sstream>

#include "fleet/fleet.h"
#include "hexterminate.h"
#include "player.h"
#include "requests/campaignrequest.h"
#include "requests/campaigntags.h"
#include "requests/invasionrequest.h"
#include "requests/invasionrequestinfo.h"
#include "sector/sector.h"
#include "ship/shipinfo.h"
#include "stringaux.h"
#include <genesis.h>

namespace Hexterminate
{

InvasionRequest::InvasionRequest( RequestManager* pRequestManager, SectorInfo* pSectorInfo, FactionId invadingFaction )
    : ImperialRequest( pRequestManager )
    , m_pSectorInfo( pSectorInfo )
    , m_Reward( 0 )
    , m_FailureTimer( sInvasionRequestTimer )
    , m_InvadingFaction( invadingFaction )
{
    SDL_assert( pSectorInfo != nullptr );
}

void InvasionRequest::OnBegin()
{
    ImperialRequest::OnBegin();

    m_Reward = m_pSectorInfo->GetConquestReward();

    std::stringstream ss;
    ss << "Sector being invaded: +" << m_Reward << " influence";

    m_pGoal = std::make_shared<RequestGoal>( m_pSectorInfo, ss.str(), Genesis::Color( 0.0f, 0.0f, 0.6f, 0.6f ) );
    AddGoal( m_pGoal );

#ifdef _DEBUG
    int x, y;
    m_pSectorInfo->GetCoordinates( x, y );
    Genesis::FrameWork::GetLogger()->LogInfo( "Starting InvasionRequest on sector %d / %d by faction %s", x, y, ToString( m_InvadingFaction ).c_str() );
#endif
}

void InvasionRequest::Update( float delta )
{
    ImperialRequest::Update( delta );

    // If the player was in this sector and there are no hostile contesting fleets, then the request is completed successfully
    Sector* pCurrentSector = g_pGame->GetCurrentSector();
    if ( pCurrentSector != nullptr && pCurrentSector->GetSectorInfo() == m_pSectorInfo )
    {
        bool hostilesPresent = false;
        Faction* pPlayerFaction = g_pGame->GetFaction( FactionId::Player );
        const ShipList& shipList = pCurrentSector->GetShipList();
        for ( Ship* pShip : shipList )
        {
            if ( pShip->IsDestroyed() == false && Faction::sIsEnemyOf( pShip->GetFaction(), pPlayerFaction ) )
            {
                hostilesPresent = true;
                break;
            }
        }

        if ( hostilesPresent == false )
        {
            OnSuccess();
        }
    }
    // But if the sector no longer belongs to the Empire, then the request fails
    else if ( m_pSectorInfo->GetFaction() != g_pGame->GetFaction( FactionId::Empire ) )
    {
        OnFailure();
    }
    else
    {
        m_FailureTimer -= delta;

        if ( m_FailureTimer <= 0.0f )
        {
            m_pSectorInfo->SetFaction( g_pGame->GetFaction( m_InvadingFaction ), false, false );
        }
    }
}

void InvasionRequest::OnSuccess()
{
    ImperialRequest::OnSuccess();

    std::stringstream ss;
    ss << "The sector has been successfully defended. Our influence with Imperial HQ has increased by " << m_Reward << ".";
    g_pGame->AddIntel( GameCharacter::FleetIntelligence, ss.str() );

    static std::string invasionTag( InvasionRequestInfo::GetBlackboardTag( m_InvadingFaction ) );
    int invasionId = g_pGame->GetBlackboard()->Get( invasionTag );
    g_pGame->GetBlackboard()->Add( invasionTag, invasionId - 1 );

    RemoveGoal( m_pGoal );
    m_pGoal.reset();
}

void InvasionRequest::OnFailure()
{
    ImperialRequest::OnFailure();

    RemoveGoal( m_pGoal );
    m_pGoal.reset();
}

void InvasionRequest::OnPlayerEnterSector()
{
    Faction* pInvadingFaction = g_pGame->GetFaction( GetInvadingFaction() );
    FleetSharedPtr pInvasionFleet = std::make_shared<Fleet>();
    pInvasionFleet->Initialise( pInvadingFaction, g_pGame->GetCurrentSector()->GetSectorInfo() );

    ShipInfoList shipsToSpawn = GetShipsToSpawn();
    for ( const ShipInfo* pShipInfo : shipsToSpawn )
    {
        pInvasionFleet->AddShip( pShipInfo );
    }

    g_pGame->GetCurrentSector()->Reinforce( pInvasionFleet );
}

int InvasionRequest::GetConquestReward( const SectorInfo* pSectorInfo ) const
{
    return ( pSectorInfo == m_pSectorInfo ) ? sInvasionRequestReward : 0;
}

ShipInfoList InvasionRequest::GetShipsToSpawn() const
{
    static std::string invasionTag( InvasionRequestInfo::GetBlackboardTag( m_InvadingFaction ) );
    int invasionId = g_pGame->GetBlackboard()->Get( invasionTag );

    std::string factionName = ToString( GetInvadingFaction() );
    std::stringstream filename;
    filename << "data/xml/invasions/" << ToString( GetInvadingFaction() ) << "_" << invasionId << ".inv";

    ShipInfoList shipsToSpawn;
    Faction* pInvadingFaction = g_pGame->GetFaction( GetInvadingFaction() );
    std::ifstream fs( filename.str() );
    if ( fs.is_open() )
    {
        std::string shipName;
        ShipInfoManager* pShipInfoManager = g_pGame->GetShipInfoManager();
        while ( fs.good() )
        {
            fs >> shipName;

            const ShipInfo* pShipInfo = pShipInfoManager->Get( pInvadingFaction, shipName );
            if ( pShipInfo == nullptr )
            {
                Genesis::FrameWork::GetLogger()->LogWarning( "Couldn't load ship '%s' for faction '%s'", shipName.c_str(), factionName.c_str() );
            }
            else
            {
                shipsToSpawn.push_back( pShipInfo );
            }
        }
        fs.close();
    }
    else
    {
        Genesis::FrameWork::GetLogger()->LogWarning( "Could not open invasion file '%s'", filename.str().c_str() );
    }

    return shipsToSpawn;
}

} // namespace Hexterminate
