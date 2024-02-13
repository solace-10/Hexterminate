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

#include <math/constants.h>

#include "requests/campaignrequest.h"

#include "achievements.h"
#include "blackboard.h"
#include "faction/faction.h"
#include "fleet/fleet.h"
#include "hexterminate.h"
#include "requests/campaigntags.h"
#include "requests/invasionrequestinfo.h"
#include "sector/galaxy.h"
#include "sector/sector.h"
#include "sector/sectorinfo.h"
#include "ship/ship.h"
#include "ship/shipinfo.h"

namespace Hexterminate
{

static const Genesis::Color sCampaignGoalColour( 0.0f, 0.0f, 0.6f, 0.6f );

CampaignRequest::CampaignRequest( RequestManager* pRequestManager )
    : ImperialRequest( pRequestManager )
    , m_UpdateInterval( 5.0f )
{
}

void CampaignRequest::OnBegin()
{
}

void CampaignRequest::OnSuccess()
{
}

void CampaignRequest::Update( float delta )
{
    ImperialRequest::Update( delta );

    if ( m_UpdateInterval <= 0.0f )
    {
        UpdateExpansionArc( delta );
        UpdatePirateArc( delta );
        UpdateMarauderArc( delta );
        UpdateAscentArc( delta );
        UpdateIrianiArc( delta );
        UpdateChrysamereArc( delta );

        m_UpdateInterval = 1.0f;
    }
    else
    {
        m_UpdateInterval = 0.0f;
    }
}

void CampaignRequest::UpdateExpansionArc( float delta )
{
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    if ( pBlackboard->Exists( sExpansionArcFinished ) )
    {
        return;
    }

    if ( pBlackboard->Exists( sFirstExpansionEvent ) == false )
    {
        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "Our conquest will begin with Aster 9/11, which has declared independence 80 "
            "years ago. Our fleet is the vanguard - other fleets are being currently assembled "
            "and will soon join the war effort." );

        pBlackboard->Add( sFirstExpansionEvent );
    }

    SectorInfo* pSectorAster = g_pGame->GetGalaxy()->GetSectorInfo( 9, 11 );
    if ( m_pExpansionGoal == nullptr && pBlackboard->Exists( sFirstExpansionEvent ) && pBlackboard->Exists( sFirstExpansionEventCompleted ) == false )
    {
        m_pExpansionGoal = std::make_shared<RequestGoal>( pSectorAster, "Campaign: claim Aster 9/11", sCampaignGoalColour );
        AddGoal( m_pExpansionGoal );
        SetThreatScore( 0 );
    }

    // Player has captured the Aster sector
    if ( pBlackboard->Exists( sFirstExpansionEventCompleted ) == false && pSectorAster->GetFaction()->GetFactionId() == FactionId::Empire )
    {
        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "No further hostile signatures in the Aster sector. As our borders grow once more, "
            "the resources from these sectors will go straight to the war effort and to expand our fleet." );

        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "I'll notify HQ that planetary bombardment can now commence. Good work, Captain." );

        pBlackboard->Add( sFirstExpansionEventCompleted );

        RemoveGoal( m_pExpansionGoal );
        m_pExpansionGoal = nullptr;
    }

    // Begin the Syrion sector expansion event after Aster is captured
    SectorInfo* pSectorSyrion = g_pGame->GetGalaxy()->GetSectorInfo( 8, 14 );
    if ( pBlackboard->Exists( sFirstExpansionEventCompleted ) && pBlackboard->Exists( sSecondExpansionEvent ) == false && pSectorSyrion->GetFaction()->GetFactionId() != FactionId::Empire )
    {
        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "Our next target is Syrion 8/14. Unlike Aster 9/10, this sector has a shipyard which would "
            "be of considerable benefit to the Empire. We expect this sector to be better defended." );

        pBlackboard->Add( sSecondExpansionEvent );
    }

    if ( m_pExpansionGoal == nullptr && pBlackboard->Exists( sSecondExpansionEvent ) && pBlackboard->Exists( sSecondExpansionEventCompleted ) == false )
    {
        m_pExpansionGoal = std::make_shared<RequestGoal>( pSectorSyrion, "Campaign: claim Syrion 8/14", sCampaignGoalColour );
        AddGoal( m_pExpansionGoal );
        SetThreatScore( GetShipThreatScore( "special_turret", FactionId::Neutral ) * 2 );
    }

    // Player has captured the Syrion sector
    if ( pBlackboard->Exists( sSecondExpansionEventCompleted ) == false && pSectorSyrion->GetFaction()->GetFactionId() == FactionId::Empire )
    {
        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "The shipyard in Syrion 8/14 has announced its surrender. We'll put its capabilities to good "
            "use and begin assembling a fleet." );

        pBlackboard->Add( sSecondExpansionEventCompleted );

        RemoveGoal( m_pExpansionGoal );
        m_pExpansionGoal = nullptr;
    }

    // Wrap up the arc once the Syrion sector is ours
    if ( pBlackboard->Exists( sSecondExpansionEventCompleted ) )
    {
        pBlackboard->Add( sExpansionArcFinished );
    }
}

void CampaignRequest::UpdatePirateArc( float delta )
{
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();

    if ( pBlackboard->Exists( sExpansionArcFinished ) == false || pBlackboard->Exists( sPirateArcFinished ) )
    {
        return;
    }

    if ( pBlackboard->Exists( sFirstPirateEvent ) == false )
    {
        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "Our deep space probes have, over the years, managed to track down a number of raider shipyards "
            "scattered throughout the galaxy. Reclaiming those sectors will prevent them from assembling any "
            "further fleets." );

        pBlackboard->Add( sFirstPirateEvent );
    }
    else if ( pBlackboard->Exists( sFirstPirateEventCompleted ) == false && pBlackboard->Exists( sKillPirateFlagshipCompleted ) == false )
    {
        // Display all pirate sectors as campaign goals until the player takes 3 of them.
        ClearGoals( m_PirateSectorGoals );

        if ( pBlackboard->Get( sPirateShipyardsCaptured ) < 3 )
        {
            Faction* pPirateFaction = g_pGame->GetFaction( FactionId::Pirate );
            for ( auto& pControlledSector : pPirateFaction->GetControlledSectors() )
            {
                RequestGoalSharedPtr pGoal = std::make_shared<RequestGoal>( pControlledSector, "Campaign: defeat Raiders", sCampaignGoalColour );
                m_PirateSectorGoals.push_back( pGoal );
                AddGoal( pGoal );
                SetThreatScore( 0 );
            }
        }
        else
        {
            pBlackboard->Add( sFirstPirateEventCompleted );
        }
    }
    else if ( pBlackboard->Exists( sFirstPirateEventCompleted ) && pBlackboard->Exists( sKillPirateFlagship ) == false && pBlackboard->Exists( sKillPirateFlagshipCompleted ) == false )
    {
        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "It is no good. With their leader alive, these pirates just keep regrouping. We'll have to cut "
            "this at the root - our target is Navarre Hexer, a man well known to us after several successful "
            "raids in our space." );

        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "Without effective leadership, infighting will prevent them from mounting an effective resistance "
            "and we'll be able to stamp them out." );

        pBlackboard->Add( sKillPirateFlagship );
    }
    else if ( pBlackboard->Exists( sKillPirateFlagship ) && pBlackboard->Exists( sKillPirateFlagshipCompleted ) == false )
    {
        if ( m_pFleetGoal != nullptr && m_pFleetGoal->GetFleet().expired() )
        {
            RemoveGoal( m_pFleetGoal );
            m_pFleetGoal = nullptr;
        }

        if ( m_pFleetGoal == nullptr )
        {
            Faction* pPirateFaction = g_pGame->GetFaction( FactionId::Pirate );
            for ( auto& pFleet : pPirateFaction->GetFleets() )
            {
                if ( pFleet->HasFlagship() )
                {
                    m_pFleetGoal = std::make_shared<RequestGoal>( pFleet, "Campaign: destroy Navarre Hexer's flagship", sCampaignGoalColour );
                    AddGoal( m_pFleetGoal );
                    SetThreatScore( 0 );
                    break;
                }
            }
        }
    }
    else if ( pBlackboard->Exists( sKillPirateFlagshipCompleted ) )
    {
        if ( m_pFleetGoal != nullptr )
        {
            RemoveGoal( m_pFleetGoal );
            m_pFleetGoal = nullptr;
        }

        pBlackboard->Add( sPirateArcFinished );
    }
}

void CampaignRequest::UpdateMarauderArc( float delta )
{
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    static std::string invasionTag( InvasionRequestInfo::GetBlackboardTag( FactionId::Marauders ) );

    if ( pBlackboard->Exists( sPirateArcFinished ) == false || pBlackboard->Exists( sMarauderArcFinished ) )
    {
        return;
    }

    SectorInfo* pSectorSurtr = g_pGame->GetGalaxy()->GetSectorInfo( 5, 18 );
    SectorInfo* pSectorMuspell = g_pGame->GetGalaxy()->GetSectorInfo( 12, 22 );
    SectorInfo* pSectorValhalla = g_pGame->GetGalaxy()->GetSectorInfo( 6, 20 );
    FactionId sectorSurtrFactionId = pSectorSurtr->GetFaction()->GetFactionId();
    FactionId sectorMuspellFactionId = pSectorMuspell->GetFaction()->GetFactionId();
    if ( pBlackboard->Exists( sMarauderIntro ) == false && pBlackboard->Exists( sKillMarauderFlagshipCompleted ) == false )
    {
        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "Let us turn our eyes to these Marauders. They've always had strange ideas about freedom and not "
            "doing their part for the Empire. We should now be in a position where we can strike against them "
            "and protect our way of living against these dangerous ideas." );

        if ( sectorSurtrFactionId == FactionId::Marauders && sectorMuspellFactionId == FactionId::Marauders )
        {
            g_pGame->AddIntel( GameCharacter::FleetIntelligence,
                "We have identified two major staging areas - the beating hearts of their movement." );

            g_pGame->AddIntel( GameCharacter::FleetIntelligence,
                "Surtr sector contains powerful communication relays which broadcast messages that have not "
                "been pre-approved by the Ministry of Reason. This must not be allowed to continue." );

            g_pGame->AddIntel( GameCharacter::FleetIntelligence,
                "The other sector, Muspell, is guarded by a powerful battle station which provides them with "
                "an extremely strong position in that region of space." );

            pBlackboard->Add( sConquerSurtr );
            pBlackboard->Add( sConquerMuspell );
        }
        else if ( sectorSurtrFactionId == FactionId::Marauders )
        {
            g_pGame->AddIntel( GameCharacter::FleetIntelligence,
                "Our objective will be Surtr sector, where powerful communication relays have been deployed. "
                "These broadcast messages which have not been pre-approved by the Ministry of Reason. This "
                "not be allowed to continue." );

            pBlackboard->Add( sConquerSurtr );
        }
        else if ( sectorMuspellFactionId == FactionId::Marauders )
        {
            g_pGame->AddIntel( GameCharacter::FleetIntelligence,
                "Our objective will be Muspell sector, currently is guarded by a powerful battle station which "
                "provides them with an extremely strong position in that region of space." );

            pBlackboard->Add( sConquerMuspell );
        }

        pBlackboard->Add( sMarauderIntro );
    }

    if ( pBlackboard->Exists( sConquerSurtr ) && pBlackboard->Exists( sConquerSurtrCompleted ) == false && GoalExists( pSectorSurtr ) == false )
    {
        AddGoal( std::make_shared<RequestGoal>( pSectorSurtr, "Campaign: destroy broadcast relay", sCampaignGoalColour ) );
        SetThreatScore( GetShipThreatScore( "special_fimbulvetr", FactionId::Marauders ) );
    }
    else if ( pBlackboard->Exists( sConquerSurtrCompleted ) == false && pSectorSurtr->GetFaction()->GetFactionId() == FactionId::Empire )
    {
        pBlackboard->Add( sConquerSurtrCompleted );
    }
    else if ( pBlackboard->Exists( sConquerSurtrCompleted ) && GoalExists( pSectorSurtr ) )
    {
        RemoveGoal( pSectorSurtr );
    }

    if ( pBlackboard->Exists( sConquerMuspell ) && pBlackboard->Exists( sConquerMuspellCompleted ) == false && GoalExists( pSectorMuspell ) == false )
    {
        AddGoal( std::make_shared<RequestGoal>( pSectorMuspell, "Campaign: destroy battlestation", sCampaignGoalColour ) );
        SetThreatScore(
            GetShipThreatScore( "special_battlestation", FactionId::Marauders ) + GetShipThreatScore( "special_naglfar", FactionId::Marauders ) );
    }
    else if ( pBlackboard->Exists( sConquerMuspellCompleted ) == false && pSectorMuspell->GetFaction()->GetFactionId() == FactionId::Empire )
    {
        pBlackboard->Add( sConquerMuspellCompleted );
    }
    else if ( pBlackboard->Exists( sConquerMuspellCompleted ) && GoalExists( pSectorMuspell ) )
    {
        RemoveGoal( pSectorMuspell );
    }

    // Both Surtr and Muspell sectors have been conquered, the invasion begins.
    if ( pBlackboard->Exists( sConquerSurtrCompleted ) && pBlackboard->Exists( sConquerMuspellCompleted ) && pBlackboard->Exists( invasionTag ) == false )
    {
        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "Foreseeing our inevitable victory, the Marauders have launched an invasion across our Empire. We'll need our flagship present to drive them back, "
            "before can crush them once and for all." );

        pBlackboard->Add( invasionTag, 5 );
    }
    // If the invasion has been defeated, we can attack Valhalla
    else if ( pBlackboard->Exists( sConquerValhalla ) == false && pBlackboard->Exists( invasionTag ) && pBlackboard->Get( invasionTag ) == 0 )
    {
        pSectorValhalla->SetHyperspaceInhibitor( false );
        pBlackboard->Add( sConquerValhalla );
    }
    else if ( pBlackboard->Exists( sConquerValhalla ) && pBlackboard->Exists( sConquerValhallaCompleted ) == false && GoalExists( pSectorValhalla ) == false )
    {
        AddGoal( std::make_shared<RequestGoal>( pSectorValhalla, "Campaign: capture Valhalla", sCampaignGoalColour ) );
    }
    else if ( pBlackboard->Exists( sConquerValhalla ) && pSectorValhalla->GetFaction()->GetFactionId() == FactionId::Empire )
    {
        RemoveGoal( pSectorValhalla );
        pBlackboard->Add( sConquerValhallaCompleted );
        pBlackboard->Add( sMarauderArcFinished );

        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "The Marauders will no longer be a threat, Captain. With Valhalla's stronghold destroyed, we have mastery "
            "over what remains of their space. We will begin organising planetary landfalls." );

        g_pGame->GetAchievementsManager()->UnlockAchievement( ACH_MANAGED_FREEDOM );
    }
}

void CampaignRequest::UpdateAscentArc( float delta )
{
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();

    if ( pBlackboard->Exists( sMarauderArcFinished ) == false || pBlackboard->Exists( sAscentArcFinished ) )
    {
        return;
    }

    if ( pBlackboard->Exists( sAscentIntro ) == false )
    {
        g_pGame->AddIntel(
            GameCharacter::FleetIntelligence,
            "To fully reclaim the galaxy we will have to subjugate both the Ascent and the Iriani." );

        g_pGame->AddIntel(
            GameCharacter::FleetIntelligence,
            "The Iriani have an advanced hyperspace inhibitor technology which protects their space quite "
            "effectively but some research has highlighted an option worth pursuing." );

        g_pGame->AddIntel(
            GameCharacter::FleetIntelligence,
            "During the time of the Old Empire, we had mastered hyperspace technology. As we had created "
            "the inhibitors, we were also capable of disabling them in case any sector decided to rebel." );

        g_pGame->AddIntel(
            GameCharacter::FleetIntelligence,
            "This is what we seek, a device called Orion's Sword. Although the location of the research center "
            "is now lost to us, our spies tell us that the Ascent High Command know where it is." );

        g_pGame->AddIntel(
            GameCharacter::FleetIntelligence,
            "You will have to siege Solaris Secundus - since we've turned the star in Solaris Primaris supernova, "
            "this is where their HQ is these days." );

        g_pGame->AddIntel(
            GameCharacter::FleetIntelligence,
            "However, the more sectors they control around Solaris Secundus, the more reinforcements they will "
            "be able to field. The battle will be much easier if you can cut the sector off." );

        pBlackboard->Add( sAscentIntro );
        pBlackboard->Add( sConquerSolarisSecundus );
    }

    SectorInfo* pSectorSolarisSecundus = g_pGame->GetGalaxy()->GetSectorInfo( 15, 2 );
    SDL_assert( pSectorSolarisSecundus != nullptr );
    if ( pBlackboard->Exists( sConquerSolarisSecundus ) && pBlackboard->Exists( sConquerSolarisSecundusCompleted ) == false && GoalExists( pSectorSolarisSecundus ) == false )
    {
        AddGoal( std::make_shared<RequestGoal>( pSectorSolarisSecundus, "Campaign: capture Solaris Secundus", sCampaignGoalColour ) );
        SetThreatScore( 999999 ); // Overwhelming!
        pSectorSolarisSecundus->SetHyperspaceInhibitor( false );
    }
    else if ( pBlackboard->Exists( sConquerSolarisSecundusCompleted ) == false && pSectorSolarisSecundus->GetFaction()->GetFactionId() == FactionId::Empire )
    {
        pBlackboard->Add( sConquerSolarisSecundusCompleted );
    }
    else if ( pBlackboard->Exists( sConquerSolarisSecundusCompleted ) && GoalExists( pSectorSolarisSecundus ) )
    {
        RemoveGoal( pSectorSolarisSecundus );
    }
    else if ( pBlackboard->Exists( sConquerSolarisSecundusCompleted ) && pBlackboard->Exists( sAscentArcFinished ) == false )
    {
        pBlackboard->Add( sAscentArcFinished );
    }
}

void CampaignRequest::UpdateIrianiArc( float delta )
{
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();

    if ( pBlackboard->Exists( sChrysamereArcFinished ) == false || pBlackboard->Exists( sIrianiArcFinished ) )
    {
        return;
    }

    if ( pBlackboard->Exists( sIrianiIntro ) == false )
    {
        g_pGame->AddIntel(
            GameCharacter::FleetIntelligence,
            "With Orion's Sword, we'll be able to get into Iriani space and put an end to their expansion." );

        g_pGame->AddIntel(
            GameCharacter::FleetIntelligence,
            "Although we can now jump directly to Iriani Prime to lay siege to their home planet, it will be fiercely defended." );

        g_pGame->AddIntel(
            GameCharacter::FleetIntelligence,
            "The Iriani have specialised ships called Arbiters which can jump directly to the homeworld if it is attacked - we would do well to destroy them ahead of time." );

        pBlackboard->Add( sIrianiIntro );
        pBlackboard->Add( sConquerIrianiPrime );
    }

    SectorInfo* pSectorIrianiPrime = g_pGame->GetGalaxy()->GetSectorInfo( 20, 16 );
    SDL_assert( pSectorIrianiPrime != nullptr );
    if ( pBlackboard->Exists( sConquerIrianiPrime ) && pBlackboard->Exists( sConquerIrianiPrimeCompleted ) == false && GoalExists( pSectorIrianiPrime ) == false )
    {
        AddGoal( std::make_shared<RequestGoal>( pSectorIrianiPrime, "Campaign: conquer Iriani Prime", sCampaignGoalColour ) );
        SetThreatScore( 999999 ); // Overwhelming!
    }
    else if ( pBlackboard->Exists( sConquerIrianiPrimeCompleted ) == false && pSectorIrianiPrime->GetFaction()->GetFactionId() == FactionId::Empire )
    {
        // The player's ship doesn't count towards the fleet size, so we unlock it when the fleet size is 0.
        FleetSharedPtr pPlayerFleet = g_pGame->GetPlayerFleet().lock();
        if ( pPlayerFleet != nullptr && pPlayerFleet->GetShips().size() == 0 )
        {
            g_pGame->GetAchievementsManager()->UnlockAchievement( ACH_DEMIGOD );
        }

        pBlackboard->Add( sConquerIrianiPrimeCompleted );
    }
    else if ( pBlackboard->Exists( sConquerIrianiPrimeCompleted ) )
    {
        if ( GoalExists( pSectorIrianiPrime ) )
        {
            RemoveGoal( pSectorIrianiPrime );
        }

        pBlackboard->Add( sIrianiArcFinished );

        g_pGame->AddIntel(
            GameCharacter::FleetIntelligence,
            "This is it, Captain. The Iriani leadership is destroyed and the Arbiters are no more. In time, all that remains of the Iriani empire will fall to us." );
    }
}

void CampaignRequest::UpdateChrysamereArc( float delta )
{
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();

    if ( pBlackboard->Exists( sAscentArcFinished ) == false || pBlackboard->Exists( sChrysamereArcFinished ) )
    {
        return;
    }

    if ( pBlackboard->Exists( sChrysamereIntro ) == false )
    {
        g_pGame->AddIntel(
            GameCharacter::FleetIntelligence,
            "We've located where Orion's Sword is: the Cradle of Isaldren, a remote, anomalous sector in the outskirts of the galaxy." );

        g_pGame->AddIntel(
            GameCharacter::FleetIntelligence,
            "Records indicate that shields will barely work - if at all - in that region of space, due to the proximity of a black hole." );

        g_pGame->AddIntel(
            GameCharacter::Chrysamere,
            "Do not step into the Cradle. Vessels greater than yours have paid the ultimate price." );

        pBlackboard->Add( sChrysamereIntro );
        pBlackboard->Add( sEnterCradle );
    }

    SectorInfo* pSectorCradle = g_pGame->GetGalaxy()->GetSectorInfo( 21, 6 );
    SDL_assert( pSectorCradle != nullptr );
    if ( pBlackboard->Exists( sEnterCradle ) && pBlackboard->Exists( sEnterCradleCompleted ) == false && GoalExists( pSectorCradle ) == false )
    {
        AddGoal( std::make_shared<RequestGoal>( pSectorCradle, "Campaign: enter the Cradle of Isaldren", sCampaignGoalColour ) );
        pSectorCradle->SetFaction( g_pGame->GetFaction( FactionId::Special ), true, false );
        pSectorCradle->SetProceduralSpawning( false );
        pSectorCradle->SetShipyard( false );
        SetThreatScore( 999999 ); // Overwhelming!
    }
    else if ( pBlackboard->Exists( sEnterCradleCompleted ) )
    {
        if ( GoalExists( pSectorCradle ) )
        {
            RemoveGoal( pSectorCradle );
        }

        pBlackboard->Add( sChrysamereArcFinished );
    }
}

void CampaignRequest::ClearGoals( RequestGoalSharedPtrList& list )
{
    for ( auto& pGoal : list )
    {
        RemoveGoal( pGoal );
    }
    list.clear();
}

void CampaignRequest::OnPlayerEnterSector()
{
    ImperialRequest::OnPlayerEnterSector();

    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    if ( pBlackboard->Exists( sConquerValhalla ) && pBlackboard->Exists( sConquerValhallaCompleted ) == false )
    {
        const float anchorX = 0.0f;
        const float anchorY = 400.0f;
        SpawnShip( "special_hw_station", FactionId::Marauders, 0.0f, anchorY );

        const int steps = 12;
        const float theta = Genesis::kDegToRad * 360.0f / static_cast<float>( steps );
        const float radius = 300.0f;
        float angle = 0.0f;
        for ( int i = 0; i < steps; i++ )
        {
            SpawnShip( "special_hw_turret", FactionId::Marauders,
                anchorX + cosf( angle ) * radius,
                anchorY + sinf( angle ) * radius );

            angle += theta;
        }
    }
    else if ( pBlackboard->Exists( sConquerSurtr ) && pBlackboard->Exists( sConquerSurtrCompleted ) == false )
    {
        SpawnShip( "special_relay", FactionId::Marauders, 0.0f, 0.0f );
        SpawnShip( "special_relay", FactionId::Marauders, -400.0f, -400.0f );
        SpawnShip( "special_relay", FactionId::Marauders, 400.0f, -400.0f );
        SpawnShip( "special_fimbulvetr", FactionId::Marauders, -2000.0f, 0.0f );

        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "The Fimbulvetr has been detected in this sector. It is known to have a large torpedo battery, "
            "but a decent missile interceptor should all but neutralise it." );
    }
    else if ( pBlackboard->Exists( sConquerMuspell ) && pBlackboard->Exists( sConquerMuspellCompleted ) == false )
    {
        SpawnShip( "special_battlestation", FactionId::Marauders, 0.0f, 0.0f );
        SpawnShip( "special_naglfar", FactionId::Marauders, 0.0f, -500.0f );

        g_pGame->AddIntel( GameCharacter::FleetIntelligence,
            "The Naglfar is currently in this sector. From previous engagements we know that the frontal armour is "
            "extremely thick, our fleet will have to out-maneuver it." );
    }
    else if ( pBlackboard->Exists( sEnterCradle ) && pBlackboard->Exists( sEnterCradleCompleted ) == false )
    {
        SpawnShip( "special_relay", FactionId::Marauders, 0.0f, 0.0f );
    }
    else if ( pBlackboard->Exists( sConquerIrianiPrime ) && pBlackboard->Exists( sConquerIrianiPrimeCompleted ) == false )
    {
        SpawnShip( "special_flagship", FactionId::Iriani, 0.0f, 2.0f );
        SpawnShip( "special_maiden", FactionId::Iriani, -400.0f, 2.0f );
        SpawnShip( "special_maiden", FactionId::Iriani, 400.0f, 2.0f );

        g_pGame->AddIntel( GameCharacter::AeliseGloriam, "The end comes, Imperial." );
    }
}

void CampaignRequest::SpawnShip( const std::string& shipName, FactionId factionId, float x, float y )
{
    Faction* pFaction = g_pGame->GetFaction( factionId );
    const ShipInfo* pShipInfo = g_pGame->GetShipInfoManager()->Get( pFaction, shipName );
    assert( pShipInfo != nullptr );

    ShipCustomisationData customisationData( pShipInfo->GetModuleInfoHexGrid() );
    ShipSpawnData spawnData( x, y );
    Sector* pSector = g_pGame->GetCurrentSector();

    Ship* pShip = new Ship();
    pShip->SetInitialisationParameters(
        pFaction,
        pSector->GetRegionalFleet(),
        customisationData,
        spawnData,
        pShipInfo );

    pShip->Initialize();
    pSector->AddShip( pShip );
}

} // namespace Hexterminate
