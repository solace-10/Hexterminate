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

#include "ship/addon/addonparticleaccelerator.h"
#include "achievements.h"
#include "faction/faction.h"
#include "hexterminate.h"
#include "laser/laser.h"
#include "laser/lasermanager.h"
#include "menus/contextualtips.h"
#include "particles/particleemitter.h"
#include "particles/particlemanager.h"
#include "player.h"
#include "sector/sector.h"
#include "ship/controller/controllerai.h"
#include "ship/shield.h"

#include <genesis.h>
#include <math/constants.h>
#include <math/misc.h>
#include <physics/rigidbody.h>
#include <physics/shape.h>
#include <physics/simulation.h>
#include <render/debugrender.h>
#include <resources/resourcesound.h>
#include <sound/soundinstance.h>
#include <sound/soundmanager.h>

// clang-format off
#include <beginexternalheaders.h>
#include <glm/gtc/matrix_access.hpp>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <endexternalheaders.h>
// clang-format on

namespace Hexterminate
{

AddonParticleAccelerator::AddonParticleAccelerator( AddonModule* pModule, Ship* pOwner )
    : Addon( pModule, pOwner )
    , m_pSFX( nullptr )
    , m_TimesToFire( 0 )
    , m_TimeToNextShot( 0.0f )
    , m_Stage( Stage::Done )
{
    for ( int i = 0; i < sParticleAcceleratorLaserCount; ++i )
    {
        m_ParticleAcceleratorLasers[ i ].m_Timer = 0.0f;
    }

    pModule->GetModel()->GetDummy( "dummy", &m_EmitterOffset );

    m_pSFX = Genesis::ResourceSound::LoadAs3D( "data/sfx/particle_accelerator.wav" );

    // Contextual tip to notify the player of how dangerous particle accelerators are.
    // This tip ignores the "no contextual tips" setting.
    if ( m_pOwner->GetFaction()->GetFactionId() != FactionId::Player )
    {
        ContextualTips::Present( ContextualTipType::EnterSectorWithParticleAccelerator, true );
    }
}

bool AddonParticleAccelerator::CanUse() const
{
    bool canUse = Addon::CanUse();

    // Coarse additional check so the AI doesn't attempt to use this module when it doesn't have a target or when it isn't close enough.
    if ( canUse && m_pOwner != g_pGame->GetPlayer()->GetShip() )
    {
        ControllerAI* pController = static_cast<ControllerAI*>( m_pOwner->GetController() );
        Ship* pTargetShip = pController->GetTargetShip();
        if ( pTargetShip == nullptr || glm::distance( pTargetShip->GetTowerPosition(), m_pOwner->GetTowerPosition() ) > 700.0f )
        {
            canUse = false;
        }
    }
    return canUse;
}

// Activating the particle accelerator displays an aiming laser for a few seconds before actually firing.
void AddonParticleAccelerator::Activate()
{
    Addon::Activate();

    m_TimesToFire = sParticleAcceleratorLaserCount;
    m_TimeToNextShot = 0.0f;

    m_AimingLaser.m_Duration = 2.75f;
    m_AimingLaser.m_Timer = 2.75f;
    m_AimingLaser.m_Width = 4.0f;
    m_AimingLaser.m_Offset = 0.0f;
    m_AimingLaser.m_Colour = Genesis::Color( 1.0f, 0.0f, 0.0f );

    m_Stage = Stage::Aiming;

    // The sound has been authored to have the correct duration to match the aiming + firing stages.
    PlaySFX();
}

void AddonParticleAccelerator::Update( float delta )
{
    Addon::Update( delta );

    UpdateFiring( delta );
    UpdateLaser( delta );
}

// The actual firing is composed of many small lasers but this is just a visual effect.
void AddonParticleAccelerator::UpdateFiring( float delta )
{
    if ( m_Stage == Stage::Firing && m_TimeToNextShot <= 0.0f && m_TimesToFire > 0 )
    {
        // The normal distribution should use float, but because of an implementation bug in VS2017 this
        // generates a conversion warning.
        std::normal_distribution<double> distribution( 0.0, 3.0 );

        ParticleLaser& particleLaser = m_ParticleAcceleratorLasers.at( m_TimesToFire - 1 );
        particleLaser.m_Duration = particleLaser.m_Timer = gRand( 0.25f, 1.25f );
        particleLaser.m_Width = gRand( 3.0f, 6.0f );
        particleLaser.m_Offset = gClamp( static_cast<float>( distribution( m_RandomEngine ) ), -6.0f, 6.0f );
        particleLaser.m_Colour = ( gRand( 0, 3 ) > 0 ) ? Genesis::Color( 0.0f, 0.5f, 1.0f ) : Genesis::Color( 1.0f, 0.5f, 0.1f );

        m_TimeToNextShot = gRand( 0.0f, 0.05f );
        m_TimesToFire--;
    }

    m_TimeToNextShot = gMax( 0.0f, m_TimeToNextShot - delta );
}

void AddonParticleAccelerator::UpdateLaser( float delta )
{
    LaserManager* pLaserManager = g_pGame->GetCurrentSector()->GetLaserManager();

    glm::vec3 source, target, right;
    CalculateVectors( source, target, right );

    const float hitFraction = ProcessCollision( false, delta );
    target = glm::mix( source, target, hitFraction );

    if ( m_Stage == Stage::Aiming )
    {
        m_AimingLaser.m_Laser = Laser(
            source,
            target,
            m_AimingLaser.m_Colour,
            m_AimingLaser.m_Width );

        pLaserManager->AddLaser( m_AimingLaser.m_Laser );

        m_AimingLaser.m_Timer -= delta;
        if ( m_AimingLaser.m_Timer <= 0.0f )
        {
            m_Stage = Stage::Firing;
            ProcessCollision( true, delta );
        }
    }
    else if ( m_Stage == Stage::Firing )
    {
        for ( auto& particleLaser : m_ParticleAcceleratorLasers )
        {
            if ( particleLaser.m_Timer > 0.0f )
            {
                const float laserWidth = particleLaser.m_Timer / particleLaser.m_Duration * particleLaser.m_Width;
                if ( laserWidth > 0.0f )
                {
                    glm::vec3 sourceOffset = source + right * particleLaser.m_Offset;
                    glm::vec3 targetOffset = target + right * particleLaser.m_Offset;

                    particleLaser.m_Laser = Laser(
                        sourceOffset,
                        targetOffset,
                        particleLaser.m_Colour,
                        laserWidth );

                    pLaserManager->AddLaser( particleLaser.m_Laser );
                }

                particleLaser.m_Timer -= delta;
            }
        }
    }
}

void AddonParticleAccelerator::CalculateVectors( glm::vec3& source, glm::vec3& target, glm::vec3& right ) const
{
    glm::mat4x4 moduleWorldTransform = m_pOwner->GetRigidBody()->GetWorldTransform();
    glm::vec3 emitterLocalTranslation = m_pModule->GetLocalPosition() + m_EmitterOffset;
    glm::mat4x4 emitterLocalTransform = glm::translate( emitterLocalTranslation );

    glm::mat4x4 emitterWorldTransform = moduleWorldTransform * emitterLocalTransform;

    source = glm::vec3( glm::column( emitterWorldTransform, 3 ) );
    glm::vec3 forward( glm::column( emitterWorldTransform, 1 ) );
    right = glm::vec3( glm::column( emitterWorldTransform, 0 ) );
    target = source;
    target += forward * 1000.0f;
}

// ProcessCollision returns the intersection point between the laser and a ship.
// It can optionally deal damage to the shield / modules it hits.
// If it hits a shield it stops any further processing as the particle accelerator
// is meant to either destroy modules or deal loads of damage to the shields but
// not both at the same time.
float AddonParticleAccelerator::ProcessCollision( bool appliesDamage, float delta )
{
    glm::vec3 source = glm::vec3( 0.0f );
    glm::vec3 target = glm::vec3( 0.0f );
    glm::vec3 right = glm::vec3( 0.0f );

    CalculateVectors( source, target, right );

    Genesis::Physics::RayTestResultVector rayTestResults;
    g_pGame->GetPhysicsSimulation()->RayTest( source, target, rayTestResults );
    float hitFraction = 1.0f;

    for ( auto& result : rayTestResults )
    {
        // If it's a compound shape, then the ShipCollisionInfo is in the child shape.
        Genesis::Physics::ShapeSharedPtr pShape = !result.GetChildShape().expired() ? result.GetChildShape().lock() : result.GetShape().lock();
        ShipCollisionInfo* pCollisionInfo = reinterpret_cast<ShipCollisionInfo*>( pShape->GetUserData() );
        if ( pCollisionInfo == nullptr || pCollisionInfo->GetShip() == m_pOwner )
        {
            continue;
        }
        else if ( pCollisionInfo->GetType() == ShipCollisionType::PhaseBarrier )
        {
            g_pGame->GetAchievementsManager()->UnlockAchievement( ACH_NOT_TODAY );
            hitFraction = result.GetFraction();
            break;
        }
        else if ( pCollisionInfo->GetType() == ShipCollisionType::Shield )
        {
            Shield* pShield = pCollisionInfo->GetShield();
            if ( pShield && pShield->GetQuantumState() == ShieldState::Activated )
            {
                hitFraction = result.GetFraction();

                if ( appliesDamage )
                {
                    // The particle accelerator deals 60% of the shield's maximum HP.
                    // This guarantees that the particle accelerators remain a threat
                    // no matter what.
                    const float damage = pShield->GetMaximumHealthPoints() * 0.6f;
                    pCollisionInfo->GetShip()->DamageShield(
                        WeaponSystem::Universal,
                        DamageType::Energy,
                        damage,
                        1,
                        m_pOwner,
                        delta,
                        result.GetPosition() );
                }

                break;
            }
        }
        else if ( appliesDamage && pCollisionInfo->GetType() == ShipCollisionType::Module )
        {
            pCollisionInfo->GetShip()->DamageModule(
                WeaponSystem::Universal,
                DamageType::TrueDamage,
                10000.0f,
                1,
                m_pOwner,
                pCollisionInfo->GetModule(),
                delta );
        }
    }

    return hitFraction;
}

void AddonParticleAccelerator::PlaySFX()
{
    if ( m_pSFX )
    {
        using namespace Genesis;

        Sound::SoundInstanceSharedPtr pSoundInstance = FrameWork::GetSoundManager()->CreateSoundInstance( m_pSFX, Genesis::Sound::SoundBus::Type::SFX, m_pModule->GetWorldPosition(), 300.0f );
    }
}

} // namespace Hexterminate