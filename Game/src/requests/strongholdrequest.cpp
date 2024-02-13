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

#include "requests/strongholdrequest.h"

#include <cassert>
#include <sstream>

#include "hexterminate.h"
#include "player.h"
#include "sector/sector.h"
#include "ship/shipinfo.h"
#include <genesis.h>

namespace Hexterminate
{

StrongholdRequest::StrongholdRequest( RequestManager* pRequestManager, SectorInfo* pSectorInfo )
    : ImperialRequest( pRequestManager )
    , m_pSectorInfo( pSectorInfo )
    , m_Reward( 0 )
{
    SDL_assert( pSectorInfo != nullptr );
}

void StrongholdRequest::OnBegin()
{
    ImperialRequest::OnBegin();

    m_Reward = m_pSectorInfo->GetConquestReward();

    std::stringstream ss;
    ss << "Destroy stronghold: +" << m_Reward << " influence";

    m_pGoal = std::make_shared<RequestGoal>( m_pSectorInfo, ss.str() );
    AddGoal( m_pGoal );

#ifdef _DEBUG
    int x, y;
    m_pSectorInfo->GetCoordinates( x, y );
    Genesis::FrameWork::GetLogger()->LogInfo( "Starting StrongholdRequest on sector %d / %d", x, y );
#endif

    // The threat for the stronghold is artificially boosted since it is actually a pretty nasty structure,
    // specially early in the game.
    const ShipInfo* pStrongholdInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetFaction( FactionId::Neutral ), "special_stronghold" );
    SDL_assert( pStrongholdInfo != nullptr );
    SetThreatScore( (unsigned int)( (float)pStrongholdInfo->GetPoints() * 1.5f ) );
}

void StrongholdRequest::Update( float delta )
{
    ImperialRequest::Update( delta );

    // If the player was in this sector and it is now ours, then the request is completed successfully
    Sector* pCurrentSector = g_pGame->GetCurrentSector();
    if ( pCurrentSector != nullptr && pCurrentSector->GetSectorInfo() == m_pSectorInfo && pCurrentSector->IsPlayerVictorious() )
    {
        OnSuccess();
    }
    // But if the sector now belongs to the Empire without the player interfering, then the request fails
    else if ( m_pSectorInfo->GetFaction() == g_pGame->GetFaction( FactionId::Empire ) )
    {
        OnFailure();
    }
}

void StrongholdRequest::OnSuccess()
{
    ImperialRequest::OnSuccess();

    std::stringstream ss;
    ss << "The enemy stronghold has been destroyed and no further hostile signatures have been detected. Our influence with Imperial HQ has increased by " << m_Reward << ".";
    g_pGame->AddIntel( GameCharacter::FleetIntelligence, ss.str() );

    RemoveGoal( m_pGoal );
    m_pGoal.reset();
}

void StrongholdRequest::OnFailure()
{
    ImperialRequest::OnFailure();

    RemoveGoal( m_pGoal );
    m_pGoal.reset();
}

void StrongholdRequest::OnPlayerEnterSector()
{
    const ShipInfo* pStrongholdInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetFaction( FactionId::Neutral ), "special_stronghold" );
    SDL_assert_release( pStrongholdInfo != nullptr );

    ShipSpawnData spawnData;
    spawnData.m_PositionX = 0.0f;
    spawnData.m_PositionY = 0.0f;

    ShipCustomisationData customisationData;
    customisationData.m_CaptainName = "";
    customisationData.m_ShipName = "";
    customisationData.m_pModuleInfoHexGrid = pStrongholdInfo->GetModuleInfoHexGrid();

    Sector* pCurrentSector = g_pGame->GetCurrentSector();
    Ship* pStronghold = new Ship();
    pStronghold->SetInitialisationParameters(
        pCurrentSector->GetSectorInfo()->GetFaction(),
        pCurrentSector->GetRegionalFleet(),
        customisationData,
        spawnData,
        pStrongholdInfo );

    pStronghold->Initialize();

    pCurrentSector->AddShip( pStronghold );

    g_pGame->AddIntel(
        GameCharacter::FleetIntelligence,
        "The stronghold has powerful void shields - it can withstand a siege for a while but the shields themselves do not regenerate." );
}

int StrongholdRequest::GetConquestReward( const SectorInfo* pSectorInfo ) const
{
    return ( pSectorInfo == m_pSectorInfo ) ? StrongholdRequestReward : 0;
}

} // namespace Hexterminate
