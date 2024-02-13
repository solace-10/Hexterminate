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

#include "ship/addon/addonphasebarrier.h"
#include "faction/faction.h"
#include "hexterminate.h"
#include "laser/laser.h"
#include "laser/lasermanager.h"
#include "menus/shiptweaks.h"
#include "misc/mathaux.h"
#include "sector/sector.h"
#include "ship/hyperspacecore.h"
#include "ship/ship.h"
#include "ship/shipshaderuniforms.h"

#include <genesis.h>
#include <math/constants.h>
#include <math/misc.h>
#include <physics/ghost.h>
#include <physics/rigidbody.h>
#include <physics/shape.h>
#include <physics/simulation.h>
#include <render/debugrender.h>
#include <rendersystem.h>
#include <resources/resourceimage.h>
#include <scene/scene.h>
#include <scene/scenecamera.h>
#include <shadercache.h>
#include <shaderuniform.h>
#include <vertexbuffer.h>

// clang-format off
#include <beginexternalheaders.h>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <endexternalheaders.h>
// clang-format on

namespace Hexterminate
{

static const float sChargeDuration = 10.0f;
static const float sDischargeDuration = 5.0f;
static const Uint32 sMaxPhaseBarrierPoints = 4;

AddonPhaseBarrier::AddonPhaseBarrier( AddonModule* pModule, Ship* pOwner )
    : Addon( pModule, pOwner )
    , m_Charge( 1.0f )
    , m_pTexture( nullptr )
    , m_pShader( nullptr )
    , m_pVertexBuffer( nullptr )
    , m_pShieldStrengthUniform( nullptr )
    , m_pClipActiveUniform( nullptr )
    , m_pClipUniform( nullptr )
    , m_pClipForwardUniform( nullptr )
    , m_pAmbientColourUniform( nullptr )
    , m_pActiveUniform( nullptr )
    , m_pGhost( nullptr )
    , m_pCollisionInfo( nullptr )
    , m_Coverage( 30.0f )
    , m_SphereCenter( 0.0f )
    , m_SphereRadius( 0.0f )
{
    using namespace Genesis;

    m_Coverage = (float)atof( ( (AddonInfo*)pModule->GetModuleInfo() )->GetParameter().c_str() );

    m_RenderTransform = glm::mat4x4( 1.0f );

    m_pTexture = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/images/shieldgrid.jpg" );
    m_pTexture->EnableMipMapping( false );

    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );
    m_pShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "phasebarrier" );
    ShaderUniform* pSampler = m_pShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    pSampler->Set( m_pTexture, GL_TEXTURE0 );

    m_pShieldStrengthUniform = m_pShader->RegisterUniform( "k_shieldStrength", ShaderUniformType::Float );
    m_pShieldStrengthUniform->Set( 1.0f );

    // The phase barrier requires additional uniforms which would normally be set up by the ship, but since we use a custom
    // shader that doesn't work and we need to set them up manually.
    Shader* pSphereShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "phasebarriersphere" );
    m_pClipActiveUniform = pSphereShader->RegisterUniform( "k_clipActive", ShaderUniformType::Integer, false );
    m_pClipUniform = pSphereShader->RegisterUniform( "k_clip", ShaderUniformType::FloatVector4, false );
    m_pClipForwardUniform = pSphereShader->RegisterUniform( "k_clipForward", ShaderUniformType::FloatVector4, false );
    m_pAmbientColourUniform = pSphereShader->RegisterUniform( "k_a", ShaderUniformType::FloatVector4, false );

    // Is the phase barrier active or not? This is passed to the shader to toggle the glow. Value either 0.0 or 1.0.
    m_pActiveUniform = pSphereShader->RegisterUniform( "k_active", ShaderUniformType::Float );

    m_pCollisionInfo = std::make_unique<ShipCollisionInfo>( pOwner, this );
}

AddonPhaseBarrier::~AddonPhaseBarrier()
{
    DestroyPhysicsGhost();

    delete m_pVertexBuffer;
}

void AddonPhaseBarrier::Initialise()
{
    CreateGeometry();
    CalculateSphereCenter();
    CalculateSphereRadius();
}

void AddonPhaseBarrier::CalculateSphereCenter()
{
    // We need to know where the center of the sphere is so we can generate laser beams coming from it, rather than
    // the model's origin.
    glm::vec3 dummyPosition;
    if ( GetModule()->GetModel() != nullptr && GetModule()->GetModel()->GetDummy( "phasebarrier", &dummyPosition ) )
    {
        m_SphereCenter = dummyPosition.z;
    }
}

void AddonPhaseBarrier::CalculateSphereRadius()
{
    // We don't want the laser beams to come from the very center of the sphere, but from the edge.
    // The model has a dummy "phasebarrieredge" at the edge of the sphere, which we use to calculate the radius.
    using namespace Genesis;
    glm::vec3 dummyPosition;
    if ( GetModule()->GetModel() != nullptr && GetModule()->GetModel()->GetDummy( "phasebarrieredge", &dummyPosition ) )
    {
        const glm::vec3 diff = dummyPosition - glm::vec3( 0.0f, 0.0f, m_SphereCenter );
        m_SphereRadius = glm::length( diff );
    }
}

void AddonPhaseBarrier::Activate()
{
    Addon::Activate();

    CreatePhysicsGhost();
}

void AddonPhaseBarrier::Deactivate()
{
    Addon::Deactivate();

    DestroyPhysicsGhost();
}

// The phase barrier depletes when active and charges when not in use.
// Returned value is between 0 and 1.
float AddonPhaseBarrier::GetCharge() const
{
    return m_Charge;
}

void AddonPhaseBarrier::UpdateCharge( float delta )
{
    if ( IsActive() )
    {
        m_Charge = std::max( m_Charge - delta / sDischargeDuration, 0.0f );
        if ( m_Charge <= 0.0f )
        {
            Deactivate();
        }
    }
    else
    {
        m_Charge = std::min( m_Charge + delta / sChargeDuration, 1.0f );
    }
}

void AddonPhaseBarrier::Update( float delta )
{
    Addon::Update( delta );

    UpdateCharge( delta );

    HyperspaceCore* pHyperspaceCore = GetOwner()->GetHyperspaceCore();
    const bool jumping = ( pHyperspaceCore != nullptr && pHyperspaceCore->IsJumping() );
    const bool destroyed = m_pModule->IsDestroyed();
    if ( IsActive() && ( jumping || destroyed ) )
    {
        Deactivate();
    }

    if ( m_pGhost != nullptr )
    {
        // Find out where we're pointing at in relation to the ship's centre
        Genesis::InputManager* pInputManager = Genesis::FrameWork::GetInputManager();
        const glm::vec2& mousePosition = pInputManager->GetMousePosition();
        const glm::vec3 raycastResult = Genesis::FrameWork::GetRenderSystem()->Raycast( mousePosition );
        const glm::vec3 shipCentre = GetOwner()->GetCentre( TransformSpace::World );

        glm::vec4 forward( glm::normalize( glm::vec3(
                               raycastResult.x - shipCentre.x,
                               raycastResult.y - shipCentre.y,
                               0.0f ) ),
            0.0f );

        glm::vec4 right( forward.y, -forward.x, 0.0f, 0.0f );
        glm::vec4 up( 0.0f, 0.0f, 1.0f, 0.0f );
        glm::mat4x4 barrierDirectionTransform( right, forward, up, glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );

        glm::vec3 topLeft, bottomRight;
        GetOwner()->GetBoundingBox( topLeft, bottomRight ); // The bounding box is in local space
        const float w = ( topLeft.x - bottomRight.x ) / 2.0f;
        const float h = ( topLeft.y - bottomRight.y ) / 2.0f;
        const float distanceFromCentre = sqrtf( w * w + h * h ) + 45.0f;

        glm::mat4x4 barrierTransform = glm::column( barrierDirectionTransform, 3, glm::vec4( shipCentre, 1.0f ) + forward * distanceFromCentre );

#ifdef _DEBUG
        if ( g_pGame->GetCurrentSector()->GetShipTweaks()->GetDrawPhaseBarrier() )
        {
            Genesis::Render::DebugRender* pDebugRender = Genesis::FrameWork::GetDebugRender();

            // Ship centre
            pDebugRender->DrawCircle( shipCentre, 10.0f, glm::vec3( 1.0f, 0.0f, 0.0f ) );

            // The "rail" around which the barrier goes.
            pDebugRender->DrawCircle( shipCentre, distanceFromCentre, glm::vec3( 1.0f, 0.0f, 0.0f ) );

            // The barrier's location.
            glm::vec3 barrierLocation = glm::vec3( glm::column( barrierTransform, 3 ) );
            float barrierRadius = m_Coverage / 2.0f;
            pDebugRender->DrawCircle( barrierLocation, barrierRadius, glm::vec3( 1.0f, 1.0f, 1.0f ) );

            pDebugRender->DrawLine( barrierLocation, barrierLocation + glm::vec3( forward ) * barrierRadius, glm::vec3( 0.0f, 1.0f, 0.0f ) );
            pDebugRender->DrawLine( barrierLocation, barrierLocation + glm::vec3( right ) * barrierRadius, glm::vec3( 0.0f, 1.0f, 1.0f ) );
        }
#endif

        m_RenderTransform = barrierTransform;
        m_pGhost->SetWorldTransform( barrierTransform );

        CreateLasers( glm::vec3( shipCentre.x, shipCentre.y, shipCentre.z ), glm::vec3( glm::column( barrierTransform, 3 ) ) );
    }
}

void AddonPhaseBarrier::CreateLasers( const glm::vec3& barrierOrigin, const glm::vec3& barrierPosition )
{
    glm::vec3 dir = glm::normalize( barrierPosition - barrierOrigin );

    LaserManager* pLaserManager = g_pGame->GetCurrentSector()->GetLaserManager();
    Genesis::Color barrierColour( 1.0f, 0.55f, 0.0f, 0.65f );

    // The laser origin is at the edge of the phase barrier's sphere
    glm::vec3 laserOrigin = GetModule()->GetWorldPosition() + glm::vec3( 0.0f, 0.0f, m_SphereCenter );
    laserOrigin += dir * m_SphereRadius;

    // We have three beams, one to the left, one to the right and one randomly in-between
    const float halfCoverage = m_Coverage / 2.0f;
    glm::vec3 leftLaserEndPosition = glm::vec3( -dir.y * halfCoverage, dir.x * halfCoverage, 0.0f ) + barrierPosition;
    Laser laserLeft( laserOrigin, leftLaserEndPosition, barrierColour, 1.5f );
    pLaserManager->AddLaser( laserLeft );

    glm::vec3 rightLaserEndPosition = glm::vec3( dir.y * halfCoverage, -dir.x * halfCoverage, 0.0f ) + barrierPosition;
    Laser laserRight( laserOrigin, rightLaserEndPosition, barrierColour, 1.5f );
    pLaserManager->AddLaser( laserRight );

    float randomFactor = gRand( 0.0f, 2.0f );
    glm::vec3 centreLaserEndPosition = rightLaserEndPosition + glm::vec3( -dir.y * halfCoverage * randomFactor, dir.x * halfCoverage * randomFactor, 0.0f );
    Laser centreLaser( laserOrigin, centreLaserEndPosition, barrierColour, 1.5f );
    pLaserManager->AddLaser( centreLaser );
}

void AddonPhaseBarrier::Render( const glm::mat4& /*moduleModelTransform*/ )
{
    using namespace Genesis;

    ShipShaderUniforms* pShipShaderUniforms = GetModule()->GetOwner()->GetShipShaderUniforms();

    int clipActive;
    pShipShaderUniforms->Get( ShipShaderUniform::ClipActive )->Get( &clipActive );
    m_pClipActiveUniform->Set( clipActive );

    glm::vec4 clip;
    pShipShaderUniforms->Get( ShipShaderUniform::Clip )->Get( &clip );
    m_pClipUniform->Set( clip );

    glm::vec4 clipForward;
    pShipShaderUniforms->Get( ShipShaderUniform::ClipForward )->Get( &clipForward );
    m_pClipForwardUniform->Set( clipForward );

    glm::vec4 ambientColour;
    pShipShaderUniforms->Get( ShipShaderUniform::AmbientColour )->Get( &ambientColour );
    m_pAmbientColourUniform->Set( ambientColour );

    m_pActiveUniform->Set( IsActive() ? 1.0f : 0.0f );

    if ( m_pGhost != nullptr )
    {
        FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Add );

        m_pShieldStrengthUniform->Set( 1.0f );
        m_pShader->Use( m_RenderTransform );

        Genesis::FrameWork::GetRenderSystem()->SetRenderTarget( Genesis::RenderTargetId::Glow );
        m_pVertexBuffer->Draw();

        Genesis::FrameWork::GetRenderSystem()->SetRenderTarget( Genesis::RenderTargetId::Default );
        m_pVertexBuffer->Draw();

        FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Disabled );
    }
}

void AddonPhaseBarrier::CreateGeometry()
{
    using namespace Genesis;

    PositionData posData;
    UVData uvData;
    posData.reserve( sMaxPhaseBarrierPoints * 3 * 6 );
    uvData.reserve( sMaxPhaseBarrierPoints * 3 * 6 );

    const float scale[ 4 ] = { 0.5f, 1.0f, 1.0f, 0.4f };
    const float height[ 4 ] = { -40.0f, 0.0f, 0.0f, 40.0f };
    const float v[ 4 ] = { 0.0f, 0.34f, 0.66f, 1.0f };
    const float uStep = 1.0f / (float)sMaxPhaseBarrierPoints;

    const float segmentAperture = Genesis::kPi / sMaxPhaseBarrierPoints;
    float theta = segmentAperture - Genesis::kPi * 2.0f;

    float pc = cosf( 0.0f );
    float ps = sinf( 0.0f );

    const float thickness = 10.0f;
    const float halfCoverage = m_Coverage / 2.0f;

    for ( int k = 0; k < 3; ++k )
    {
        float u = 0.0f;

        for ( int i = 0; i < sMaxPhaseBarrierPoints; ++i )
        {
            const float c = cosf( theta );
            const float s = sinf( theta );
            const float l1 = scale[ k ];
            const float l2 = scale[ k + 1 ];
            const float z1 = height[ k ];
            const float z2 = height[ k + 1 ];
            const float v1 = v[ k ];
            const float v2 = v[ k + 1 ];

            posData.emplace_back( pc * l1 * halfCoverage, ps * l1 * thickness, z1 ); // 0
            posData.emplace_back( pc * l2 * halfCoverage, ps * l2 * thickness, z2 ); // 2
            posData.emplace_back( c * l1 * halfCoverage, s * l1 * thickness, z1 ); // 1

            posData.emplace_back( pc * l2 * halfCoverage, ps * l2 * thickness, z2 ); // 2
            posData.emplace_back( c * l2 * halfCoverage, s * l2 * thickness, z2 ); // 3
            posData.emplace_back( c * l1 * halfCoverage, s * l1 * thickness, z1 ); // 1

            uvData.emplace_back( u, v1 );
            uvData.emplace_back( u, v2 );
            uvData.emplace_back( u + uStep, v1 );

            uvData.emplace_back( u, v2 );
            uvData.emplace_back( u + uStep, v2 );
            uvData.emplace_back( u + uStep, v1 );

            theta += segmentAperture;
            u += uStep;
            pc = c;
            ps = s;
        }
    }

    m_pVertexBuffer->CopyPositions( posData );
    m_pVertexBuffer->CopyUVs( uvData );

    UpdateColour();
}

void AddonPhaseBarrier::UpdateColour()
{
    using namespace Genesis;

    ColourData colourData;
    colourData.reserve( sMaxPhaseBarrierPoints * 3 * 6 );

    const float alphaMultiplier[ 4 ] = { 0.0f, 1.0f, 1.0f, 0.75f };
    const Genesis::Color shieldColour( 1.0f, 0.35f, 0.0f, 1.0f );

    for ( int k = 0; k < 3; ++k )
    {
        const float am1 = alphaMultiplier[ k ];
        const float am2 = alphaMultiplier[ k + 1 ];

        for ( int i = 0; i < sMaxPhaseBarrierPoints; ++i )
        {
            colourData.emplace_back( shieldColour.r * am1, shieldColour.g * am1, shieldColour.b * am1, 0.5f * am1 );
            colourData.emplace_back( shieldColour.r * am2, shieldColour.g * am2, shieldColour.b * am2, 0.5f * am2 );
            colourData.emplace_back( shieldColour.r * am1, shieldColour.g * am1, shieldColour.b * am1, 0.5f * am1 );

            colourData.emplace_back( shieldColour.r * am2, shieldColour.g * am2, shieldColour.b * am2, 0.5f * am2 );
            colourData.emplace_back( shieldColour.r * am2, shieldColour.g * am2, shieldColour.b * am2, 0.5f * am2 );
            colourData.emplace_back( shieldColour.r * am1, shieldColour.g * am1, shieldColour.b * am1, 0.5f * am1 );
        }
    }

    m_pVertexBuffer->CopyColours( colourData );
}

void AddonPhaseBarrier::CreatePhysicsGhost()
{
    using namespace Genesis::Physics;

    SDL_assert( m_pGhost == nullptr );

    Genesis::Physics::ShapeSharedPtr pShape = CreatePhysicsShape();
    pShape->SetUserData( m_pCollisionInfo.get() );
    m_pGhost = new Ghost( pShape, glm::mat4x4( 1.0f ) );

    g_pGame->GetPhysicsSimulation()->Add( m_pGhost );
}

void AddonPhaseBarrier::DestroyPhysicsGhost()
{
    if ( m_pGhost != nullptr )
    {
        g_pGame->GetPhysicsSimulation()->Remove( m_pGhost );
        delete m_pGhost;
        m_pGhost = nullptr;
    }
}

Genesis::Physics::ShapeSharedPtr AddonPhaseBarrier::CreatePhysicsShape()
{
    using namespace Genesis::Physics;

    const float halfCoverage = m_Coverage / 2.0f;

    // The physics shape for the shield is a very simplified version of the visual shape.
    // Since the game is effectively 2D for gameplay purposes, the shape rather than being an ellipse is
    // a cylinder with distinct major and minor axis.

    const int maxPoints = 16;
    const int numVertices = 16 * 2;
    ConvexHullVertices vertexData;
    vertexData.reserve( numVertices );

    float height[ 2 ] = { -30.0f, 30.0f };

    const float segmentAperture = Genesis::kPi * 2.0f / maxPoints;
    float theta = segmentAperture;

    for ( int j = 0; j < 2; ++j )
    {
        const float z = height[ j ];

        for ( int i = 0; i < maxPoints; ++i, theta += segmentAperture )
        {
            vertexData.emplace_back( cosf( theta ) * halfCoverage, sinf( theta ) * 10.0f, z );
        }
    }

    return std::make_shared<ConvexHullShape>( vertexData );
}

} // namespace Hexterminate
