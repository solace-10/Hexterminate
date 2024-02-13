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

#include <imgui/imgui.h>

#include <genesis.h>
#include <logger.h>
#include <memory.h>
#include <xml.h>

#include "achievements.h"
#include "hexterminate.h"
#include "imgui/imgui_impl.h"
#include "menus/galaxywindow.h"
#include "player.h"
#include "requests/campaigntags.h"
#include "sector/fogofwar.h"
#include "sector/galaxy.h"
#include "sector/galaxycreationinfo.h"
#include "sector/galaxygenerator.h"
#include "sector/galaxyrep.h"
#include "sector/sector.h"
#include "xmlaux.h"

namespace Hexterminate
{

Galaxy::Galaxy()
    : m_Initialised( false )
    , m_pRep( nullptr )
    , m_Compression( 0.0f )
    , m_DebugWindowOpen( false )
    , m_pFogOfWar( nullptr )
{
    for ( int x = 0; x < NumSectorsX; x++ )
    {
        for ( int y = 0; y < NumSectorsY; ++y )
        {
            m_Sectors[ x ][ y ] = nullptr;
        }
    }

    m_pRep = new GalaxyRep( this );
    m_pRep->Initialise();

    Genesis::ImGuiImpl::RegisterMenu( "Game", "Galaxy", &m_DebugWindowOpen );
}

Galaxy::~Galaxy()
{
    Reset();

    m_pRep->RemoveFromScene();
    delete m_pRep;

    Genesis::ImGuiImpl::UnregisterMenu( "Game", "Galaxy" );
}

void Galaxy::Reset()
{
    for ( int x = 0; x < NumSectorsX; x++ )
    {
        for ( int y = 0; y < NumSectorsY; ++y )
        {
            if ( m_Sectors[ x ][ y ] != nullptr )
            {
                delete m_Sectors[ x ][ y ];
                m_Sectors[ x ][ y ] = nullptr;
            }
        }
    }

    m_Initialised = false;

    if ( m_pFogOfWar != nullptr )
    {
        delete m_pFogOfWar;
        m_pFogOfWar = nullptr;
    }

    m_pRep->OnGalaxyReset();
}

void Galaxy::Create( const GalaxyCreationInfo& creationInfo )
{
    using namespace tinyxml2;
    using namespace Genesis;

    const GalaxyCreationInfo::CreationMode mode = creationInfo.GetMode();
    if ( mode == GalaxyCreationInfo::CreationMode::Campaign || mode == GalaxyCreationInfo::CreationMode::InfiniteWar )
    {
        std::string filename;
        if ( mode == GalaxyCreationInfo::CreationMode::Campaign )
        {
            filename = "data/xml/sectors/campaign.xml";
        }
        else if ( mode == GalaxyCreationInfo::CreationMode::InfiniteWar )
        {
            filename = "data/xml/sectors/infinitewar.xml";
        }

        tinyxml2::XMLDocument sectorsFile;
        if ( sectorsFile.LoadFile( filename.c_str() ) == XML_SUCCESS )
        {
            XMLElement* pRootElement = sectorsFile.FirstChildElement();
            for ( XMLElement* pElement = pRootElement->FirstChildElement(); pElement != nullptr; pElement = pElement->NextSiblingElement() )
            {
                const std::string value( pElement->Value() );
                if ( value == "Sector" )
                {
                    SectorInfo* pSectorInfo = new SectorInfo();
                    if ( pSectorInfo->Read( pElement ) )
                    {
                        // When we first load a sector, set up its regional fleet normally.
                        // This can then be overridden after all the sectors are loaded to make sectors easier than the default.
                        pSectorInfo->SetupRegionalFleet( false );

                        const glm::ivec2& coordinates = pSectorInfo->GetCoordinates();
                        delete m_Sectors[ coordinates.x ][ coordinates.y ];
                        m_Sectors[ coordinates.x ][ coordinates.y ] = pSectorInfo;
                    }
                    else
                    {
                        FrameWork::GetLogger()->LogWarning( "Couldn't load sector %d / %d", pSectorInfo->GetCoordinates().x, pSectorInfo->GetCoordinates().y );
                    }
                }
            }
        }
        else
        {
            FrameWork::GetLogger()->LogError( "Failed to load sectors file: '%s'.", filename.c_str() );
        }

        if ( mode == GalaxyCreationInfo::CreationMode::Campaign )
        {
            // Find all the sectors bordering the Empire and make them easier. This gives the player a more accessible start.
            Faction* pEmpireFaction = g_pGame->GetFaction( FactionId::Empire );
            const SectorInfoVector& imperialSectors = pEmpireFaction->GetControlledSectors();
            for ( auto& pSectorInfo : imperialSectors )
            {
                SectorInfoVector borderingSectors;
                pSectorInfo->GetBorderingSectors( borderingSectors );
                for ( auto& pBorderingSectorInfo : borderingSectors )
                {
                    if ( pBorderingSectorInfo->GetFaction() != pEmpireFaction )
                    {
                        pBorderingSectorInfo->SetupRegionalFleet( true );
                    }
                }
            }
        }
        else if ( mode == GalaxyCreationInfo::CreationMode::InfiniteWar )
        {
            GenerateProceduralGalaxy( creationInfo );
            m_pFogOfWar = new FogOfWar();
        }
    }

    m_Initialised = true;

    m_pRep->OnGalaxyPopulated();
}

void Galaxy::GenerateProceduralGalaxy( const GalaxyCreationInfo& creationInfo )
{
    GalaxyGenerator generator;
    generator.Run( this, creationInfo );
}

void Galaxy::ForceNextTurn()
{
    if ( m_Initialised )
    {
        for ( int i = 0; i < (int)FactionId::Count; ++i )
        {
            g_pGame->GetFaction( (FactionId)i )->ForceNextTurn();
        }
    }
}

void Galaxy::CalculateCompression()
{
    GameState state = g_pGame->GetState();

    if ( m_Initialised == false || g_pGame->IsPaused() )
        m_Compression = 0.0f;
    else if ( state == GameState::GalaxyView && g_pGame->IsTutorialActive() )
        m_Compression = 0.0f;
    else if ( state == GameState::GalaxyView ) // Galaxy ticks at the normal speed only when we are in Galaxy View
        m_Compression = 1.0f;
    else if ( state == GameState::Combat || state == GameState::Shipyard ) // If we are in combat or customising our ship, it ticks slower
        m_Compression = GalaxyTimeCompression;
    else // In menus or in any other screen, the Galaxy is frozen and doesn't tick at all
        m_Compression = 0.0f;
}

void Galaxy::Update( float delta )
{
    if ( m_Initialised )
    {
        CalculateCompression();

        float compressedDelta = delta;
        if ( GetCompression() <= FLT_MIN )
            compressedDelta = 0.0f;
        else
            compressedDelta /= GetCompression();

        Show( g_pGame->GetState() == GameState::GalaxyView );

        if ( g_pGame->GetGameMode() == GameMode::InfiniteWar && m_pFogOfWar == nullptr )
        {
            m_pFogOfWar = new FogOfWar();
        }

        if ( m_pFogOfWar != nullptr )
        {
            for ( int x = 0; x < NumSectorsX; ++x )
            {
                for ( int y = 0; y < NumSectorsY; ++y )
                {
                    if ( m_Sectors[ x ][ y ]->HasProbe() )
                    {
                        m_pFogOfWar->MarkAsVisible( m_Sectors[ x ][ y ], 2 );
                    }
                }
            }

            m_pFogOfWar->Update( compressedDelta );
        }

        for ( int i = 0; i < (int)FactionId::Count; ++i )
        {
            g_pGame->GetFaction( (FactionId)i )->Update( compressedDelta );
        }

        for ( int i = 0; i < (int)FactionId::Count; ++i )
        {
            g_pGame->GetFaction( (FactionId)i )->PostUpdate();
        }
    }

    if ( m_pRep && g_pGame->GetState() == GameState::GalaxyView )
    {
        m_pRep->Update( delta );
        EndGameCheck();
    }

    UpdateDebugUI();
}

void Galaxy::EndGameCheck()
{
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    const GameMode gameMode = g_pGame->GetGameMode();
    if ( gameMode == GameMode::Campaign )
    {
        if ( pBlackboard->Exists( sGameEnd ) == false && pBlackboard->Exists( sExpansionArcFinished ) && pBlackboard->Exists( sPirateArcFinished ) && pBlackboard->Exists( sMarauderArcFinished ) && pBlackboard->Exists( sAscentArcFinished ) && pBlackboard->Exists( sChrysamereArcFinished ) && pBlackboard->Exists( sIrianiArcFinished ) )
        {
            m_pRep->GetGalaxyWindow()->ShowEndGameWindow();
            pBlackboard->Add( sGameEnd );
            g_pGame->GetAchievementsManager()->UnlockAchievement( ACH_BOOT_STAMPING );

            if ( g_pGame->GetDifficulty() == Difficulty::Hardcore )
            {
                g_pGame->GetAchievementsManager()->UnlockAchievement( ACH_HEXTERMINATED );
            }
        }
    }
    else if ( gameMode == GameMode::InfiniteWar )
    {
        if ( pBlackboard->Exists( sGameEnd ) == false && pBlackboard->Exists( "#infinity_war_finished" ) )
        {
            m_pRep->GetGalaxyWindow()->ShowEndGameWindow();
            pBlackboard->Add( sGameEnd );
        }
    }
}

void Galaxy::Show( bool state )
{
    if ( m_pRep )
    {
        m_pRep->Show( state );
    }
}

bool Galaxy::IsVisible() const
{
    return ( m_pRep && m_pRep->IsVisible() );
}

bool Galaxy::Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement )
{
    bool state = true;

    tinyxml2::XMLElement* pGalaxyElement = xmlDoc.NewElement( "Galaxy" );
    pRootElement->LinkEndChild( pGalaxyElement );

    for ( int x = 0; x < NumSectorsX; ++x )
    {
        for ( int y = 0; y < NumSectorsY; ++y )
        {
            state &= m_Sectors[ x ][ y ]->Write( xmlDoc, pGalaxyElement );
        }
    }

    return state;
}

bool Galaxy::Read( tinyxml2::XMLElement* pRootElement )
{
    int version = 1;

    for ( tinyxml2::XMLElement* pSectorElement = pRootElement->FirstChildElement(); pSectorElement != nullptr; pSectorElement = pSectorElement->NextSiblingElement() )
    {
        Xml::Serialise( pSectorElement, "Version", version );

        if ( std::string( pSectorElement->Value() ) == "Sector" )
        {
            SectorInfo* pSectorInfo = new SectorInfo();
            if ( pSectorInfo->Read( pSectorElement ) )
            {
                const glm::ivec2& coordinates = pSectorInfo->GetCoordinates();
                delete m_Sectors[ coordinates.x ][ coordinates.y ];
                m_Sectors[ coordinates.x ][ coordinates.y ] = pSectorInfo;
            }
        }
    }

    if ( version != GetVersion() )
    {
        UpgradeFromVersion( version );
    }

    return true;
}

void Galaxy::UpdateDebugUI()
{
#ifdef _DEBUG
    if ( m_DebugWindowOpen )
    {
        ImGui::SetNextWindowSize( ImVec2( 600.0f, 800.0f ) );
        ImGui::Begin( "Galaxy", &m_DebugWindowOpen );

        ImGui::Text( "Factions" );

        for ( int i = 0; i < (int)FactionId::Count; ++i )
        {
            Faction* pFaction = g_pGame->GetFaction( (FactionId)i );
            if ( ImGui::CollapsingHeader( pFaction->GetName().c_str() ) )
            {
                ImGui::Columns( 2 );

                ImGui::Text( "Collapsing" );
                ImGui::NextColumn();
                ImGui::Text( "%s", pFaction->IsCollapsing() ? "True" : "False" );
                ImGui::NextColumn();

                ImGui::Text( "Sectors" );
                ImGui::NextColumn();
                ImGui::Text( "%zu", pFaction->GetControlledSectors().size() );
                ImGui::NextColumn();

                ImGui::Text( "Fleets" );
                ImGui::NextColumn();
                ImGui::Text( "%zu", pFaction->GetFleets().size() );
                ImGui::NextColumn();

                ImGui::Text( "Fleets in construction" );
                ImGui::NextColumn();
                ImGui::Text( "%zu", pFaction->GetFleetsInConstruction().size() );
                ImGui::NextColumn();

                const auto& sectors = pFaction->GetControlledSectors();
                int numShipyards = 0;
                for ( const SectorInfo* pSectorInfo : sectors )
                {
                    if ( pSectorInfo->HasShipyard() )
                    {
                        numShipyards++;
                    }
                }

                ImGui::Text( "Shipyards" );
                ImGui::NextColumn();
                ImGui::Text( "%d", numShipyards );

                ImGui::Columns( 1 );
            }
        }

        ImGui::Text( "Cheats" );
        Player* pPlayer = g_pGame->GetPlayer();
        if ( ImGui::Button( "Add 100 perk points" ) )
        {
            pPlayer->SetPerkPoints( pPlayer->GetPerkPoints() + 100 );
        }

        if ( ImGui::Button( "Add 10000 influence" ) )
        {
            pPlayer->SetInfluence( pPlayer->GetInfluence() + 10000 );
        }
        ImGui::End();
    }
#endif
}

void Galaxy::UpgradeFromVersion( int version )
{
    Genesis::FrameWork::GetLogger()->LogInfo( "Galaxy::UpgradeFromVersion(): %d -> %d", version, GetVersion() );

    // In 0.11.4 new components were added to two sectors.
    if ( version == 1 )
    {
        SectorInfo* pSolarisSecundusSector = m_Sectors[ 15 ][ 2 ];
        pSolarisSecundusSector->AddComponentName( "AnchorComponent" );
        pSolarisSecundusSector->AddComponentName( "ReinforcementsComponent" );

        SectorInfo* pIrianiPrimeSector = m_Sectors[ 20 ][ 16 ];
        pIrianiPrimeSector->AddComponentName( "ReinforcementsComponent" );

        version++;
    }

    if ( version == 2 )
    {
        SectorInfo* pIrianiPrimeSector = m_Sectors[ 20 ][ 16 ];
        pIrianiPrimeSector->AddComponentName( "ArbiterReinforcementComponent" );
        version++;
    }

    if ( version == 3 )
    {
        // This was due to a mismatched enum, where an Iriani invasion would get tagged.
        // The invasion has no backing data files and was preventing the campaign from
        // progressing. The num has been fixed, but the tag needs to be removed so no
        // events are accidentally spawned.
        if ( g_pGame->GetBlackboard()->Exists( "#invasion_iriani" ) )
        {
            g_pGame->GetBlackboard()->Add( "#invasion_iriani", 0 );
        }
        version++;
    }

    if ( version == 4 )
    {
        // It is possible for the Cradle to be conquered by anothe faction, and for that
        // faction to then establish a shipyard. If this happens and then the player
        // progresses sufficiently in the campaign as for the Cradle to be claimed by
        // the Chrysamere (Special) faction, the game will crash to desktop when the
        // player attempts to enter the sector as that faction has no turret prototype
        // to defend the shipyard with.
        SectorInfo* pSectorCradle = g_pGame->GetGalaxy()->GetSectorInfo( 21, 6 );
        pSectorCradle->SetShipyard( false );
    }
}

} // namespace Hexterminate