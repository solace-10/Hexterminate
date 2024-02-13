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

//#define SHIELD_HIT_REGISTRY_DEBUG

#include "ship/shield.h"

#include <genesis.h>
#include <math/constants.h>
#include <math/misc.h>
#include <physics/ghost.h>
#include <physics/rigidbody.h>
#include <physics/shape.h>
#include <physics/simulation.h>
#include <rendersystem.h>
#include <resourcemanager.h>
#include <resources/resourceimage.h>
#include <shader.h>
#include <shadercache.h>
#include <shaderuniform.h>

#include "faction/faction.h"
#include "hexterminate.h"
#include "player.h"
#include "sector/sector.h"
#include "shadertweaks.h"
#include "ship/hyperspacecore.h"
#include "ship/module.h"
#include "ship/moduleinfo.h"
#include "ship/ship.h"
#include "ship/shipcollisioninfo.h"

namespace Hexterminate
{

static const float sShieldOfflineDuration = 20.0f;

Shield::Shield( Ship* pShip )
    : m_pOwner( pShip )
    , m_pTexture( nullptr )
    , m_pShader( nullptr )
    , m_pVertexBuffer( nullptr )
    , m_pShieldStrengthUniform( nullptr )
    , m_RadiusX( 1.0f )
    , m_RadiusY( 1.0f )
    , m_RechargeRate( 0.0f )
    , m_MaximumHitPoints( FLT_MAX )
    , m_CurrentHitPoints( FLT_MAX )
    , m_DeactivatedTimer( 0.0f )
    , m_ActivationRatio( 0.0f )
    , m_EmergencyCapacitorsCooldown( 0.0f )
    , m_State( ShieldState::Deactivated )
    , m_pCollisionInfo( nullptr )
{
    using namespace Genesis;

    m_pTexture = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/images/shieldgrid.jpg" );
    m_pTexture->EnableMipMapping( false );

    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );
    m_pShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "shield" );

    ShaderUniform* pSampler = m_pShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    if ( pSampler != nullptr )
    {
        pSampler->Set( m_pTexture, GL_TEXTURE0 );
    }

    m_pQuantumModeUniform = m_pShader->RegisterUniform( "k_quantum", ShaderUniformType::Boolean );
    if ( m_pQuantumModeUniform != nullptr )
    {
        m_pQuantumModeUniform->Set( false );
    }

    m_pShieldStrengthUniform = m_pShader->RegisterUniform( "k_shieldStrength", ShaderUniformType::Float );
    if ( m_pShieldStrengthUniform != nullptr )
    {
        m_pShieldStrengthUniform->Set( 0.0f );
    }

    m_pQuantumShieldScaleUniform = m_pShader->RegisterUniform( "k_quantumScale", ShaderUniformType::Float );
    m_pQuantumShieldTriangleGapUniform = m_pShader->RegisterUniform( "k_quantumGaps", ShaderUniformType::Float );
    m_pQuantumShieldIntensityUniform = m_pShader->RegisterUniform( "k_quantumIntensity", ShaderUniformType::Float );
    m_pQuantumShieldColourUniform = m_pShader->RegisterUniform( "k_quantumColour", ShaderUniformType::FloatVector3 );

    m_pCollisionInfo = std::make_unique<ShipCollisionInfo>( pShip, this );
}

Shield::~Shield()
{
    delete m_pVertexBuffer;

    if ( m_State != ShieldState::Deactivating && m_State != ShieldState::Deactivated )
    {
        g_pGame->GetPhysicsSimulation()->Remove( m_pGhost.get() );
    }
}

void Shield::Update( float delta )
{
    Genesis::SceneObject::Update( delta );

    m_MaximumHitPoints = 0.0f;
    m_RechargeRate = 0.0f;

    const ShieldModuleList& shieldModules = m_pOwner->GetShieldModules();
    for ( auto& pShieldModule : shieldModules )
    {
        if ( pShieldModule->IsDestroyed() == false )
        {
            m_MaximumHitPoints += pShieldModule->GetCapacity();
            m_RechargeRate += pShieldModule->GetPeakRechargeRate();
        }
    }

    m_EmergencyCapacitorsCooldown = gMax( 0.0f, m_EmergencyCapacitorsCooldown - delta );

    Sector* pSector = g_pGame->GetCurrentSector();
    if ( pSector != nullptr )
    {
        TowerBonus towerBonus;
        float bonusMagnitude;
        pSector->GetTowerBonus( m_pOwner->GetFaction(), &towerBonus, &bonusMagnitude );
        if ( towerBonus == TowerBonus::Shields )
        {
            m_RechargeRate *= bonusMagnitude;
            m_MaximumHitPoints *= bonusMagnitude;
        }
    }

    HyperspaceCore* pHyperspaceCore = m_pOwner->GetHyperspaceCore();
    const bool jumping = ( pHyperspaceCore != nullptr && pHyperspaceCore->IsJumping() );
    if ( m_State == ShieldState::Deactivated )
    {
        if ( m_RechargeRate > 0.0f )
        {
            m_DeactivatedTimer -= delta;
        }

        if ( m_DeactivatedTimer <= 0.0f && jumping == false && m_pOwner->IsDestroyed() == false )
        {
            m_State = ShieldState::Activating;

            // When the shield is reactivated, start it at 20% capacity
            if ( m_CurrentHitPoints <= 0.0f )
            {
                m_CurrentHitPoints = m_MaximumHitPoints * 0.2f;
            }
        }
    }
    else if ( m_State == ShieldState::Activating )
    {
        if ( m_ActivationRatio >= 1.0f )
        {
            m_State = ShieldState::Activated;
            m_ActivationRatio = 1.0f;
            g_pGame->GetPhysicsSimulation()->Add( m_pGhost.get() );
        }
        else
        {
            m_ActivationRatio += delta * 2.0f;
        }
    }
    else if ( m_State == ShieldState::Activated )
    {
        m_HitRegistry.Update( delta );

#ifdef SHIELD_HIT_REGISTRY_DEBUG
        glm::vec3 position = m_pOwner->GetTowerPosition();
        position.add( glm::vec3( 60.0f, 0.0f, 0.0f ) );
        m_HitRegistry.DebugDraw( position( 0 ), position( 1 ) );
#endif

        bool destroyed = m_pOwner->IsDestroyed();
        if ( jumping || destroyed )
        {
            Deactivate();
        }
        else
        {
            if ( m_MaximumHitPoints > 0.0f && m_RechargeRate > 0.0f )
            {
                // The recharge curve peaks at about a third of the maximum shield capacity
                const float rechargeFraction = m_CurrentHitPoints / m_MaximumHitPoints;
                const float rechargeAmount = m_RechargeRate * 1.0f / coshf( rechargeFraction * 6.0f - 2.0f ); // 1 / cosh = hyperbolic secant
                m_CurrentHitPoints += rechargeAmount * delta;
            }

            if ( m_CurrentHitPoints > m_MaximumHitPoints )
            {
                m_CurrentHitPoints = m_MaximumHitPoints;
            }
        }
    }
    else if ( m_State == ShieldState::Deactivating )
    {
        if ( m_ActivationRatio <= 0.0f )
        {
            m_State = ShieldState::Deactivated;
            m_ActivationRatio = 0.0f;
        }
        else
        {
            m_ActivationRatio -= delta * 2.0f;
        }
    }

    if ( m_State != ShieldState::Deactivated )
    {
        m_pGhost->SetWorldTransform( m_pOwner->GetRigidBody()->GetWorldTransform() );
    }
}

void Shield::RenderRegularShield( const glm::mat4& modelTransform )
{
    using namespace Genesis;

    if ( m_pQuantumModeUniform != nullptr )
    {
        m_pQuantumModeUniform->Set( false );
    }

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Add );

    const float shieldStrength = ( m_MaximumHitPoints <= 0.0f ) ? 0.0f : m_CurrentHitPoints / m_MaximumHitPoints;
    m_pShieldStrengthUniform->Set( shieldStrength );

    glm::mat4 shieldTransform = modelTransform * glm::translate( m_pOwner->GetCentre( TransformSpace::Local ) );
    m_pShader->Use( shieldTransform );

    Genesis::FrameWork::GetRenderSystem()->SetRenderTarget( RenderTargetId::Glow );
    m_pVertexBuffer->Draw();

    Genesis::FrameWork::GetRenderSystem()->SetRenderTarget( RenderTargetId::Default );
    m_pVertexBuffer->Draw();

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Disabled );
}

void Shield::RenderQuantumShield( const glm::mat4& modelTransform )
{
    using namespace Genesis;

    AddonQuantumStateAlternator* pAlternator = m_pOwner->GetQuantumStateAlternator();
    if ( pAlternator == nullptr || pAlternator->GetQuantumState() == QuantumState::Inactive )
    {
        return;
    }

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Blend );

    if ( m_pQuantumModeUniform != nullptr )
    {
        m_pQuantumModeUniform->Set( true );
    }

    if ( m_pQuantumShieldScaleUniform != nullptr )
    {
        m_pQuantumShieldScaleUniform->Set( ShaderTweaksDebugWindow::GetQuantumShieldPatternScale() );
    }

    if ( m_pQuantumShieldTriangleGapUniform != nullptr )
    {
        m_pQuantumShieldTriangleGapUniform->Set( ShaderTweaksDebugWindow::GetQuantumShieldTriangleGap() );
    }

    if ( m_pQuantumShieldIntensityUniform != nullptr )
    {
        m_pQuantumShieldIntensityUniform->Set( ShaderTweaksDebugWindow::GetQuantumShieldIntensity() );
    }

    if ( m_pQuantumShieldColourUniform != nullptr )
    {
        if ( pAlternator->GetQuantumState() == QuantumState::White )
        {
            m_pQuantumShieldColourUniform->Set( glm::vec3( 1.0f, 1.0f, 1.0f ) );
        }
        else if ( pAlternator->GetQuantumState() == QuantumState::Black )
        {
            m_pQuantumShieldColourUniform->Set( glm::vec3( 0.0f, 0.0f, 0.0f ) );
        }
    }

    float scale = ShaderTweaksDebugWindow::GetQuantumShieldGeometryScale();
    glm::mat4 shieldTransform = glm::scale( modelTransform, glm::vec3( scale ) ) * glm::translate( m_pOwner->GetCentre( TransformSpace::Local ) );
    m_pShader->Use( shieldTransform );

    if ( ShaderTweaksDebugWindow::GetQuantumShieldGlowPass() )
    {
        Genesis::FrameWork::GetRenderSystem()->SetRenderTarget( RenderTargetId::Glow );
        m_pVertexBuffer->Draw();
    }

    Genesis::FrameWork::GetRenderSystem()->SetRenderTarget( RenderTargetId::Default );
    m_pVertexBuffer->Draw();

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Disabled );
}

void Shield::Render( const glm::mat4& modelTransform )
{
    using namespace Genesis;

    if ( m_State == ShieldState::Deactivated )
    {
        return;
    }

    CreateGeometry();

    // Shields can't write to the depth buffer, otherwise they'll render incorrectly when they overlap.
    glDepthMask( false );

    if ( m_pShieldStrengthUniform != nullptr )
    {
        const float shieldStrength = ( m_MaximumHitPoints <= 0.0f ) ? 0.0f : m_CurrentHitPoints / m_MaximumHitPoints;
        m_pShieldStrengthUniform->Set( shieldStrength );
    }

    RenderRegularShield( modelTransform );
    RenderQuantumShield( modelTransform );

    glDepthMask( true );
}

void Shield::CreateGeometry()
{
    using namespace Genesis;

    PositionData posData;
    UVData uvData;

    const float scale[ 4 ] = { 0.5f, 1.0f, 1.0f, 0.5f };
    const float height[ 4 ] = { -30.0f, -10.0f, 10.0f, 30.0f };
    const float v[ 4 ] = { 0.0f, 0.34f, 0.66f, 1.0f };
    const float uStep = 1.0f / (float)sMaxShieldPoints;

    const float segmentAperture = Genesis::kPi * 2.0f / sMaxShieldPoints;
    float theta = segmentAperture - Genesis::kPi * 2.0f;

    float pc = cosf( 0.0f );
    float ps = sinf( 0.0f );

    for ( int k = 0; k < 3; ++k )
    {
        float u = 0.0f;

        for ( int i = 0; i < sMaxShieldPoints; ++i )
        {
            const float c = cosf( theta );
            const float s = sinf( theta );
            const float l1 = scale[ k ];
            const float l2 = scale[ k + 1 ];
            const float z1 = height[ k ];
            const float z2 = height[ k + 1 ];
            const float v1 = v[ k ];
            const float v2 = v[ k + 1 ];

            posData.push_back( glm::vec3( pc * l1 * m_RadiusX, ps * l1 * m_RadiusY, z1 ) ); // 0
            posData.push_back( glm::vec3( pc * l2 * m_RadiusX, ps * l2 * m_RadiusY, z2 ) ); // 2
            posData.push_back( glm::vec3( c * l1 * m_RadiusX, s * l1 * m_RadiusY, z1 ) ); // 1

            posData.push_back( glm::vec3( pc * l2 * m_RadiusX, ps * l2 * m_RadiusY, z2 ) ); // 2
            posData.push_back( glm::vec3( c * l2 * m_RadiusX, s * l2 * m_RadiusY, z2 ) ); // 3
            posData.push_back( glm::vec3( c * l1 * m_RadiusX, s * l1 * m_RadiusY, z1 ) ); // 1

            uvData.push_back( glm::vec2( u, v1 ) );
            uvData.push_back( glm::vec2( u, v2 ) );
            uvData.push_back( glm::vec2( u + uStep, v1 ) );

            uvData.push_back( glm::vec2( u, v2 ) );
            uvData.push_back( glm::vec2( u + uStep, v2 ) );
            uvData.push_back( glm::vec2( u + uStep, v1 ) );

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

void Shield::UpdateColour()
{
    using namespace Genesis;

    m_ColourData.clear();
    m_ColourData.reserve( sMaxShieldPoints * 3 * 6 );

    const float alphaMultiplier[ 4 ] = { 0.0f, 1.0f, 1.0f, 0.0f };
    Genesis::Color shieldColour = m_pOwner->GetFaction()->GetColour( m_pOwner->IsFlagship() ? FactionColourId::GlowFlagship : FactionColourId::Glow );
    float activationThreshold = m_ActivationRatio * sMaxShieldPoints;

    for ( int k = 0; k < 3; ++k )
    {
        const float am1 = alphaMultiplier[ k ];
        const float am2 = alphaMultiplier[ k + 1 ];

        for ( int i = 0; i < sMaxShieldPoints; ++i )
        {
            const float hitRegistryValue = 0.1f + m_HitRegistry.SampleAt( i ) * 0.9f;
            if ( i <= (int)activationThreshold )
            {
                m_ColourData.push_back( glm::vec4( shieldColour.r * am1, shieldColour.g * am1, shieldColour.b * am1, hitRegistryValue * am1 ) );
                m_ColourData.push_back( glm::vec4( shieldColour.r * am2, shieldColour.g * am2, shieldColour.b * am2, hitRegistryValue * am2 ) );
                m_ColourData.push_back( glm::vec4( shieldColour.r * am1, shieldColour.g * am1, shieldColour.b * am1, hitRegistryValue * am1 ) );

                m_ColourData.push_back( glm::vec4( shieldColour.r * am2, shieldColour.g * am2, shieldColour.b * am2, hitRegistryValue * am2 ) );
                m_ColourData.push_back( glm::vec4( shieldColour.r * am2, shieldColour.g * am2, shieldColour.b * am2, hitRegistryValue * am2 ) );
                m_ColourData.push_back( glm::vec4( shieldColour.r * am1, shieldColour.g * am1, shieldColour.b * am1, hitRegistryValue * am1 ) );
            }
            else
            {
                for ( int j = 0; j < 6; ++j )
                {
                    m_ColourData.push_back( glm::vec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
                }
            }
        }
    }

    m_pVertexBuffer->CopyColours( m_ColourData );
}

void Shield::InitialisePhysics( const glm::vec3& translation, float radiusX, float radiusY )
{
    using namespace Genesis::Physics;

    m_RadiusX = radiusX;
    m_RadiusY = radiusY;

    // The physics shape for the shield is a very simplified version of the visual shape.
    // Since the game is effectively 2D for gameplay purposes, the shape rather than being an ellipse is
    // a cylinder with distinct major and minor axis.

    const int maxPoints = 40;
    const int numVertices = 40 * 2;
    ConvexHullVertices vertexData;
    vertexData.reserve( numVertices );

    float height[ 2 ] = { -30.0f, 30.0f };

    const float segmentAperture = Genesis::kPi * 2.0f / maxPoints;
    float theta = segmentAperture;

    for ( int j = 0; j < 2; ++j )
    {
        const float z = height[ j ];

        for ( int i = 0; i < maxPoints; ++i )
        {
            const float c = cosf( theta );
            const float s = sinf( theta );
            vertexData.emplace_back( c * radiusX, s * radiusY, z );
            theta += segmentAperture;
        }
    }

    ConvexHullShapeSharedPtr pConvexHullShape = std::make_shared<ConvexHullShape>( vertexData );
    pConvexHullShape->SetUserData( m_pCollisionInfo.get() );
    m_pGhost = std::make_unique<Ghost>( pConvexHullShape, glm::translate( translation ) );
}

// displayAmount represents how large the visual impact on the shield is.
// frameAmount is the actual damage the shield takes this frame.
// These two values are kept deliberately separate to make sure all weapons have a visually pleasant
// effect when the hit a shield.
void Shield::ApplyDamage( float displayAmount, float frameAmount, float angle, WeaponSystem weaponSystem, DamageType damageType, Ship* pDealtBy )
{
    if ( m_State == ShieldState::Activated )
    {
        // Overload the shield on EMP strike or through the Ion cannon's perk
        if ( damageType == DamageType::EMP || ( weaponSystem == WeaponSystem::Ion && pDealtBy->HasPerk( Perk::Overload ) && gRand() <= ( 0.025f / 60.0f ) ) )
        {
            displayAmount = frameAmount = GetCurrentHealthPoints() * 2.0f;
        }
        else
        {
            if ( damageType == DamageType::Energy && pDealtBy->HasPerk( Perk::PhaseSynchronisation ) )
            {
                displayAmount *= 1.15f;
                frameAmount *= 1.15f;
            }

            if ( damageType == DamageType::Energy && m_pOwner->HasPerk( Perk::FrequencyCycling ) )
            {
                displayAmount *= 0.85f;
                frameAmount *= 0.85f;
            }
            else if ( damageType == DamageType::Kinetic && m_pOwner->HasPerk( Perk::KineticHardening ) )
            {
                displayAmount *= 0.85f;
                frameAmount *= 0.85f;
            }
        }

        AddonQuantumStateAlternator* pAlternator = m_pOwner->GetQuantumStateAlternator();
        if ( pAlternator != nullptr && pAlternator->GetModule()->IsDestroyed() == false )
        {
            const float multiplier = pAlternator->GetShieldResistance();
            displayAmount *= multiplier;
            frameAmount *= multiplier;
        }

        m_CurrentHitPoints -= frameAmount;

        m_HitRegistry.Hit( angle, displayAmount );

        if ( m_CurrentHitPoints < 0.0f )
        {
            if ( m_pOwner->HasPerk( Perk::EmergencyCapacitors ) && m_EmergencyCapacitorsCooldown <= 0.0f )
            {
                g_pGame->AddIntel( GameCharacter::FleetIntelligence, "Emergency capacitors discharged!", false );
                m_EmergencyCapacitorsCooldown = 120.0f;
                m_CurrentHitPoints = m_MaximumHitPoints * 0.6f;
            }
            else
            {
                Deactivate();

                // Overtuned shield modules are destroyed if the shield receives enough damage to be taken down
                const ShieldModuleList& shieldModules = m_pOwner->GetShieldModules();
                for ( auto& pShieldModule : shieldModules )
                {
                    ShieldInfo* pShieldInfo = static_cast<ShieldInfo*>( pShieldModule->GetModuleInfo() );
                    if ( pShieldInfo->IsOvertuned() )
                    {
                        pShieldModule->ApplyDamage( 1000000.0f, DamageType::TrueDamage, nullptr );
                    }
                }
            }
        }
    }
}

void Shield::Deactivate()
{
    if ( m_State != ShieldState::Deactivating && m_State != ShieldState::Deactivated )
    {
        m_State = ShieldState::Deactivating;
        m_CurrentHitPoints = 0.0f;
        m_DeactivatedTimer = sShieldOfflineDuration;
        g_pGame->GetPhysicsSimulation()->Remove( m_pGhost.get() );
    }
}

float Shield::CalculateEfficiency( const ShieldModuleList& shieldModules )
{
    float efficiency = 1.0f;
    const size_t numShieldModules = shieldModules.size();
    for ( size_t i = 1; i < numShieldModules; ++i )
    {
        efficiency *= 0.85f;
    }
    return efficiency;
}

} // namespace Hexterminate