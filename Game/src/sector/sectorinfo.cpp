// Copyright 2014 Pedro Nunes
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

#include <genesis.h>
#include <logger.h>
#include <math/misc.h>
#include <xml.h>

#include "faction/faction.h"
#include "fleet/fleet.h"
#include "globals.h"
#include "hexterminate.h"
#include "player.h"
#include "requests/imperialrequest.h"
#include "requests/requestmanager.h"
#include "sector/backgroundinfo.h"
#include "sector/galaxy.h"
#include "sector/sectorinfo.h"
#include "ship/inventory.h"
#include "ship/moduleinfo.h"
#include "ship/shipinfo.h"
#include "xmlaux.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// SectorInfo
///////////////////////////////////////////////////////////////////////////////

SectorInfo::SectorInfo()
    : m_Coordinates( -1, -1 )
    , m_pFaction( nullptr )
    , m_HasShipyard( false )
    , m_HasProbe( false )
    , m_HasStarfort( false )
    , m_HasHyperspaceInhibitor( false )
    , m_Contested( false )
    , m_AutoResolve( true )
    , m_IsPersonal( false )
    , m_HasStar( true )
    , m_HasProceduralSpawning( true )
    , m_IsHomeworld( false )
    , m_RegionalFleetBasePoints( 0 )
    , m_RegionalFleetPoints( 0 )
    , m_StarfortHealth( 0 )
    , m_pBackgroundInfo( nullptr )
{
}

SectorInfo::SectorInfo( int x, int y )
    : m_Coordinates( x, y )
    , m_HasShipyard( false )
    , m_HasProbe( false )
    , m_HasStarfort( false )
    , m_Contested( false )
    , m_AutoResolve( true )
    , m_IsPersonal( false )
    , m_HasStar( true )
    , m_HasProceduralSpawning( true )
    , m_IsHomeworld( false )
    , m_RegionalFleetBasePoints( 0 )
    , m_RegionalFleetPoints( 0 )
    , m_StarfortHealth( 0 )
    , m_pBackgroundInfo( nullptr )
{
    m_pFaction = nullptr;
}

void SectorInfo::SetupRegionalFleet( bool isEasySector )
{
    if ( isEasySector )
    {
        m_RegionalFleetBasePoints = gRand( 250.0f, 350.0f );
        m_RegionalFleetPoints = static_cast<int>( m_RegionalFleetBasePoints );
    }
    else
    {
        // The regional fleets become stronger the further away the player is from center of the Empire
        using namespace Genesis;
        const int regionalFleetResistenceStep = 70;
        const int distanceToTannhauser = ( m_Coordinates - glm::ivec2( 7, 11 ) ).length();
        const int additionalResistence = regionalFleetResistenceStep * distanceToTannhauser;

        m_RegionalFleetBasePoints = floor( gRand( RegionalFleetMin, RegionalFleetMax ) / 10.0f ) * 10 + additionalResistence;
        m_RegionalFleetPoints = static_cast<int>( m_RegionalFleetBasePoints );
    }
}

void SectorInfo::ProcessTurn()
{
    UpdateRegionalFleet();
    UpdateContestedStatus();
    UpdateStarfort();
}

void SectorInfo::UpdateRegionalFleet()
{
    if ( m_Contested )
    {
        return;
    }

    m_RegionalFleetPoints = gClamp<int>( m_RegionalFleetPoints, (int)( m_RegionalFleetPoints + m_RegionalFleetBasePoints * RegionalFleetRegen ), (int)m_RegionalFleetBasePoints );
}

void SectorInfo::UpdateStarfort()
{
    if ( m_Contested )
    {
        return;
    }

    m_StarfortHealth = std::min( m_StarfortHealth + static_cast<int>( StarfortRegen * 100.0f ), 100 );
}

void SectorInfo::Contest()
{
    m_Contested = true;
    m_ContestedFleets.clear();

    for ( int i = 0; i < (int)FactionId::Count; ++i )
    {
        for ( auto& pFleet : g_pGame->GetFaction( (FactionId)i )->GetFleets() )
        {
            if ( pFleet->IsTerminating() == false && pFleet->GetCurrentSector() == this && pFleet->IsImmune() == false && m_ContestedFleets.size() < cMaxContestingFleets )
            {
                m_ContestedFleets.push_back( pFleet );
                pFleet->SetEngaged( true );
            }
        }
    }
}

void SectorInfo::UpdateContestedStatus()
{
    if ( m_Contested == false || m_AutoResolve == false )
        return;

    // We need to know how many fleets of each faction are present in this sector so we can spread the damage correctly
    float damage = 70.0f;
    int numFactionFleets[ (int)FactionId::Count ];
    for ( int i = 0; i < (int)FactionId::Count; ++i )
        numFactionFleets[ i ] = 0;

    for ( FleetWeakPtr fleetWeakPtr : m_ContestedFleets )
    {
        FleetSharedPtr pFleet = fleetWeakPtr.lock();
        if ( pFleet != nullptr )
        {
            numFactionFleets[ (int)pFleet->GetFaction()->GetFactionId() ]++;
        }
    }

    // If we have a regional fleet, count that as well
    int regionalFleetIdx = (int)GetFaction()->GetFactionId();
    if ( GetRegionalFleetPoints() > 0 )
        numFactionFleets[ regionalFleetIdx ]++;

    // Damage all contesting AI fleets, unless the fleet has a flagship.
    // This makes the flagship fleets a force to be reckoned it.
    FleetWeakPtrList::iterator it = m_ContestedFleets.begin();
    while ( it != m_ContestedFleets.end() )
    {
        FleetSharedPtr pFleet = it->lock();
        if ( pFleet == nullptr )
        {
            it = m_ContestedFleets.erase( it );
        }
        else if ( pFleet->GetPoints() > 0 && pFleet->HasFlagship() )
        {
            it++;
        }
        else
        {
            int factionIdx = (int)pFleet->GetFaction()->GetFactionId();

            // Damage this fleet, splitting the damage evenly across fleets of the same faction
            pFleet->SetAutoResolvePoints( pFleet->GetAutoResolvePoints() - (int)( damage / numFactionFleets[ (int)factionIdx ] ) );

            // Was this fleet destroyed?
            if ( pFleet->GetAutoResolvePoints() == 0 || pFleet->GetPoints() == 0 )
            {
                numFactionFleets[ (int)pFleet->GetFaction()->GetFactionId() ]--;
                pFleet->GetFaction()->DestroyFleet( pFleet );
                it = m_ContestedFleets.erase( it );
            }
            else
            {
                it++;
            }
        }
    }

    // Personal sectors can only be taken by the player, to prevent other factions or fleets from
    // trivialising the campaign.
    if ( GetRegionalFleetPoints() > 0 && IsPersonal() == false )
    {
        SetRegionalFleetPoints( GetRegionalFleetPoints() - (int)( damage / numFactionFleets[ regionalFleetIdx ] ) );

        // Was the regional fleet destroyed?
        if ( GetRegionalFleetPoints() == 0 )
        {
            numFactionFleets[ regionalFleetIdx ]--;
        }
    }

    // If there are no fleets in this sector, then they've all been destroyed and the sector's ownership remains as it was
    // (Victory for the regional fleet!)
    if ( m_ContestedFleets.empty() )
    {
        m_Contested = false;
    }
    else
    {
        if ( HasStarfort() )
        {
            m_StarfortHealth -= static_cast<int>( m_ContestedFleets.size() );
            if ( m_StarfortHealth < 0 )
            {
                SetStarfort( false );
            }
        }

        // How many factions are still fighting for this?
        int numDifferentFactions = 0;
        for ( int i = 0; i < (int)FactionId::Count; ++i )
        {
            if ( numFactionFleets[ i ] > 0 )
                numDifferentFactions++;
        }

        // A single faction now controls this sector and it has no starfort - notify all fleets of its victory
        if ( numDifferentFactions == 1 && HasStarfort() == false )
        {
            for ( auto& contestedFleet : m_ContestedFleets )
            {
                FleetSharedPtr pFleet = contestedFleet.lock();
                if ( pFleet != nullptr )
                {
                    pFleet->NotifyBattleWon();
                    pFleet->SetEngaged( false );
                }
            }
            m_Contested = false;
        }
    }
}

int SectorInfo::GetRegionalFleetPoints() const
{
    return m_RegionalFleetPoints;
}

void SectorInfo::SetRegionalFleetPoints( int value )
{
    m_RegionalFleetPoints = value;
    if ( m_RegionalFleetPoints < 0 )
        m_RegionalFleetPoints = 0;
}

// Changing faction immediately will have issues if this sector is being processed as part of the
// faction's update loop. Only use immediate if the controlled sectors list is not currently being iterated on.
void SectorInfo::SetFaction( Faction* pFaction, bool immediate, bool byPlayer )
{
    if ( pFaction == m_pFaction )
    {
        return;
    }
    else
    {
        if ( m_pFaction != nullptr )
        {
            m_pFaction->RemoveControlledSector( this, immediate );
        }
        pFaction->AddControlledSector( this, immediate, byPlayer );
        m_pFaction = pFaction;
    }
}

void SectorInfo::FleetDisengaged( FleetWeakPtr fleetWeakPtr )
{
    FleetSharedPtr pDisengagedFleet = fleetWeakPtr.lock();
    if ( pDisengagedFleet == nullptr )
    {
        return;
    }

    bool found = false;

    // Remove from the list of contested fleets
    for ( FleetWeakPtrList::iterator it = m_ContestedFleets.begin(), itEnd = m_ContestedFleets.end(); it != itEnd; ++it )
    {
        FleetSharedPtr pContestedFleet = it->lock();
        if ( pContestedFleet == pDisengagedFleet )
        {
            m_ContestedFleets.erase( it );
            found = true;
            break;
        }
    }

    if ( found )
    {
        pDisengagedFleet->SetEngaged( false );
        pDisengagedFleet->SetImmunity( true ); // Makes this fleet immune for a few seconds so it can run away / relocate without immediately getting into another battle

        // If the disengaging fleet is the player's, turn on auto-resolve so the battle can carry on in the background
        if ( pDisengagedFleet == g_pGame->GetPlayerFleet().lock() )
        {
            SetAutoResolve( true );
        }
    }
}

void SectorInfo::ForceResolve( Faction* pVictoriousFaction )
{
    SDL_assert( pVictoriousFaction != g_pGame->GetFaction( FactionId::Player ) );

    if ( pVictoriousFaction != GetFaction() )
    {
        // Set the regional fleet to a basic level
        SetRegionalFleetPoints( static_cast<int>( m_RegionalFleetBasePoints / 2.0f ) );

        SetFaction( pVictoriousFaction, false, true );
    }

    for ( FleetWeakPtr contestedFleetWeakPtr : m_ContestedFleets )
    {
        FleetSharedPtr pFleet = contestedFleetWeakPtr.lock();
        if ( pFleet != nullptr )
        {
            Faction* pFleetFaction = pFleet->GetFaction();

            if ( pFleetFaction == pVictoriousFaction || ( pVictoriousFaction == g_pGame->GetFaction( FactionId::Empire ) && pFleetFaction == g_pGame->GetFaction( FactionId::Player ) ) )
            {
                pFleet->SetEngaged( false );
            }
            else
            {
                pFleetFaction->DestroyFleet( contestedFleetWeakPtr );
            }
        }
    }
    m_ContestedFleets.clear();

    SetAutoResolve( false );
    m_Contested = false;
}

void SectorInfo::RestoreRegionalFleet()
{
    m_RegionalFleetPoints = static_cast<int>( m_RegionalFleetBasePoints );
}

int SectorInfo::GetConquestReward() const
{
    if ( m_pFaction == nullptr )
    {
        return 0;
    }
    else
    {
        return m_pFaction->GetConquestReward( this );
    }
}

void SectorInfo::SetStarfort( bool state )
{
    if ( state )
    {
        m_HasStarfort = true;
        m_StarfortHealth = 100;
    }
    else
    {
        m_HasStarfort = false;
        m_StarfortHealth = 0;
    }
}

bool SectorInfo::Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement )
{
    using namespace tinyxml2;

    XMLElement* pSectorElement = xmlDoc.NewElement( "Sector" );
    pRootElement->LinkEndChild( pSectorElement );

    Xml::Write( xmlDoc, pSectorElement, "Name", m_Name );
    Xml::Write( xmlDoc, pSectorElement, "X", m_Coordinates.x );
    Xml::Write( xmlDoc, pSectorElement, "Y", m_Coordinates.y );
    Xml::Write( xmlDoc, pSectorElement, "Faction", GetFaction()->GetName() );
    Xml::Write( xmlDoc, pSectorElement, "Shipyard", HasShipyard() );
    Xml::Write( xmlDoc, pSectorElement, "Probe", HasProbe() );
    Xml::Write( xmlDoc, pSectorElement, "Starfort", HasStarfort() );
    Xml::Write( xmlDoc, pSectorElement, "StarfortHealth", GetStarfortHealth() );
    Xml::Write( xmlDoc, pSectorElement, "HyperspaceInhibitor", HasHyperspaceInhibitor() );
    Xml::Write( xmlDoc, pSectorElement, "RegionalFleetBasePoints", (int)m_RegionalFleetBasePoints );
    Xml::Write( xmlDoc, pSectorElement, "RegionalFleetPoints", m_RegionalFleetPoints );
    Xml::Write( xmlDoc, pSectorElement, "BackgroundId", m_pBackgroundInfo->GetId() );
    Xml::Write( xmlDoc, pSectorElement, "Personal", m_IsPersonal );
    Xml::Write( xmlDoc, pSectorElement, "HasStar", m_HasStar );
    Xml::Write( xmlDoc, pSectorElement, "Homeworld", m_IsHomeworld );

    SDL_assert( m_pBackgroundInfo != nullptr );
    if ( m_pBackgroundInfo == nullptr )
    {
        return false;
    }
    else
    {
        Xml::Write( xmlDoc, pSectorElement, "BackgroundId", m_pBackgroundInfo->GetId() );
    }

    if ( m_ComponentNames.empty() == false )
    {
        XMLElement* pComponentsElement = xmlDoc.NewElement( "Components" );
        pSectorElement->LinkEndChild( pComponentsElement );

        for ( const std::string& componentName : m_ComponentNames )
        {
            Xml::Write( xmlDoc, pComponentsElement, "Component", componentName );
        }
    }

    return true;
}

bool SectorInfo::Read( tinyxml2::XMLElement* pRootElement )
{
    std::string faction;
    int backgroundId = -1;

    for ( tinyxml2::XMLElement* pChildElement = pRootElement->FirstChildElement(); pChildElement != nullptr; pChildElement = pChildElement->NextSiblingElement() )
    {
        Xml::Serialise( pChildElement, "Name", m_Name );
        Xml::Serialise( pChildElement, "X", m_Coordinates.x );
        Xml::Serialise( pChildElement, "Y", m_Coordinates.y );
        Xml::Serialise( pChildElement, "Faction", faction );
        Xml::Serialise( pChildElement, "Shipyard", m_HasShipyard );
        Xml::Serialise( pChildElement, "Probe", m_HasProbe );
        Xml::Serialise( pChildElement, "Starfort", m_HasStarfort );
        Xml::Serialise( pChildElement, "StarfortHealth", m_StarfortHealth );
        Xml::Serialise( pChildElement, "HyperspaceInhibitor", m_HasHyperspaceInhibitor );
        Xml::Serialise( pChildElement, "RegionalFleetBasePoints", m_RegionalFleetBasePoints );
        Xml::Serialise( pChildElement, "RegionalFleetPoints", m_RegionalFleetPoints );
        Xml::Serialise( pChildElement, "BackgroundId", backgroundId );
        Xml::Serialise( pChildElement, "Personal", m_IsPersonal );
        Xml::Serialise( pChildElement, "HasStar", m_HasStar );
        Xml::Serialise( pChildElement, "Homeworld", m_IsHomeworld );

        if ( std::string( "Components" ) == pChildElement->Value() )
        {
            ReadComponents( pChildElement );
        }
    }

    Faction* pFaction = nullptr;
    if ( faction.empty() )
    {
        Genesis::FrameWork::GetLogger()->LogWarning( "No faction defined for sector '%d / %d', setting ownership to Neutral", m_Coordinates.x, m_Coordinates.y );
        pFaction = g_pGame->GetFaction( FactionId::Neutral );
    }
    else
    {
        pFaction = g_pGame->GetFaction( faction );
        if ( pFaction == nullptr )
        {
            Genesis::FrameWork::GetLogger()->LogWarning( "Invalid faction '%s' for sector '%d / %d', setting ownership to Neutral",
                faction.c_str(), m_Coordinates.x, m_Coordinates.y );
            pFaction = g_pGame->GetFaction( FactionId::Neutral );
        }
        else if ( m_IsHomeworld )
        {
            pFaction->SetHomeworld( this );
        }
    }

    SetFaction( pFaction, true, false );

    SDL_assert( m_Coordinates.x >= 0 );
    SDL_assert( m_Coordinates.x < NumSectorsX );
    SDL_assert( m_Coordinates.y >= 0 );
    SDL_assert( m_Coordinates.y < NumSectorsY );
    SDL_assert( backgroundId != -1 );

    for ( auto& background : g_pGame->GetBackgrounds() )
    {
        if ( background.GetId() == backgroundId )
        {
            m_pBackgroundInfo = &background;
        }
    }

    SDL_assert( m_pBackgroundInfo != nullptr );

    return true;
}

void SectorInfo::UpgradeFromVersion( int version )
{
    if ( version == 1 )
    {
        m_IsHomeworld = false;
        version++;
    }

    if ( version == 2 )
    {
        if ( HasStarfort() )
        {
            SetStarfortHealth( 100 );
        }

        version++;
    }
}

void SectorInfo::ReadComponents( tinyxml2::XMLElement* pRootElement )
{
    std::string componentName;
    for ( tinyxml2::XMLElement* pChildElement = pRootElement->FirstChildElement(); pChildElement != nullptr; pChildElement = pChildElement->NextSiblingElement() )
    {
        Xml::Serialise( pChildElement, "Component", componentName );
        m_ComponentNames.insert( componentName );
    }
}

void SectorInfo::AddComponentName( const std::string& componentName )
{
    m_ComponentNames.insert( componentName );
    ;
}

bool SectorInfo::HasComponentName( const std::string& componentName ) const
{
    return ( m_ComponentNames.find( componentName ) != m_ComponentNames.cend() );
}

void SectorInfo::GetBorderingSectors( SectorInfoVector& sectors, bool allowDiagonals /* = true */ ) const
{
    sectors.reserve( 8 );
    auto surroundingSectorFn = [ &sectors ]( int x, int y ) {
        SectorInfo* pSectorInfo = g_pGame->GetGalaxy()->GetSectorInfo( x, y );
        if ( pSectorInfo != nullptr )
        {
            sectors.push_back( pSectorInfo );
        }
    };

    surroundingSectorFn( m_Coordinates.x, m_Coordinates.y - 1 );
    surroundingSectorFn( m_Coordinates.x - 1, m_Coordinates.y );
    surroundingSectorFn( m_Coordinates.x + 1, m_Coordinates.y );
    surroundingSectorFn( m_Coordinates.x, m_Coordinates.y + 1 );

    if ( allowDiagonals )
    {
        surroundingSectorFn( m_Coordinates.x - 1, m_Coordinates.y - 1 );
        surroundingSectorFn( m_Coordinates.x + 1, m_Coordinates.y - 1 );
        surroundingSectorFn( m_Coordinates.x - 1, m_Coordinates.y + 1 );
        surroundingSectorFn( m_Coordinates.x + 1, m_Coordinates.y + 1 );
    }
}

ThreatRating SectorInfo::GetThreatRating() const
{
    int alliedScore = 0;
    int hostileScore = 0;

    FleetSharedPtr pPlayerFleet = g_pGame->GetPlayerFleet().lock();
    alliedScore += CalculateThreatValue( pPlayerFleet );

    for ( int i = 0; i < (int)FactionId::Count; ++i )
    {
        Faction* pFaction = g_pGame->GetFaction( (FactionId)i );
        const FleetList& factionFleets = pFaction->GetFleets();
        for ( auto& pFleet : factionFleets )
        {
            if ( pFleet != nullptr && pFleet->GetCurrentSector() == this && pFleet != pPlayerFleet )
            {
                if ( Faction::sIsEnemyOf( g_pGame->GetPlayerFaction(), pFleet->GetFaction() ) )
                {
                    hostileScore += CalculateThreatValue( pFleet );
                }
                else
                {
                    alliedScore += CalculateThreatValue( pFleet );
                }
            }
        }
    }

    const int regionalFleetPoints = GetRegionalFleetPoints();
    if ( Faction::sIsEnemyOf( g_pGame->GetPlayerFaction(), GetFaction() ) )
    {
        hostileScore += static_cast<int>( regionalFleetPoints * GetFaction()->GetThreatValueMultiplier() );
    }
    else
    {
        alliedScore += static_cast<int>( regionalFleetPoints * GetFaction()->GetThreatValueMultiplier() );
    }

    const ImperialRequestList& requests = g_pGame->GetRequestManager()->GetRequests();
    for ( auto& request : requests )
    {
        const RequestGoalSharedPtrList& goals = request->GetGoals();
        for ( auto& goal : goals )
        {
            if ( goal->GetSectorInfo() == this )
            {
                hostileScore += request->GetThreatScore();
            }
        }
    }

    return CalculateThreatRating( alliedScore, hostileScore );
}

ThreatRating SectorInfo::CalculateThreatRating( int alliedScore, int hostileScore ) const
{
    bool isSectorHostile = Faction::sIsEnemyOf( g_pGame->GetPlayerFaction(), GetFaction() );

    if ( hostileScore <= 0 && isSectorHostile == false )
        return ThreatRating::None;

    SDL_assert( alliedScore != 0 );

    const float ratio = static_cast<float>( hostileScore ) / static_cast<float>( alliedScore );
    if ( ratio < 0.33f )
        return ThreatRating::Trivial;
    else if ( ratio < 0.8f )
        return ThreatRating::Easy;
    else if ( ratio < 1.5f )
        return ThreatRating::Fair;
    else if ( ratio < 2.5f )
        return ThreatRating::Challenging;
    else
        return ThreatRating::Overpowering;
}

int SectorInfo::CalculateThreatValue( FleetSharedPtr pFleet ) const
{
    int threatValue = 0;

    ShipInfoManager* pShipInfoManager = g_pGame->GetShipInfoManager();
    if ( pFleet->GetFaction() == g_pGame->GetPlayerFaction() )
    {
        FleetSharedPtr pPlayerFleet = g_pGame->GetPlayerFleet().lock();
        if ( pPlayerFleet != nullptr )
        {
            const ShipInfoList& shipInfos = pPlayerFleet->GetShips();
            for ( auto& pShipInfo : shipInfos )
            {
                threatValue += pShipInfo->GetThreatValue();
            }
        }

        const Player* pPlayer = g_pGame->GetPlayer();
        ModuleInfoHexGrid* pPlayerHexGrid = pPlayer->GetInventory()->GetHexGrid();
        threatValue += ShipInfo::sCalculateThreatValue( pPlayerHexGrid );
    }
    else
    {
        threatValue += static_cast<int>( pFleet->GetPoints() * pFleet->GetFaction()->GetThreatValueMultiplier() );

        if ( pFleet->HasFlagship() )
        {
            const ShipInfo* pFlagshipInfo = pShipInfoManager->Get( pFleet->GetFaction(), "special_flagship" );
            SDL_assert( pFlagshipInfo != nullptr );
            threatValue += pFlagshipInfo->GetThreatValue();
        }
    }

    return threatValue;
}

void SectorInfo::SetHomeworld( bool state )
{
    m_IsHomeworld = true;
}

bool SectorInfo::IsHomeworld() const
{
    return m_IsHomeworld;
}

} // namespace Hexterminate