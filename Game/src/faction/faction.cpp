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

#include "faction/faction.h"
#include "achievements.h"
#include "blackboard.h"
#include "faction/empirefaction.h"
#include "fleet/fleet.h"
#include "fleet/fleetbehaviour.h"
#include "globals.h"
#include "hexterminate.h"
#include "misc/randomshuffle.h"
#include "requests/campaigntags.h"
#include "requests/expandrequest.h"
#include "requests/requestmanager.h"
#include "sector/fogofwar.h"
#include "sector/galaxy.h"
#include "ship/shipinfo.h"
#include "stringaux.h"
#include "xmlaux.h"
#include <algorithm>
#include <genesis.h>
#include <logger.h>
#include <math/misc.h>
#include <xml.h>

namespace Hexterminate
{

///////////////////////////////////////////////////////////
// Faction
///////////////////////////////////////////////////////////

Faction::Faction( const FactionInfo& info, FactionId id )
    : m_Info( info )
    , m_FactionId( id )
    , m_Turn( 0 )
    , m_NextTurnTimer( 0.0f )
    , m_pHomeworld( nullptr )
    , m_StartedWithHomeworld( false )
    , m_InitialPresence( FactionPresence::Standard )
{
}

void Faction::Update( float delta )
{
    if ( m_NextTurnTimer > 0.0f )
    {
        m_NextTurnTimer -= delta;
    }
    else
    {
        m_NextTurnTimer = TurnDuration;
        m_Turn++;
        ProcessTurn();
    }

    // Update all fleets
    for ( auto& pFleet : m_Fleets )
    {
        pFleet->Update( delta );
    }
}

void Faction::PostUpdate()
{
    for ( auto& pSectorInfo : m_ControlledSectorsToAdd )
    {
        m_ControlledSectors.push_back( pSectorInfo );
    }
    m_ControlledSectorsToAdd.clear();

    for ( auto& pSectorInfoToRemove : m_ControlledSectorsToRemove )
    {
        const size_t numControlledSectors = m_ControlledSectors.size();
        for ( size_t i = 0; i < numControlledSectors; ++i )
        {
            if ( m_ControlledSectors[ i ] == pSectorInfoToRemove )
            {
                m_ControlledSectors.erase( m_ControlledSectors.begin() + i );
                break;
            }
        }
    }
    m_ControlledSectorsToRemove.clear();

    for ( auto it = m_FleetsToDestroy.cbegin(); it != m_FleetsToDestroy.cend(); ++it )
    {
        for ( auto it2 = m_Fleets.begin(); it2 != m_Fleets.end(); ++it2 )
        {
            if ( *it2 == *it )
            {
                m_Fleets.erase( it2 );
                break;
            }
        }
    }
    m_FleetsToDestroy.clear();
}

void Faction::ProcessTurn()
{
    // Figure out how many shipyards this faction should have, based on our SectorToShipyard ratio
    // and whether this faction is collapsing or not.
    // If we need more, build them in our controlled sectors.
    int numSectors = static_cast<int>( m_ControlledSectors.size() );
    float wantedShipyards = IsCollapsing() ? 0.0f : ( numSectors * m_Info.m_SectorToShipyardRatio );
    SectorInfoVector shipyards;
    for ( auto& pSector : m_ControlledSectors )
    {
        pSector->ProcessTurn();

        if ( pSector->HasShipyard() )
        {
            shipyards.push_back( pSector );
        }
    }

    SectorInfoVector controlled_sectors = m_ControlledSectors;
    while ( wantedShipyards > (float)shipyards.size() )
    {
        RandomShuffle::Shuffle( controlled_sectors.begin(), controlled_sectors.end() );
        SectorInfo* pSectorInfo = controlled_sectors.back();
        if ( pSectorInfo->HasShipyard() == false )
        {
            pSectorInfo->SetShipyard( true );
            shipyards.push_back( pSectorInfo );
            controlled_sectors.pop_back();
        }
    }

    BuildFleets( shipyards );
    ProcessTurnFleets();
    ProcessCollapse();
}

void Faction::ForceNextTurn()
{
    m_NextTurnTimer = 0.0f;
}

void Faction::BuildFleets( const SectorInfoVector& shipyards )
{
    // Do we have enough fleets? We should have one fleet per shipyard. If not, start building a fleet.
    int numFleets = static_cast<int>( m_Fleets.size() );
    int numFleetsInConstruction = static_cast<int>( m_FleetsInConstruction.size() );
    while ( numFleets + numFleetsInConstruction < (int)shipyards.size() )
    {
        m_FleetsInConstruction.push_front( FleetBuildTime );
        numFleetsInConstruction++;
    }

    // Tick down the construction queue and if a fleet is ready, spawn it at a random shipyard.
    TurnQueue::iterator it = m_FleetsInConstruction.begin();
    while ( it != m_FleetsInConstruction.end() )
    {
        ( *it )--;
        if ( *it == 0 )
        {
            it = m_FleetsInConstruction.erase( it );
            if ( shipyards.empty() == false )
            {
                BuildFleet( shipyards[ rand() % shipyards.size() ] );
            }
        }
        else
        {
            it++;
        }
    }
}

void Faction::ProcessTurnFleets()
{
    bool isCollapsing = IsCollapsing();
    for ( auto& pFleet : m_Fleets )
    {
        if ( pFleet->HasFlagship() && isCollapsing )
        {
            const ShipInfo* pFlagshipInfo = g_pGame->GetShipInfoManager()->Get( pFleet->GetFaction(), "special_flagship" );
            pFleet->RemoveShip( pFlagshipInfo );
        }

        pFleet->ProcessTurn();
    }
}

// If a Faction is collapsing then for every turn they have a chance of losing control of sectors.
void Faction::ProcessCollapse()
{
    if ( IsCollapsing() )
    {
        for ( auto& pSectorInfo : m_ControlledSectors )
        {
            if ( gRand() <= 0.05f )
            {
                pSectorInfo->SetFaction( g_pGame->GetFaction( FactionId::Neutral ), false, false );
            }
        }
    }

    if ( m_FactionId == FactionId::Ascent && m_ControlledSectors.empty() && g_pGame->GetFaction( m_FactionId )->GetInitialPresence() != FactionPresence::None )
    {
        g_pGame->GetAchievementsManager()->UnlockAchievement( ACH_NOT_EVEN_BEACONS );
    }

    if ( g_pGame->GetGameMode() == GameMode::InfiniteWar && m_FactionId == FactionId::Hegemon && m_ControlledSectors.empty() && g_pGame->GetFaction( m_FactionId )->GetInitialPresence() != FactionPresence::None )
    {
        g_pGame->GetAchievementsManager()->UnlockAchievement( ACH_HEGEGONE );

        if ( g_pGame->GetDifficulty() == Difficulty::Hardcore )
        {
            g_pGame->GetAchievementsManager()->UnlockAchievement( ACH_FINITE_WAR );
        }
    }
}

void Faction::AddControlledSector( SectorInfo* pSector, bool immediate, bool takenByPlayer )
{
    if ( immediate )
    {
        m_ControlledSectors.push_back( pSector );
    }
    else
    {
        m_ControlledSectorsToAdd.push_back( pSector );
    }
}

void Faction::RemoveControlledSector( SectorInfo* pSectorToRemove, bool immediate /* = false */ )
{
    if ( immediate )
    {
        for ( SectorInfoVector::iterator it = m_ControlledSectors.begin(), itEnd = m_ControlledSectors.end(); it != itEnd; ++it )
        {
            if ( *it == pSectorToRemove )
            {
                m_ControlledSectors.erase( it );
                break;
            }
        }
    }
    else
    {
        m_ControlledSectorsToRemove.push_back( pSectorToRemove );
    }
}

FleetWeakPtr Faction::BuildFleet( SectorInfo* pInitialSector )
{
    // A faction that can deploy flagships can only have one flagship in play at any given time.
    const bool isFlagshipSpawned = IsFlagshipSpawned();

    // Once this faction's flagship has been destroyed, no others will take its place.
    const bool isFlagshipDestroyed = IsFlagshipDestroyed();

    const bool isFlagshipFleet = HasFlagship() && !isFlagshipSpawned && !isFlagshipDestroyed && m_Info.m_SpawnsFlagshipInGalaxy;

    FleetSharedPtr pFleet = std::make_shared<Fleet>();
    pFleet->Initialise( this, pInitialSector );

    if ( GetFactionId() != FactionId::Player )
    {
        if ( isFlagshipFleet )
        {
            pFleet->GenerateFlagshipFleet();
        }
        else
        {
            pFleet->GenerateProceduralFleet( m_Info.m_BaseFleetPoints );
        }
    }

    m_Fleets.push_back( pFleet );
    return pFleet;
}

bool Faction::IsFlagshipSpawned() const
{
    if ( HasFlagship() )
    {
        for ( auto& pFleet : m_Fleets )
        {
            if ( pFleet->HasFlagship() )
            {
                return true;
            }
        }
    }

    return false;
}

bool Faction::IsFlagshipDestroyed() const
{
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    switch ( GetFactionId() )
    {
    case FactionId::Pirate:
        return pBlackboard->Exists( sKillPirateFlagshipCompleted );
    case FactionId::Marauders:
        return pBlackboard->Exists( sKillMarauderFlagshipCompleted );
    case FactionId::Ascent:
        return pBlackboard->Exists( sKillAscentFlagshipCompleted );
    case FactionId::Iriani:
        return pBlackboard->Exists( sIrianiArcFinished );
    default:
        return false;
    }
}

void Faction::DestroyFleet( FleetWeakPtr fleetWeakPtr )
{
    FleetSharedPtr pFleet = fleetWeakPtr.lock();
    if ( pFleet != nullptr && pFleet->IsTerminating() == false )
    {
        m_FleetsToDestroy.push_back( pFleet );
        pFleet->SetTerminating();
    }
}

bool Faction::sIsEnemyOf( Faction* pFactionA, Faction* pFactionB )
{
    FactionId factionIdA = pFactionA->GetFactionId();
    FactionId factionIdB = pFactionB->GetFactionId();

    if ( factionIdA == factionIdB )
        return false;
    else if ( factionIdA == FactionId::Empire && factionIdB == FactionId::Player )
        return false;
    else if ( factionIdA == FactionId::Player && factionIdB == FactionId::Empire )
        return false;
    else
        return true;
}

const LootProbability& Faction::GetLootProbability( bool isFlagship ) const
{
    if ( isFlagship )
    {
        if ( g_pGame->GetDifficulty() == Difficulty::Hardcore )
        {
            return m_Info.m_LootProbabilityFlagshipHc;
        }
        else
        {
            return m_Info.m_LootProbabilityFlagship;
        }
    }
    else
    {
        return m_Info.m_LootProbability;
    }
}

int Faction::GetConquestReward( const SectorInfo* pSectorInfo, bool* pBonusApplied /* = nullptr */ ) const
{
    int conquestReward = m_Info.m_ConquestReward;
    int bonusReward = 0;

    EmpireFaction* pEmpireFaction = (EmpireFaction*)( g_pGame->GetFaction( FactionId::Empire ) );
    const ImperialRequestList& requests = pEmpireFaction->GetRequestManager()->GetRequests();
    for ( auto& pRequest : requests )
    {
        bonusReward += pRequest->GetConquestReward( pSectorInfo );
    }

    if ( pBonusApplied != nullptr )
    {
        *pBonusApplied = ( bonusReward > 0 );
    }

    return conquestReward + bonusReward;
}

bool Faction::IsCollapsing() const
{
    GameMode gameMode = g_pGame->GetGameMode();
    if ( gameMode == GameMode::Campaign )
    {
        if ( m_Info.m_CollapseTag.empty() )
        {
            return false;
        }
        else
        {
            BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
            return ( pBlackboard != nullptr && pBlackboard->Exists( m_Info.m_CollapseTag ) );
        }
    }
    else if ( gameMode == GameMode::InfiniteWar )
    {
        if ( HasStartedWithHomeworld() == false )
        {
            return false;
        }
        else
        {
            return GetHomeworld() == nullptr;
        }
    }
    else
    {
        return false;
    }
}

bool Faction::Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement )
{
    tinyxml2::XMLElement* pFactionElement = xmlDoc.NewElement( "Faction" );
    pRootElement->LinkEndChild( pFactionElement );

    Xml::Write( xmlDoc, pFactionElement, "FactionId", static_cast<int>( GetFactionId() ) );
    Xml::Write( xmlDoc, pFactionElement, "Turn", m_Turn );
    Xml::Write( xmlDoc, pFactionElement, "NextTurnTimer", m_NextTurnTimer );
    Xml::Write( xmlDoc, pFactionElement, "StartedWithHomeworld", m_StartedWithHomeworld );
    Xml::Write( xmlDoc, pFactionElement, "InitialPresence", ToString( m_InitialPresence ) );

    tinyxml2::XMLElement* pFleetsElement = xmlDoc.NewElement( "Fleets" );
    pFactionElement->LinkEndChild( pFleetsElement );

    for ( auto& pFleet : m_Fleets )
    {
        pFleet->Write( xmlDoc, pFleetsElement );
    }

    return true;
}

bool Faction::Read( tinyxml2::XMLElement* pRootElement )
{
    bool status = true;

    int factionId = -1;

    std::string initialPresence = "";

    for ( tinyxml2::XMLElement* pChildElement = pRootElement->FirstChildElement(); pChildElement != nullptr; pChildElement = pChildElement->NextSiblingElement() )
    {
        Xml::Serialise( pChildElement, "FactionId", factionId );
        Xml::Serialise( pChildElement, "Turn", m_Turn );
        Xml::Serialise( pChildElement, "NextTurnTimer", m_NextTurnTimer );
        Xml::Serialise( pChildElement, "StartedWithHomeworld", m_StartedWithHomeworld );
        Xml::Serialise( pChildElement, "InitialPresence", initialPresence );

        if ( std::string( pChildElement->Value() ) == "Fleets" )
        {
            for ( tinyxml2::XMLElement* pFleetElement = pChildElement->FirstChildElement(); pFleetElement != nullptr; pFleetElement = pFleetElement->NextSiblingElement() )
            {
                SDL_assert_release( std::string( pFleetElement->Value() ) == "Fleet" );

                FleetSharedPtr pFleet = std::make_shared<Fleet>();
                pFleet->SetFaction( this );

                status &= pFleet->Read( pFleetElement );
                m_Fleets.push_back( pFleet );
            }
        }
    }

    if ( initialPresence == "None" )
    {
        m_InitialPresence = FactionPresence::None;
    }
    else if ( initialPresence == "Light" )
    {
        m_InitialPresence = FactionPresence::Light;
    }
    else if ( initialPresence == "Standard" )
    {
        m_InitialPresence = FactionPresence::Standard;
    }
    else if ( initialPresence == "Heavy" )
    {
        m_InitialPresence = FactionPresence::Heavy;
    }

    SDL_assert_release( static_cast<FactionId>( factionId ) == GetFactionId() );

    return status;
}

void Faction::SetInitialPresence( FactionPresence presence )
{
    m_InitialPresence = presence;
}

FactionPresence Faction::GetInitialPresence() const
{
    return m_InitialPresence;
}

const ShipInfoVector& Faction::GetFlagshipFleetShips() const
{
    if ( m_FlagshipFleetShips.empty() )
    {
        ShipInfoManager* pShipInfoManager = g_pGame->GetShipInfoManager();
        for ( std::string shipName : m_Info.m_FlagshipFleetShips )
        {
            const ShipInfo* pShipInfo = pShipInfoManager->Get( this, shipName );
            if ( pShipInfo != nullptr )
            {
                m_FlagshipFleetShips.push_back( pShipInfo );
            }
        }
    }

    return m_FlagshipFleetShips;
}

void Faction::SetHomeworld( SectorInfo* pSectorInfo )
{
    if ( pSectorInfo != nullptr )
    {
        m_StartedWithHomeworld = true;
    }

    m_pHomeworld = pSectorInfo;
}

} // namespace Hexterminate