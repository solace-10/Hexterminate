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

#include "blackboard.h"
#include "fleet/fleet.h"
#include "fleet/fleetcommand.h"
#include "fleet/fleetspawner.h"
#include "hexterminate.h"
#include "player.h"
#include "sector/sector.h"
#include "ship/hexgrid.h"
#include "ship/hyperspacecore.h"
#include "ship/shipinfo.h"

namespace Hexterminate
{

bool FleetSpawner::Spawn( FleetSharedPtr pFleet, Sector* pSector, ShipVector* pSpawnedShips, float x, float y )
{
    if ( pFleet->GetFaction() == g_pGame->GetPlayerFaction() )
    {
        return SpawnFleetPlayer( pFleet, pSector, pSpawnedShips, x, y );
    }
    else
    {
        FleetFormation formation = FleetFormation::Cohesive; // TODO: Implement other formations
        return SpawnFleetAI( pFleet, pSector, pSpawnedShips, x, y, formation );
    }
}

bool FleetSpawner::SpawnFleetAI( FleetSharedPtr pFleet, Sector* pSector, ShipVector* pSpawnedShips, float spawnPointX, float spawnPointY, FleetFormation formation )
{
    CheckFirstEncounter( pFleet->GetFaction() );

    const ShipInfoList& shipsToSpawnList = pFleet->GetShips();
    if ( shipsToSpawnList.empty() )
    {
        return false;
    }

    ShipInfoVector shipsToSpawn( shipsToSpawnList.begin(), shipsToSpawnList.end() );
    if ( pFleet->HasFlagship() )
    {
        CheckFlagshipEncounter( pFleet->GetFaction() );
    }

    FleetCommandUniquePtr pFleetCommand( new FleetCommand );

    ShipSpawnDataVector fleetSpawnData;
    GetSpawnData( shipsToSpawn, spawnPointX, spawnPointY, fleetSpawnData, formation );

    for ( size_t i = 0, s = shipsToSpawn.size(); i < s; ++i )
    {
        Ship* pShip = SpawnShip( pSector, pFleet, shipsToSpawn[ i ], fleetSpawnData[ i ] );

        if ( pFleetCommand->HasLeader() == false )
        {
            pFleetCommand->AssignLeader( pShip );
        }
        else
        {
            pFleetCommand->AssignShip( pShip );
        }

        if ( pSpawnedShips != nullptr )
        {
            pSpawnedShips->push_back( pShip );
        }
    }

    pFleetCommand->SetupRelationships();

    pSector->AddFleetCommand( std::move( pFleetCommand ) );

    return ( shipsToSpawn.size() > 0 );
}

bool FleetSpawner::SpawnFleetPlayer( FleetSharedPtr pFleet, Sector* pSector, ShipVector* pSpawnedShips, float spawnPointX, float spawnPointY )
{
    const std::string& companionShipTemplate = g_pGame->GetPlayer()->GetCompanionShipTemplate();
    const ShipInfo* pCompanionShipInfo = g_pGame->GetShipInfoManager()->Get( g_pGame->GetPlayerFaction(), companionShipTemplate );
    SDL_assert( pCompanionShipInfo != nullptr );

    ShipInfoVector shipsToSpawn;
    shipsToSpawn.push_back( pCompanionShipInfo ); // The ShipInfo here is irrelevant, it is only used for GetSpawnData() to calculate spawn positions correctly

    const ShipInfoList& shipInfos = pFleet->GetShips();
    for ( auto& pShipInfo : shipInfos )
    {
        shipsToSpawn.push_back( pShipInfo );
    }

    ShipSpawnDataVector fleetSpawnData;
    GetSpawnData( shipsToSpawn, spawnPointX, spawnPointY, fleetSpawnData, FleetFormation::Escort );

    FleetCommandUniquePtr pFleetCommand( new FleetCommand );

    // Manually spawn the player's ship, as it will use its own custom hexgrid
    Ship* pPlayerShip = g_pGame->GetPlayer()->CreateShip( spawnPointX, spawnPointY );
    pSector->AddShip( pPlayerShip );
    pFleetCommand->AssignLeader( pPlayerShip );

    int i = 1;
    for ( auto& pShipInfo : shipInfos )
    {
        Ship* pShip = SpawnShip( pSector, pFleet, pShipInfo, fleetSpawnData[ i ] );
        pFleetCommand->AssignShip( pShip );
        i++;
    }

    pFleetCommand->SetupRelationships();
    pSector->AddFleetCommand( std::move( pFleetCommand ) );

    return true;
}

void FleetSpawner::GetSpawnData( ShipInfoVector shipsToSpawn, float spawnPointX, float spawnPointY, ShipSpawnDataVector& fleetSpawnData, FleetFormation formation )
{
    float spacing = 200.0f;
    if ( formation == FleetFormation::Cohesive )
    {
        // TODO: Revise cohesive formation to take into account ship types
        int side = static_cast<int>( ceil( sqrt( shipsToSpawn.size() ) ) );
        ShipSpawnData spawnData;
        for ( size_t i = 0, s = shipsToSpawn.size(); i < s; ++i )
        {
            spawnData.m_PositionX = spawnPointX + static_cast<float>( i % side ) * spacing - ( side * spacing ) / 2.0f;
            spawnData.m_PositionY = spawnPointY + floor( static_cast<float>( i ) / side ) * spacing - ( side * spacing ) / 2.0f;
            fleetSpawnData.push_back( spawnData );
        }
    }
    else if ( formation == FleetFormation::Escort )
    {
        const float deltaStagger = 90.0f;

        // The first ship of the vector is assumed to be the centre of the formation
        ShipSpawnData spawnData;
        spawnData.m_PositionX = spawnPointX;
        spawnData.m_PositionY = spawnPointY;
        fleetSpawnData.push_back( spawnData );

        float anchorX = spawnPointX;
        float anchorY = spawnPointY;

        // The remaining ships are placed in delta formation around the point ship
        int spacingMultiplier = 1;
        anchorY -= deltaStagger;
        size_t numShips = shipsToSpawn.size();
        for ( int i = 0; i < numShips - 1; ++i )
        {
            spawnData.m_PositionX = anchorX + spacing * spacingMultiplier;
            spawnData.m_PositionY = anchorY;
            fleetSpawnData.push_back( spawnData );

            spacing *= -1.0f;
            if ( i % 2 == 1 )
            {
                spacingMultiplier++;
                anchorY -= deltaStagger;
            }
        }
    }
    else
    {
        // TODO: Implement remaining formations
        SDL_assert( false );
    }
}

Ship* FleetSpawner::SpawnShip( Sector* pSector, FleetSharedPtr pFleet, const ShipInfo* pShipInfo, const ShipSpawnData& spawnData )
{
    ShipCustomisationData customisationData;
    customisationData.m_CaptainName = "";
    customisationData.m_ShipName = "";
    customisationData.m_pModuleInfoHexGrid = pShipInfo->GetModuleInfoHexGrid();

    Ship* pShip = new Ship();
    pShip->SetInitialisationParameters(
        pFleet->GetFaction(),
        pFleet,
        customisationData,
        spawnData,
        pShipInfo );

    pShip->Initialize();
    pSector->AddShip( pShip );
    return pShip;
}

void FleetSpawner::CheckFirstEncounter( Faction* pFaction )
{
    static const std::string sFirstEncounterNeutral( "#first_encounter_neutral" );
    static const std::string sFirstEncounterPirates( "#first_encounter_pirates" );
    static const std::string sFirstEncounterMarauders( "#first_encounter_marauders" );
    static const std::string sFirstEncounterAscent( "#first_encounter_ascent" );
    static const std::string sFirstEncounterIriani( "#first_encounter_iriani" );

    FactionId factionId = pFaction->GetFactionId();
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    if ( factionId == FactionId::Neutral && pBlackboard->Get( sFirstEncounterNeutral ) == false )
    {
        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "Most of the Unaligned Sectors once belonged to the Empire but are nowadays "
            "just a shade of their former glory. Stunted by petty wars and centuries of "
            "strife, they still rely on outdated ship designs." );

        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "Our ships should be more than a match for them but if needed, jump out into "
            "hyperspace and join up with another Imperial fleet. But have no doubt, taking "
            "these sectors is for the greater good of the Empire: their resources will "
            "keep our shipyards going." );

        pBlackboard->Add( sFirstEncounterNeutral );
    }
    else if ( factionId == FactionId::Pirate && pBlackboard->Get( sFirstEncounterPirates ) == false )
    {
        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "At some point in the past, those who were involved in copyright infringement were "
            "named pirates. Eventually, they got tired of this and turned into real piracy." );

        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "It is such a fleet we encounter today: pirates and raiders. Their ships are "
            "very fast and prefer to keep their distance but we stand a good chance if we "
            "can bring our weapons to bear." );

        pBlackboard->Add( sFirstEncounterPirates );
    }
    else if ( factionId == FactionId::Marauders && pBlackboard->Get( sFirstEncounterMarauders ) == false )
    {
        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "The edge of galaxy has always posed its threats, with habitable planets often "
            "too far from each other for the Empire's scattered outposts to keep in line. The "
            "settlers of the Rim were for the most part outcasts, prisoners, freethinkers and "
            "other undesirables." );

        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "They field slow but heavily armoured ships which can take a great deal of "
            "punishment while boring you to tears with their talks of freedom. They will "
            "understand, in time, that Unity is Strength. Now let us wipe their fleet out of "
            "our sector." );

        pBlackboard->Add( sFirstEncounterMarauders );
    }
    else if ( factionId == FactionId::Ascent && pBlackboard->Get( sFirstEncounterAscent ) == false )
    {
        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "The Ascent were originally a small breakaway faction which turned into worshipping "
            "their local star, for some reason. Eventually they declared independence and have "
            "vowed to destroy the Empire after we turned their star supernova." );

        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "Their doctrine is one of fielding many small fleets which make use of shielding "
            "technology. This allows them to reinforce neighbouring sectors quickly, so the "
            "tide can turn while we're engaged." );

        pBlackboard->Add( sFirstEncounterAscent );
    }
    else if ( factionId == FactionId::Iriani && pBlackboard->Get( sFirstEncounterIriani ) == false )
    {
        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "Far from the heart of the Empire, Iriani Prime was fairly unaffected by the Great "
            "Collapse. The region controlled by today's Iriani Federation is slowly expanding "
            "but they too need to be brought back to the Imperial fold." );

        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "Their ships have some of the most advanced technology in the galaxy, making them "
            "both difficult to overwhelm and expensive to produce. Expect considerable losses "
            "when engaging Iriani fleets but have no doubt, the modules you could recover would "
            "be well worth it." );

        pBlackboard->Add( sFirstEncounterIriani );
    }
}

void FleetSpawner::CheckFlagshipEncounter( Faction* pFaction )
{
    static const std::string sPirateLeaderEncountered( "#pirate_leader_encountered" );
    static const std::string sMaraudersLeaderEncountered( "#marauders_leader_encountered" );

    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    FactionId factionId = pFaction->GetFactionId();
    if ( factionId == FactionId::Pirate && pBlackboard->Exists( sPirateLeaderEncountered ) == false )
    {
        g_pGame->AddIntel( GameCharacter::NavarreHexer,
            "Well, about time you guys come out. Empire's trying to get back to the good old "
            "days, eh? Doubt it, these days you can't make ships worth crap. Come on, let me "
            "show you this beauty I found in one of those systems you guys abandoned. "
            "Tail between your legs and all." );

        pBlackboard->Add( sPirateLeaderEncountered );
    }
    else if ( factionId == FactionId::Marauders && pBlackboard->Exists( sMaraudersLeaderEncountered ) == false )
    {
        g_pGame->AddIntel( GameCharacter::HarkonStormchaser,
            "Finally, we meet on the battlefield! The Imperial flagship, fielded at last. Of course, "
            "the Emperor had to send his favourite lap dog to try to crush us, didn't he?" );

        g_pGame->AddIntel( GameCharacter::HarkonStormchaser,
            "Today, you will see what free men and women are made of! Come! Come and face our guns!" );

        pBlackboard->Add( sMaraudersLeaderEncountered );
    }
}

} // namespace Hexterminate
