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
#include <math/constants.h>
#include <shader.h>
#include <shadercache.h>
#include <shaderuniform.h>
#include <vertexbuffer.h>

#include "faction/faction.h"
#include "fleet/fleet.h"
#include "fleet/fleetrep.h"
#include "misc/mathaux.h"
#include "sector/fogofwar.h"
#include "sector/galaxy.h"
#include "sector/galaxyrep.h"
#include "sector/sector.h"
#include <inputmanager.h>
#include <rendersystem.h>
#include <resources/resourceimage.h>
#include <scene/layer.h>
#include <scene/scene.h>

#include "globals.h"
#include "hexterminate.h"

namespace Hexterminate
{

static const float FleetBlinkDuration = 0.5f;

FleetRep::FleetRep( Fleet* pFleet )
    : m_pFleet( pFleet )
    , m_Show( true )
    , m_pImage( nullptr )
    , m_pImageFlagship( nullptr )
    , m_pShader( nullptr )
    , m_pDiffuseSampler( nullptr )
    , m_pVertexBuffer( nullptr )
    , m_Angle( 0.0f )
    , m_BlinkTimer( 0.0f )
    , m_DisplayFlagship( false )
{
    using namespace Genesis;

    m_pImage = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/ui/sector/fleet.png" );
    m_pImageFlagship = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/ui/sector/fleetflagship.png" );

    m_pShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "textured_vertex_coloured" );
    m_pDiffuseSampler = m_pShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    m_pDiffuseSampler->Set( m_pImage, GL_TEXTURE0 );

    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );
}

FleetRep::~FleetRep()
{
    delete m_pVertexBuffer;
}

void FleetRep::Initialise()
{
    Genesis::Scene* pScene = Genesis::FrameWork::GetScene();
    m_pLayer = pScene->AddLayer( LAYER_GALAXY, true );
    m_pLayer->AddSceneObject( this );
}

void FleetRep::Update( float delta )
{
    if ( m_pFleet == nullptr )
    {
        return;
    }

    m_BlinkTimer -= delta;

    if ( m_pFleet->IsEngaged() )
    {
        if ( m_BlinkTimer < 0.0f )
        {
            m_BlinkTimer = FleetBlinkDuration;
        }
    }

    if ( m_BlinkTimer < 0.0f )
        m_BlinkTimer = 0.0f;

    if ( m_pFleet->HasFlagship() && !m_DisplayFlagship )
    {
        m_pDiffuseSampler->Set( m_pImageFlagship, GL_TEXTURE0 );
        m_DisplayFlagship = true;
    }
    else if ( !m_pFleet->HasFlagship() && m_DisplayFlagship )
    {
        m_pDiffuseSampler->Set( m_pImage, GL_TEXTURE0 );
        m_DisplayFlagship = false;
    }
}

void FleetRep::RemoveFromScene()
{
    m_pFleet = nullptr;
    m_pLayer->RemoveSceneObject( this );
}

void FleetRep::Render()
{
    using namespace Genesis;

    if ( !g_pGame->GetGalaxy() || !g_pGame->GetGalaxy()->IsVisible() )
    {
        return;
    }

    FogOfWar* pFogOfWar = g_pGame->GetGalaxy()->GetFogOfWar();
    if ( pFogOfWar != nullptr && pFogOfWar->IsVisible( m_pFleet->GetCurrentSector() ) == false )
    {
        return;
    }

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Blend );

    DrawChevron();

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Disabled );
}

void FleetRep::DrawChevron()
{
    using namespace Genesis;

    if ( m_Show == false )
        return;

    // Skip the "blinking" effect if the game is paused, as not to give the idea that
    // the game is still running.
    if ( g_pGame->IsPaused() == false && m_BlinkTimer > FleetBlinkDuration / 2.0f )
        return;

    GalaxyRep* pGalaxyRep = g_pGame->GetGalaxy()->GetRepresentation();
    glm::vec2 fleetPos = m_pFleet->GetPosition();
    glm::vec2 screenPos = fleetPos * pGalaxyRep->GetSize();

    const Math::FPoint2 galaxyOffset = pGalaxyRep->GetOffset();
    screenPos.x += galaxyOffset.x;
    screenPos.y += galaxyOffset.y;

    SetPointOfInterestEnd( screenPos );

    // Calculate the angle based on the fleet's direction
    glm::vec2 fleetDst = m_pFleet->GetDestination();
    glm::vec2 positionDelta = fleetDst - fleetPos;
    float deltaLen = glm::length( positionDelta );
    if ( deltaLen > 0.0f ) // Only update the angle if we actually have a valid direction, otherwise use the last one
    {
        m_Angle = atan2( positionDelta.y, positionDelta.x );
    }

    const Genesis::Color& clr = m_pFleet->GetFaction()->GetColour( FactionColourId::FleetChevron );
    m_pVertexBuffer->CreateTexturedQuad( -16.0f, -16.0f, 32.0f, 32.0f, clr.glm() );
    m_pDiffuseSampler->Set( m_pFleet->HasFlagship() ? m_pImageFlagship : m_pImage, GL_TEXTURE0 );
    const glm::mat4 modelMatrix = glm::translate( glm::vec3( screenPos.x, screenPos.y, 0.0f ) ) * glm::rotate( glm::mat4( 1.0f ), m_Angle, glm::vec3( 0.0f, 0.0f, 1.0f ) );
    m_pShader->Use( modelMatrix );
    m_pVertexBuffer->Draw();
}

void FleetRep::Show( bool state )
{
    m_Show = state;
}

} // namespace Hexterminate