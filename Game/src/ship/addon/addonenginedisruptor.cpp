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

#include "ship/addon/addonenginedisruptor.h"
#include "faction/faction.h"
#include "hexterminate.h"
#include "laser/laser.h"
#include "laser/lasermanager.h"
#include "menus/shiptweaks.h"
#include "particles/particleemitter.h"
#include "particles/particlemanager.h"
#include "sector/sector.h"

#include <genesis.h>
#include <math/constants.h>
#include <math/misc.h>
#include <physics/shape.h>
#include <physics/simulation.h>
#include <render/debugrender.h>
#include <resources/resourcesound.h>
#include <sound/soundinstance.h>
#include <sound/soundmanager.h>

namespace Hexterminate
{

AddonEngineDisruptor::AddonEngineDisruptor( AddonModule* pModule, Ship* pOwner )
    : Addon( pModule, pOwner )
    , m_pSFX( nullptr )
    , m_ShockwaveMaximumRadius( 30.0f )
    , m_ShockwaveParticleScale( 0.5f )
    , m_TimesToFire( 0 )
    , m_TimeToNextShot( 0.0f )
{
    AddonInfo* pAddonInfo = static_cast<AddonInfo*>( pModule->GetModuleInfo() );
    m_ShockwaveMaximumRadius = static_cast<float>( atof( pAddonInfo->GetParameter().c_str() ) ) / 2.0f;
    m_ShockwaveParticleScale = m_ShockwaveMaximumRadius / 60.0f; // Some nonsense with the particle emitter. Scale of 1 equals a radius of 60 in-game units.

    for ( int i = 0; i < sEngineDisruptorShockwaveCount; ++i )
    {
        m_ShockwaveTimers[ i ] = 0.0f;
        m_DisruptorLasers[ i ].m_Timer = 0.0f;
    }

    LoadSFX();
}

void AddonEngineDisruptor::LoadSFX()
{
    using namespace Genesis;
    using namespace std::literals;

    m_pSFX = (ResourceSound*)FrameWork::GetResourceManager()->GetResource( "data/sfx/engine_disruptor.wav" );
    if ( m_pSFX != nullptr )
    {
        m_pSFX->Initialise( SOUND_FLAG_3D | SOUND_FLAG_FX );
        m_pSFX->SetInstancingLimit( 100ms );
    }
}

void AddonEngineDisruptor::Activate()
{
    Addon::Activate();

    using namespace Genesis;
    const glm::vec2& mousePosition = FrameWork::GetInputManager()->GetMousePosition();
    m_DisruptorAnchor = FrameWork::GetRenderSystem()->Raycast( mousePosition );

    m_TimesToFire = sEngineDisruptorShockwaveCount;
    m_TimeToNextShot = 0.0f;
}

void AddonEngineDisruptor::Update( float delta )
{
    Addon::Update( delta );

    UpdateFiring( delta );
    UpdateLaser( delta );
    UpdateShockwave( delta );
}

void AddonEngineDisruptor::UpdateFiring( float delta )
{
    if ( m_TimeToNextShot <= 0.0f && m_TimesToFire > 0 )
    {
        DisruptorLaser& disruptorLaser = m_DisruptorLasers.at( m_TimesToFire - 1 );
        const float angle = gRand( -Genesis::kPi, Genesis::kPi );
        const float distance = gRand( 0.0f, m_ShockwaveMaximumRadius );
        disruptorLaser.m_Target = m_DisruptorAnchor + glm::vec3( cosf( angle ) * distance, sinf( angle ) * distance, 0.0f );
        disruptorLaser.m_Timer = 1.0f;

        const unsigned int atlasElementSize = 512;
        ParticleManager* pParticleManager = g_pGame->GetCurrentSector()->GetParticleManager();
        ParticleEmitter* pEmitter = pParticleManager->GetAvailableEmitter();
        pEmitter->SetBlendMode( Genesis::BlendMode::Add );
        pEmitter->SetParticleCount( 1 );
        pEmitter->SetEmissionDelay( 0.0f );
        pEmitter->SetPosition( disruptorLaser.m_Target );
        pEmitter->SetTextureAtlas( "data/particles/Plasma_Ring_v1.png", atlasElementSize, atlasElementSize, 50 );
        pEmitter->SetScale( m_ShockwaveParticleScale, m_ShockwaveParticleScale );
        pEmitter->SetLifetime( sEngineDisruptorShockwaveDuration, sEngineDisruptorShockwaveDuration );
        pEmitter->Start();

        m_ShockwaveTimers.at( m_TimesToFire - 1 ) = sEngineDisruptorShockwaveDuration;

        PlaySFX();

        m_TimeToNextShot = 0.5f;
        m_TimesToFire--;
    }

    m_TimeToNextShot = gMax( 0.0f, m_TimeToNextShot - delta );
}

void AddonEngineDisruptor::UpdateLaser( float delta )
{
    LaserManager* pLaserManager = g_pGame->GetCurrentSector()->GetLaserManager();

    for ( auto& disruptorLaser : m_DisruptorLasers )
    {
        if ( disruptorLaser.m_Timer > 0.0f )
        {
            const float laserWidth = disruptorLaser.m_Timer / sEngineDisruptorLaserDuration * 4.0f;
            if ( laserWidth > 0.0f )
            {
                disruptorLaser.m_Laser = Laser(
                    m_pModule->GetWorldPosition(),
                    glm::vec3( disruptorLaser.m_Target.x, disruptorLaser.m_Target.y, 0.0f ),
                    Genesis::Color( 0.55f, 0.89f, 1.0f ),
                    laserWidth );

                pLaserManager->AddLaser( disruptorLaser.m_Laser );
            }

            disruptorLaser.m_Timer -= delta;
        }
    }
}

void AddonEngineDisruptor::UpdateShockwave( float delta )
{
    for ( int i = 0; i < sEngineDisruptorShockwaveCount; ++i )
    {
        if ( m_ShockwaveTimers[ i ] > 0.0f )
        {
            // The visual radius is an attempt to match the visual effect of the expanding shockwave (as per the particle atlas)
            const float visualRadius = ( 1.0f - ( m_ShockwaveTimers[ i ] / sEngineDisruptorShockwaveDuration ) ) * m_ShockwaveMaximumRadius * 0.8f;
            m_ShockwaveTimers[ i ] -= delta;

            // visualRadius can be 0, which would generate a 0-length raytest.
            if ( visualRadius < std::numeric_limits<float>::epsilon() )
            {
                continue;
            }

            // Knowing the visual radius of the shockwave, we need to detect whether the shockwave has collided with any enemy ships.
            // To achieve this we just do a line check. The line always goes through the centre of the shockwave and touches both
            // ends in a random direction.
            const float angle = gRand( -Genesis::kPi, Genesis::kPi );
            const float c = cosf( angle );
            const float s = sinf( angle );
            const glm::vec3 disruptorTarget( m_DisruptorLasers[ i ].m_Target.x, m_DisruptorLasers[ i ].m_Target.y, 0.0f );
            glm::vec3 p1( disruptorTarget.x + c * visualRadius, disruptorTarget.y + s * visualRadius, 0.0f );
            glm::vec3 p2( disruptorTarget.x - c * visualRadius, disruptorTarget.y - s * visualRadius, 0.0f );

#ifdef _DEBUG
            if ( g_pGame->GetCurrentSector()->GetShipTweaks()->GetDrawEngineDisruptor() )
            {
                Genesis::Render::DebugRender* pDebugRender = Genesis::FrameWork::GetDebugRender();
                pDebugRender->DrawCircle( disruptorTarget, visualRadius, glm::vec3( 1.0f, 0.0f, 0.0f ) );
                pDebugRender->DrawLine( p1, p2, glm::vec3( 1.0f, 1.0f, 0.0f ) );
            }
#endif

            Genesis::Physics::RayTestResultVector rayTestResults;
            g_pGame->GetPhysicsSimulation()->RayTest( p1, p2, rayTestResults );

            for ( auto& result : rayTestResults )
            {
                // If it's a compound shape, then the ShipCollisionInfo is in the child shape.
                Genesis::Physics::ShapeSharedPtr pShape = !result.GetChildShape().expired() ? result.GetChildShape().lock() : result.GetShape().lock();
                ShipCollisionInfo* pCollisionInfo = reinterpret_cast<ShipCollisionInfo*>( pShape->GetUserData() );
                if ( pCollisionInfo->GetType() == ShipCollisionType::Module )
                {
                    pCollisionInfo->GetShip()->DisruptEngines();
                }
            }
        }
    }
}

void AddonEngineDisruptor::PlaySFX()
{
    if ( m_pSFX )
    {
        using namespace Genesis;

        Sound::SoundInstanceSharedPtr pSoundInstance = FrameWork::GetSoundManager()->CreateSoundInstance( m_pSFX, Genesis::Sound::SoundBus::Type::SFX, m_pModule->GetWorldPosition(), 300.0f );
    }
}

} // namespace Hexterminate