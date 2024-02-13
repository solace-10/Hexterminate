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

#include <sstream>
#include <string>

#include <configuration.h>
#include <genesis.h>
#include <inputmanager.h>
#include <math/misc.h>
#include <rendersystem.h>
#include <resources/resourceimage.h>
#include <scene/layer.h>
#include <scene/scene.h>
#include <shadercache.h>
#include <shaderuniform.h>
#include <vertexbuffer.h>

#include "drawutils.h"
#include "faction/empirefaction.h"
#include "faction/faction.h"
#include "fleet/fleet.h"
#include "globals.h"
#include "hexterminate.h"
#include "menus/eva.h"
#include "menus/galaxywindow.h"
#include "menus/popup.h"
#include "menus/sectordetails.h"
#include "misc/gui.h"
#include "requests/imperialrequest.h"
#include "requests/requestgoal.h"
#include "requests/requestgoalrep.h"
#include "requests/requestmanager.h"
#include "sector/fogofwar.h"
#include "sector/galaxy.h"
#include "sector/galaxyrep.h"
#include "sector/sector.h"
#include "ui/rootelement.h"

namespace Hexterminate
{

/////////////////////////////////////////////////////////////////////
// GalaxyRep
/////////////////////////////////////////////////////////////////////

GalaxyRep::GalaxyRep( Galaxy* pGalaxy )
    : m_pGalaxy( pGalaxy )
    , m_Show( false )
    , m_pBackgroundShader( nullptr )
    , m_pBackgroundVB( nullptr )
    , m_pSectorShader( nullptr )
    , m_pSectorShipyardShader( nullptr )
    , m_pSectorInhibitorShader( nullptr )
    , m_pSectorHomeworldShader( nullptr )
    , m_pSectorVB( nullptr )
    , m_pSectorInhibitorVB( nullptr )
    , m_pSectorHomeworldVB( nullptr )
    , m_pSectorCrossShader( nullptr )
    , m_pSectorCrossVB( nullptr )
    , m_pSectorHomeworldDiffuseSampler( nullptr )
    , m_pSectorThreatShader( nullptr )
    , m_pGoalTargetVB( nullptr )
    , m_pLayer( nullptr )
    , m_OffsetX( 0.0f )
    , m_OffsetY( 0.0f )
    , m_pHoverSector( nullptr )
    , m_pSectorDetails( nullptr )
    , m_ExitMenu( false )
    , m_pGalaxyWindow( nullptr )
    , m_InputPending( false )
    , m_InputTimer( 0u )
{
    using namespace Genesis;

    // The visual size of the galaxy is dependent on the screen resolution and on GalaxyMinSize, whichever is larger.
    m_Size = glm::max( glm::vec2( GalaxyMinSize ), glm::max( static_cast<float>( Configuration::GetScreenWidth() ), static_cast<float>( Configuration::GetScreenHeight() ) ) );

    ResourceManager* pRm = FrameWork::GetResourceManager();
    ResourceImage* pBackgroundImage = static_cast<ResourceImage*>( pRm->GetResource( "data/backgrounds/galaxy.png" ) );
    ResourceImage* pBackgroundLine = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/line.png" ) );

    m_pBackgroundShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "galaxy" );
    ShaderUniform* pBackgroundDiffuseSampler = m_pBackgroundShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    pBackgroundDiffuseSampler->Set( pBackgroundImage, GL_TEXTURE0 );
    ShaderUniform* pLineSampler = m_pBackgroundShader->RegisterUniform( "lineSampler", ShaderUniformType::Texture );
    pLineSampler->Set( pBackgroundLine, GL_TEXTURE1 );
    ShaderUniform* pScreenWidth = m_pBackgroundShader->RegisterUniform( "k_screenWidth", ShaderUniformType::Float );
    pScreenWidth->Set( (float)Configuration::GetScreenWidth() );
    ShaderUniform* pScreenHeight = m_pBackgroundShader->RegisterUniform( "k_screenHeight", ShaderUniformType::Float );
    pScreenHeight->Set( (float)Configuration::GetScreenHeight() );

    m_pBackgroundVB = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV );
    m_pSectorShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "sector" );

    ResourceImage* pSectorShipyardImage = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/shipyard.png" ) );
    m_pSectorShipyardShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "sector_shipyard" );
    ShaderUniform* pSectorShipyardDiffuseSampler = m_pSectorShipyardShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    pSectorShipyardDiffuseSampler->Set( pSectorShipyardImage, GL_TEXTURE0 );

    ResourceImage* pSectorProbeImage = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/probe.png" ) );
    m_pSectorProbeShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "sector_probe" );
    ShaderUniform* pSectorProbeDiffuseSampler = m_pSectorProbeShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    pSectorProbeDiffuseSampler->Set( pSectorProbeImage, GL_TEXTURE0 );

    ResourceImage* pSectorStarfortImage = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/starfort.png" ) );
    m_pSectorStarfortShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "sector_starfort" );
    ShaderUniform* pSectorStarfortDiffuseSampler = m_pSectorStarfortShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    pSectorStarfortDiffuseSampler->Set( pSectorStarfortImage, GL_TEXTURE0 );

    ResourceImage* pSectorInhibitorImage = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/hyperspace_inhibitor.png" ) );
    m_pSectorInhibitorShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "sector_inhibitor" );
    ShaderUniform* pSectorInhibitorDiffuseSampler = m_pSectorInhibitorShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    pSectorInhibitorDiffuseSampler->Set( pSectorInhibitorImage, GL_TEXTURE0 );

    m_pSectorHomeworldShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "sector_homeworld" );
    m_pSectorHomeworldDiffuseSampler = m_pSectorHomeworldShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );

    m_HomeworldImages[ (int)FactionId::Neutral ] = nullptr;
    m_HomeworldImages[ (int)FactionId::Player ] = nullptr;
    m_HomeworldImages[ (int)FactionId::Empire ] = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/homeworld/empire.png" ) );
    m_HomeworldImages[ (int)FactionId::Ascent ] = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/homeworld/ascent.png" ) );
    m_HomeworldImages[ (int)FactionId::Pirate ] = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/homeworld/pirates.png" ) );
    m_HomeworldImages[ (int)FactionId::Marauders ] = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/homeworld/marauders.png" ) );
    m_HomeworldImages[ (int)FactionId::Iriani ] = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/homeworld/iriani.png" ) );
    m_HomeworldImages[ (int)FactionId::Special ] = nullptr;
    m_HomeworldImages[ (int)FactionId::Hegemon ] = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/homeworld/hegemon.png" ) );

    m_pSectorVB = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );
    m_pSectorInhibitorVB = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );
    m_pSectorHomeworldVB = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV );

    ResourceImage* pSectorCrossImage = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/sector_cross.png" ) );
    m_pSectorCrossShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "diffuse_alpha" );
    ShaderUniformInstance sectorCrossSamplerUniform = m_pSectorCrossShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    sectorCrossSamplerUniform.Set( pSectorCrossImage, GL_TEXTURE0 );
    m_SectorCrossUniforms.push_back( sectorCrossSamplerUniform );

    m_pSectorCrossVB = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV );

    m_pSectorThreatShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "sector_threat" );

    std::array<ResourceImage*, static_cast<size_t>( ThreatRating::Count )> threatImages;
    threatImages[ static_cast<size_t>( ThreatRating::None ) ] = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/threat_none.png" ) );
    threatImages[ static_cast<size_t>( ThreatRating::Trivial ) ] = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/threat_trivial.png" ) );
    threatImages[ static_cast<size_t>( ThreatRating::Easy ) ] = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/threat_easy.png" ) );
    threatImages[ static_cast<size_t>( ThreatRating::Fair ) ] = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/threat_fair.png" ) );
    threatImages[ static_cast<size_t>( ThreatRating::Challenging ) ] = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/threat_challenging.png" ) );
    threatImages[ static_cast<size_t>( ThreatRating::Overpowering ) ] = static_cast<ResourceImage*>( pRm->GetResource( "data/ui/sector/threat_overpowering.png" ) );

    for ( size_t i = 0; i < static_cast<size_t>( ThreatRating::Count ); ++i )
    {
        ResourceImage* pImage = threatImages[ i ];
        ShaderUniformInstance uniform = m_pSectorThreatShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
        uniform.Set( pImage, GL_TEXTURE0 );
        m_pSectorThreatUniforms[ i ].push_back( uniform );
    }

    m_LeftMouseButtonDownToken = FrameWork::GetInputManager()->AddMouseCallback( std::bind( &GalaxyRep::OnLeftMouseButtonDown, this ), MouseButton::Left, ButtonState::Pressed );
}

GalaxyRep::~GalaxyRep()
{
    if ( Genesis::FrameWork::GetInputManager() != nullptr )
    {
        Genesis::FrameWork::GetInputManager()->RemoveMouseCallback( m_LeftMouseButtonDownToken );
    }

    delete m_pSectorDetails;
    delete m_pBackgroundVB;
    delete m_pSectorVB;
    delete m_pSectorInhibitorVB;
    delete m_pSectorHomeworldVB;
    delete m_pSectorCrossVB;
    delete m_pGoalTargetVB;
}

void GalaxyRep::Initialise()
{
    Genesis::Scene* pScene = Genesis::FrameWork::GetScene();
    m_pLayer = pScene->AddLayer( LAYER_GALAXY, true );
    m_pLayer->AddSceneObject( this, false );

    m_pGalaxyWindow = std::make_shared<GalaxyWindow>();
    g_pGame->GetUIRoot()->Add( m_pGalaxyWindow );
    m_pGalaxyWindow->Show( false );
}

void GalaxyRep::OnGalaxyPopulated()
{
    SDL_assert( m_pSectorDetails == nullptr );
    m_pSectorDetails = new SectorDetails();
    m_pSectorDetails->Show( false );
}

void GalaxyRep::OnGalaxyReset()
{
    delete m_pSectorDetails;
    m_pSectorDetails = nullptr;

    m_pGalaxyWindow->Show( false );
}

GalaxyWindow* GalaxyRep::GetGalaxyWindow() const
{
    return m_pGalaxyWindow.get();
}

void GalaxyRep::RemoveFromScene()
{
    m_pLayer->RemoveSceneObject( this );
}

void GalaxyRep::Update( float delta )
{
    if ( m_Show == false || m_pGalaxy == nullptr )
        return;

    if ( m_pGalaxy->IsInitialised() == false )
        return;

    FocusOnPlayerFleet();
    SetHoverSector();
    UpdateDrawInfo();

    m_pGalaxyWindow->Update();

    if ( m_pSectorDetails != nullptr )
    {
        m_pSectorDetails->Update( delta );
    }

    UpdateInput();

    if ( m_ExitMenu )
    {
        PopupState state = g_pGame->GetPopup()->GetState();
        if ( state == PopupState::Yes )
        {
            m_ExitMenu = false;
            g_pGame->EndGame();
        }
        else if ( state == PopupState::No )
        {
            m_pGalaxyWindow->Show( true );
            m_ExitMenu = false;
        }
    }
}

void GalaxyRep::UpdateInput()
{
    if ( Genesis::FrameWork::GetInputManager()->IsButtonPressed( SDL_SCANCODE_ESCAPE ) )
    {
        m_pGalaxyWindow->Show( false );
        m_ExitMenu = true;
        g_pGame->GetPopup()->Show( PopupMode::YesNo, "Exit to main menu?" );
    }

    if ( m_InputPending )
    {
        uint32_t ticks = SDL_GetTicks();
        if ( ticks > m_InputTimer && ticks < m_InputTimer + 1000 )
        {
            m_InputPending = false;
            FleetSharedPtr pPlayerFleet = g_pGame->GetPlayerFleet().lock();
            if ( pPlayerFleet != nullptr && pPlayerFleet->IsEngaged() == false )
            {
                bool acceptsInput = true;
                acceptsInput &= ( g_pGame->IsInputBlocked() == false );
                acceptsInput &= ( m_pGalaxy->GetCompression() > 0.0f );
                acceptsInput &= ( g_pGame->GetPopup()->IsActive() == false );
                acceptsInput &= ( g_pGame->IsTutorialActive() == false );
                acceptsInput &= ( GetGalaxyWindow()->HasSubWindowsVisible() == false );

                if ( acceptsInput )
                {
                    const glm::vec2& mousePos = Genesis::FrameWork::GetInputManager()->GetMousePosition();
                    pPlayerFleet->SetDestination( ( mousePos.x - m_OffsetX ) / m_Size.x, ( mousePos.y - m_OffsetY ) / m_Size.y );
                }
            }
        }
        else if ( ticks >= m_InputTimer + 1000 )
        {
            m_InputPending = false;
        }
    }
}

void GalaxyRep::UpdateDrawInfo()
{
    UpdateGoalDrawInfo();

    m_SectorDrawInfo.clear();
    m_SectorDrawInfoShipyard.clear();
    m_SectorDrawInfoProbes.clear();
    m_SectorDrawInfoStarforts.clear();
    m_SectorDrawInfoInhibitors.clear();

    FogOfWar* pFogOfWar = m_pGalaxy->GetFogOfWar();

    for ( int x = 0; x < NumSectorsX; x++ )
    {
        for ( int y = 0; y < NumSectorsY; y++ )
        {
            SectorInfo* pSectorInfo = m_pGalaxy->GetSectorInfo( x, y );
            Faction* pFaction = pSectorInfo->GetFaction();

            SectorDrawInfo drawInfo;
            drawInfo.x = x;
            drawInfo.y = y;

            bool drawIfNeutral = false;
            if ( pFogOfWar == nullptr || pFogOfWar->IsVisible( pSectorInfo ) )
            {
                if ( pSectorInfo->HasShipyard() )
                {
                    m_SectorDrawInfoShipyard.push_back( drawInfo );
                    drawIfNeutral = true;
                }

                if ( pSectorInfo->HasProbe() )
                {
                    m_SectorDrawInfoProbes.push_back( drawInfo );
                    drawIfNeutral = true;
                }

                if ( pSectorInfo->HasStarfort() )
                {
                    m_SectorDrawInfoStarforts.push_back( drawInfo );
                    drawIfNeutral = true;
                }

                if ( pSectorInfo->HasHyperspaceInhibitor() )
                {
                    m_SectorDrawInfoInhibitors.push_back( drawInfo );
                    drawIfNeutral = true;
                }

                if ( drawIfNeutral == false && pFaction == g_pGame->GetFaction( FactionId::Neutral ) )
                {
                    continue;
                }

                drawInfo.colour = pFaction->GetColour( FactionColourId::Base );
                drawInfo.colour.a = 0.3f;
            }
            else
            {
                drawInfo.colour = Genesis::Color( 0.0f, 0.0f, 0.0f, 0.6f );
            }

            m_SectorDrawInfo.push_back( drawInfo );
        }
    }
}

void GalaxyRep::UpdateGoalDrawInfo()
{
    Faction* pFaction = g_pGame->GetFaction( FactionId::Empire );
    if ( pFaction == nullptr )
    {
        return;
    }

    EmpireFaction* pEmpireFaction = (EmpireFaction*)pFaction;
    RequestManager* pRequestManager = pEmpireFaction->GetRequestManager();
    for ( auto& pRequest : pRequestManager->GetRequests() )
    {
        for ( auto& pGoal : pRequest->GetGoals() )
        {
            RequestGoalRepSharedPtr goalRep = pGoal->GetRepresentation();
            bool goalVisible = ( pGoal->GetSectorInfo() == nullptr ) || ( m_pSectorDetails->GetSectorInfo() != pGoal->GetSectorInfo() );
            goalVisible &= ( GetGalaxyWindow()->HasSubWindowsVisible() == false );
            goalVisible &= ( m_pGalaxy->GetCompression() > 0.0f );
            goalVisible &= ( g_pGame->GetPopup()->IsActive() == false );
            goalVisible &= ( g_pGame->IsTutorialActive() == false );

            goalRep->Show( goalVisible );
            goalRep->Update( this );
        }
    }
}

void GalaxyRep::FocusOnPlayerFleet()
{
    m_OffsetX = m_OffsetY = 0.0f;

    Faction* pPlayerFaction = g_pGame->GetPlayerFaction();
    if ( pPlayerFaction )
    {
        const FleetList& fleets = pPlayerFaction->GetFleets();
        if ( fleets.empty() == false )
        {
            Fleet* pPlayerFleet = fleets.back().get();
            SDL_assert( pPlayerFleet != nullptr );

            glm::vec2 screenSizeNormalised = glm::vec2(
                (float)Genesis::Configuration::GetScreenWidth() / m_Size.x,
                (float)Genesis::Configuration::GetScreenHeight() / m_Size.y );

            glm::vec2 halfScreenSizeNormalised;
            halfScreenSizeNormalised = screenSizeNormalised * 0.5f;

            glm::vec2 fleetPosition = pPlayerFleet->GetPosition();

            // Constrain the scrolling to the galaxy area
            if ( fleetPosition.x > halfScreenSizeNormalised.x )
                m_OffsetX = halfScreenSizeNormalised.x - fleetPosition.x;
            if ( fleetPosition.x > ( 1.0f - halfScreenSizeNormalised.x ) )
                m_OffsetX = -( 1.0f - screenSizeNormalised.x );
            if ( fleetPosition.y > halfScreenSizeNormalised.y )
                m_OffsetY = halfScreenSizeNormalised.y - fleetPosition.y;
            if ( fleetPosition.y > ( 1.0f - halfScreenSizeNormalised.y ) )
                m_OffsetY = -( 1.0f - screenSizeNormalised.y );

            m_OffsetX *= m_Size.x;
            m_OffsetY *= m_Size.y;
        }
    }
}

void GalaxyRep::SetHoverSector()
{
    using namespace Genesis;

    if ( g_pGame->GetPopup()->IsActive() || GetGalaxyWindow()->HasSubWindowsVisible() )
    {
        m_pSectorDetails->Show( false );
        return;
    }

    m_pHoverSector = nullptr;
    Faction* pPlayerFaction = g_pGame->GetPlayerFaction();
    if ( pPlayerFaction )
    {
        const FleetList& fleets = pPlayerFaction->GetFleets();
        if ( fleets.empty() == false )
        {
            Fleet* pFleet = fleets.back().get();
            if ( pFleet != nullptr && ( pFleet->HasArrived() || pFleet->IsIdle() ) && g_pGame->IsTutorialActive() == false )
            {
                SectorInfo* pCurrentSector = pFleet->GetCurrentSector();
                m_pHoverSector = pCurrentSector;
                m_pSectorDetails->SetSectorInfo( pCurrentSector );
                int sectorX, sectorY;
                pCurrentSector->GetCoordinates( sectorX, sectorY );
                const float posX = (float)sectorX / (float)NumSectorsX * m_Size.x + m_OffsetX;
                const float posY = (float)sectorY / (float)NumSectorsY * m_Size.y + m_OffsetY;
                m_pSectorDetails->SetAnchor( floor( posX ), floor( posY ) );
                m_pSectorDetails->Show( true );
            }
            else
            {
                m_pSectorDetails->SetSectorInfo( nullptr );
                m_pSectorDetails->Show( false );
            }
        }
    }
}

void GalaxyRep::Render()
{
    using namespace Genesis;

    if ( m_Show == false || m_pGalaxy == nullptr )
        return;

    DrawBackground();

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Blend );

    if ( m_pGalaxy->IsInitialised() )
    {
        DrawSectors( m_SectorDrawInfo, m_pSectorShader, nullptr, true );

        FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Add );
        DrawSectors( m_SectorDrawInfoInhibitors, m_pSectorInhibitorShader, nullptr, true );
        FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Blend );

        DrawSectors( m_SectorDrawInfoShipyard, m_pSectorShipyardShader, nullptr, false );
        DrawSectors( m_SectorDrawInfoProbes, m_pSectorProbeShader, nullptr, false );
        DrawSectors( m_SectorDrawInfoStarforts, m_pSectorStarfortShader, nullptr, false );
        DrawHomeworldSectors();
        DrawSectorsThreatRatings();

        FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Blend );
    }

    DrawGrid();
    DrawGoals();

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Disabled );
}

void GalaxyRep::DrawBackground()
{
    using namespace Genesis;

    m_pBackgroundVB->CreateTexturedQuad( m_OffsetX, m_OffsetY, m_Size.x, m_Size.y );
    m_pBackgroundShader->Use();
    m_pBackgroundVB->Draw();
}

void GalaxyRep::DrawHomeworldSectors()
{
    using namespace Genesis;

    if ( g_pGame->GetGameMode() != GameMode::InfiniteWar || m_Show == false || m_pGalaxy->IsInitialised() == false )
    {
        return;
    }

    const float sectorSize = m_Size.x / NumSectorsX;

    PositionData posData;
    UVData uvData;

    for ( int i = 0; i < static_cast<int>( FactionId::Count ); ++i )
    {
        Faction* pFaction = g_pGame->GetFaction( static_cast<FactionId>( i ) );
        SectorInfo* pHomeworld = pFaction->GetHomeworld();

        if ( pHomeworld != nullptr && m_pGalaxy->GetFogOfWar()->IsVisible( pHomeworld ) && m_HomeworldImages[ i ] != nullptr )
        {
            int x, y;
            pHomeworld->GetCoordinates( x, y );

            posData.clear();
            uvData.clear();

            const float x1 = m_OffsetX + sectorSize * x;
            const float y1 = m_OffsetY + sectorSize * y;
            const float x2 = m_OffsetX + sectorSize + sectorSize * x;
            const float y2 = m_OffsetY + sectorSize + sectorSize * y;

            posData.emplace_back( x1, y1, 0.0f ); // 0
            posData.emplace_back( x1, y2, 0.0f ); // 1
            posData.emplace_back( x2, y2, 0.0f ); // 2
            posData.emplace_back( x1, y1, 0.0f ); // 0
            posData.emplace_back( x2, y2, 0.0f ); // 2
            posData.emplace_back( x2, y1, 0.0f ); // 3

            uvData.emplace_back( 0.0f, 0.0f ); // 0
            uvData.emplace_back( 0.0f, 1.0f ); // 1
            uvData.emplace_back( 1.0f, 1.0f ); // 2
            uvData.emplace_back( 0.0f, 0.0f ); // 0
            uvData.emplace_back( 1.0f, 1.0f ); // 2
            uvData.emplace_back( 1.0f, 0.0f ); // 3

            m_pSectorHomeworldDiffuseSampler->Set( m_HomeworldImages[ i ], GL_TEXTURE0 );

            m_pSectorHomeworldVB->CopyPositions( posData );
            m_pSectorHomeworldVB->CopyUVs( uvData );
            m_pSectorHomeworldShader->Use();
            m_pSectorHomeworldVB->Draw( 6 );
        }
    }
}

void GalaxyRep::DrawSectorsThreatRatings()
{
    if ( g_pGame->GetGameMode() == GameMode::InfiniteWar )
    {
        return;
    }

    std::array<SectorDrawInfoVector, static_cast<size_t>( ThreatRating::Count )> drawInfoVecs;
    FleetSharedPtr playerFleet = g_pGame->GetPlayerFleet().lock();
    const glm::vec2 playerFleetCoordinates( playerFleet->GetCurrentSector()->GetCoordinates() );

    for ( int x = 0; x < NumSectorsX; x++ )
    {
        for ( int y = 0; y < NumSectorsY; y++ )
        {
            if ( glm::distance( glm::vec2( x, y ), playerFleetCoordinates ) <= 2.0f )
            {
                SectorDrawInfo drawInfo( x, y );
                SectorInfo* pSectorInfo = m_pGalaxy->GetSectorInfo( x, y );
                ThreatRating threatRating = pSectorInfo->GetThreatRating();
                drawInfoVecs[ static_cast<size_t>( threatRating ) ].push_back( drawInfo );
            }
        }
    }

    for ( size_t i = 0; i < static_cast<size_t>( ThreatRating::Count ); ++i )
    {
        SectorDrawInfoVector& drawInfoVec = drawInfoVecs[ i ];
        Genesis::ShaderUniformInstances& shaderUniforms = m_pSectorThreatUniforms[ i ];
        DrawSectors( drawInfoVec, m_pSectorThreatShader, &shaderUniforms, false );
    }
}

void GalaxyRep::DrawSectors( SectorDrawInfoVector& drawInfoVec, Genesis::Shader* pShader, Genesis::ShaderUniformInstances* pShaderUniforms, bool useFactionColour )
{
    using namespace Genesis;

    if ( m_Show == false || m_pGalaxy->IsInitialised() == false || drawInfoVec.empty() )
        return;

    const float sectorSize = m_Size.x / NumSectorsX;

    PositionData posData;
    UVData uvData;
    ColourData colourData;

    const size_t numVertices = drawInfoVec.size() * 6;
    posData.reserve( numVertices );
    uvData.reserve( numVertices );
    colourData.reserve( numVertices );

    for ( auto& drawInfo : drawInfoVec )
    {
        const float x1 = m_OffsetX + sectorSize * drawInfo.x;
        const float y1 = m_OffsetY + sectorSize * drawInfo.y;
        const float x2 = m_OffsetX + sectorSize + sectorSize * drawInfo.x;
        const float y2 = m_OffsetY + sectorSize + sectorSize * drawInfo.y;

        posData.emplace_back( x1, y1, 0.0f ); // 0
        posData.emplace_back( x1, y2, 0.0f ); // 1
        posData.emplace_back( x2, y2, 0.0f ); // 2
        posData.emplace_back( x1, y1, 0.0f ); // 0
        posData.emplace_back( x2, y2, 0.0f ); // 2
        posData.emplace_back( x2, y1, 0.0f ); // 3

        uvData.emplace_back( 0.0f, 0.0f ); // 0
        uvData.emplace_back( 0.0f, 1.0f ); // 1
        uvData.emplace_back( 1.0f, 1.0f ); // 2
        uvData.emplace_back( 0.0f, 0.0f ); // 0
        uvData.emplace_back( 1.0f, 1.0f ); // 2
        uvData.emplace_back( 1.0f, 0.0f ); // 3

        const glm::vec4 colour = useFactionColour ? drawInfo.colour.glm() : glm::vec4( 1.0f, 1.0f, 1.0f, 0.75f );
        for ( int i = 0; i < 6; ++i )
        {
            colourData.push_back( colour );
        }
    }

    m_pSectorVB->CopyPositions( posData );
    m_pSectorVB->CopyUVs( uvData );
    m_pSectorVB->CopyColours( colourData );
    pShader->Use( pShaderUniforms );
    m_pSectorVB->Draw( static_cast<uint32_t>( numVertices ) );
}

void GalaxyRep::DrawGrid()
{
    using namespace Genesis;

    if ( m_Show == false )
    {
        return;
    }

    const float sectorSize = m_Size.x / NumSectorsX;
    const float crossHalfSize = 4.0f;

    PositionData posData;
    UVData uvData;

    const unsigned int numVertices = NumSectorsX * NumSectorsY * 6;
    posData.reserve( numVertices );
    uvData.reserve( numVertices );

    for ( int x = 0; x < NumSectorsX; x++ )
    {
        for ( int y = 0; y < NumSectorsY; y++ )
        {
            posData.emplace_back( m_OffsetX + sectorSize * x - crossHalfSize, m_OffsetY + sectorSize * y - crossHalfSize, 0.0f ); // 0
            posData.emplace_back( m_OffsetX + sectorSize * x - crossHalfSize, m_OffsetY + sectorSize * y + crossHalfSize, 0.0f ); // 1
            posData.emplace_back( m_OffsetX + sectorSize * x + crossHalfSize, m_OffsetY + sectorSize * y + crossHalfSize, 0.0f ); // 2
            posData.emplace_back( m_OffsetX + sectorSize * x - crossHalfSize, m_OffsetY + sectorSize * y - crossHalfSize, 0.0f ); // 0
            posData.emplace_back( m_OffsetX + sectorSize * x + crossHalfSize, m_OffsetY + sectorSize * y + crossHalfSize, 0.0f ); // 2
            posData.emplace_back( m_OffsetX + sectorSize * x + crossHalfSize, m_OffsetY + sectorSize * y - crossHalfSize, 0.0f ); // 3

            uvData.emplace_back( 0.0f, 0.0f ); // 0
            uvData.emplace_back( 0.0f, 1.0f ); // 1
            uvData.emplace_back( 1.0f, 1.0f ); // 2
            uvData.emplace_back( 0.0f, 0.0f ); // 0
            uvData.emplace_back( 1.0f, 1.0f ); // 2
            uvData.emplace_back( 1.0f, 0.0f ); // 3
        }
    }

    m_pSectorCrossVB->CopyPositions( posData );
    m_pSectorCrossVB->CopyUVs( uvData );
    m_pSectorCrossShader->Use( &m_SectorCrossUniforms );
    m_pSectorCrossVB->Draw();
}

void GalaxyRep::DrawGoals()
{
    using namespace Genesis;

    Faction* pFaction = g_pGame->GetFaction( FactionId::Empire );
    if ( pFaction == nullptr )
    {
        return;
    }

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Add );

    EmpireFaction* pEmpireFaction = (EmpireFaction*)pFaction;
    RequestManager* pRequestManager = pEmpireFaction->GetRequestManager();
    for ( auto& pRequest : pRequestManager->GetRequests() )
    {
        for ( auto& pGoal : pRequest->GetGoals() )
        {
            pGoal->GetRepresentation()->Render();
        }
    }

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Disabled );
}

void GalaxyRep::Show( bool state )
{
    if ( state != IsVisible() )
    {
        m_Show = state;

        // Enabling the galaxy view disables the rendering of all other scene layers
        Genesis::Scene* pScene = Genesis::FrameWork::GetScene();
        pScene->SetLayerMask( state ? LAYER_GALAXY : ( ~LAYER_GALAXY ) );

        Faction* pFaction = g_pGame->GetFaction( FactionId::Empire );
        if ( pFaction != nullptr )
        {
            EmpireFaction* pEmpireFaction = (EmpireFaction*)pFaction;
            RequestManager* pRequestManager = pEmpireFaction->GetRequestManager();
            for ( auto& pRequest : pRequestManager->GetRequests() )
            {
                for ( auto& pGoal : pRequest->GetGoals() )
                {
                    pGoal->GetRepresentation()->Show( state );
                }
            }
        }
    }
}

void GalaxyRep::OnLeftMouseButtonDown()
{
    if ( m_InputPending == false )
    {
        m_InputPending = true;
        m_InputTimer = SDL_GetTicks() + 100u;
    }
}

} // namespace Hexterminate