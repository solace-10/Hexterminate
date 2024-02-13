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

#include <genesis.h>
#include <glm/gtc/matrix_access.hpp>
#include <math/constants.h>
#include <math/misc.h>
#include <physics/rigidbody.h>
#include <render/debugrender.h>
#include <resourcemanager.h>
#include <sound/soundinstance.h>
#include <sound/soundmanager.h>

#include "ammo/ammo.h"
#include "ammo/ammomanager.h"
#include "hexterminate.h"
#include "menus/shiptweaks.h"
#include "player.h"
#include "sector/sector.h"
#include "ship/hyperspacecore.h"
#include "ship/ship.h"
#include "ship/weapon.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// Weapon
///////////////////////////////////////////////////////////////////////////////

Weapon::Weapon( Ship* pOwner, Module* pModule, WeaponInfo* pInfo, const WeaponHardpoint& hardpoint )
    : m_pOwner( pOwner )
    , m_pModule( pModule )
    , m_pInfo( pInfo )
    , m_Hardpoint( hardpoint )
    , m_pWeaponModel( nullptr )
    , m_Angle( 0.0f )
    , m_AngleWorld( 0.0f )
    , m_ReloadTimer( 0.0f )
    , m_CurrentMuzzle( 0 )
    , m_BulletsToFire( 0 )
    , m_BurstTimer( 0.0f )
    , m_FiringMode( WeaponFiringMode::SingleShot )
    , m_IsFiring( false )
    , m_pCurrentAmmo( nullptr )
    , m_RateOfFire( 1.0f )
    , m_ContinuousFireTimer( 0.0f )
    , m_SiegebreakerStacks( 0 )
{
    if ( pInfo->GetWeaponModel() != "" )
    {
        m_pWeaponModel = (Genesis::ResourceModel*)Genesis::FrameWork::GetResourceManager()->GetResource( pInfo->GetWeaponModel() );
        m_pWeaponModel->SetFlipAxis( false );
    }

    SetupMuzzles();

    m_TargetPosition = glm::vec3( 0.0f );
    m_WorldTransform = glm::mat4x4( 1.0f );

    if ( pInfo->GetIsSwarm() )
    {
        m_FiringMode = WeaponFiringMode::Swarm;
    }
    else if ( pInfo->GetSystem() == WeaponSystem::Ion )
    {
        m_FiringMode = WeaponFiringMode::Continuous;
    }
    else if ( pInfo->GetBurst() > 1 && pInfo->GetSystem() == WeaponSystem::Rocket )
    {
        m_FiringMode = WeaponFiringMode::RandomBurstFire;
    }
    else if ( pInfo->GetBurst() > 1 )
    {
        m_FiringMode = WeaponFiringMode::BurstFire;
    }
    else
    {
        m_FiringMode = WeaponFiringMode::SingleShot;
    }

    m_RateOfFire = m_pInfo->GetRateOfFire( m_pOwner );
}

Weapon::~Weapon()
{
    if ( m_pContinuousSFX != nullptr )
    {
        m_pContinuousSFX->Stop();
        m_pContinuousSFX = nullptr;
    }
}

void Weapon::Render( const glm::mat4& modelTransform )
{
    if ( m_pWeaponModel != nullptr )
    {
        glm::mat4 localTransform = glm::rotate( glm::translate( m_Hardpoint.offset ), m_Angle, glm::vec3( 0.0f, 0.0f, 1.0f ) );
        m_pWeaponModel->Render( modelTransform * localTransform );
    }
}

void Weapon::Update( float delta )
{
    if ( m_ReloadTimer > 0.0f )
    {
        m_ReloadTimer -= delta;
    }

    HyperspaceCore* pHyperspaceCore = GetOwner()->GetHyperspaceCore();
    if ( m_BulletsToFire > 0 && ( pHyperspaceCore == nullptr || pHyperspaceCore->IsJumping() == false ) )
    {
        if ( m_FiringMode == WeaponFiringMode::SingleShot )
            FireSingle( delta );
        else if ( m_FiringMode == WeaponFiringMode::BurstFire )
            FireBurst( delta );
        else if ( m_FiringMode == WeaponFiringMode::RandomBurstFire )
            FireRandomBurst( delta );
        else if ( m_FiringMode == WeaponFiringMode::Swarm )
            FireSwarm( delta );
        else if ( m_FiringMode == WeaponFiringMode::Continuous )
            FireContinuous( delta );
    }
    else if ( IsFiring() )
    {
        StopFiring();
    }

    // Rotate turret to target
    glm::mat4x4 ownerTransform = m_pOwner->GetRigidBody()->GetWorldTransform();
    glm::vec3 ownerForward( glm::column( ownerTransform, 1 ) );
    float ownerAngle = atan2f( ownerForward.x, ownerForward.y );

    const glm::vec3 moduleLocalPosition = m_pModule->GetLocalPosition();

    glm::mat4x4 turretLocalTransform = glm::rotate( m_Angle, glm::vec3( 0.0f, 0.0f, 1.0f ) );
    turretLocalTransform = glm::column( turretLocalTransform, 3, glm::vec4( m_Hardpoint.offset + moduleLocalPosition, 1.0f ) );

    m_WorldTransform = ownerTransform * turretLocalTransform;

    const bool inShipyard = m_pOwner->GetDockingState() != DockingState::Undocked;
    const bool jumpingIn = pHyperspaceCore != nullptr && pHyperspaceCore->IsJumping() && pHyperspaceCore->GetJumpDirection() == HyperspaceJumpDirection::JumpIn;

    if ( m_Hardpoint.isTurret && g_pGame->IsShipCaptureModeActive() == false && !jumpingIn && !inShipyard )
    {
        glm::vec3 turretOrigin( glm::column( m_WorldTransform, 3 ) );
        glm::vec3 turretForward( glm::column( m_WorldTransform, 1 ) );

        // Normalize our wanted vector
        glm::vec3 turretWanted = glm::normalize( m_TargetPosition - turretOrigin );

        // TODO: Interpolate the turretForward to turretWanted
        float wantedAngle = atan2( turretWanted.x, turretWanted.y );

        m_AngleWorld = wantedAngle;
        m_Angle = ownerAngle - wantedAngle; // world angle to local angle
    }
    else
    {
        m_AngleWorld = ownerAngle;
        m_Angle = 0.0f;
    }

    if ( g_pGame->GetCurrentSector()->GetShipTweaks()->GetDrawTurrets() )
    {
        glm::vec3 turretOrigin( glm::column( m_WorldTransform, 3 ) );
        glm::vec3 turretForward( glm::column( m_WorldTransform, 1 ) );

        glm::vec3 turretForwardEnd = turretOrigin + turretForward * 10.0f;

        // Turret's forward vector
        Genesis::FrameWork::GetDebugRender()->DrawLine(
            turretOrigin,
            turretForwardEnd,
            glm::vec3( 1.0f, 1.0f, 1.0f ) );

        // Turret's target
        if ( m_Hardpoint.isTurret )
        {
            Genesis::FrameWork::GetDebugRender()->DrawLine(
                turretOrigin,
                m_TargetPosition,
                glm::vec3( 1.0f, 0.0f, 0.0f ) );
        }
    }
}

void Weapon::FireSingle( float delta )
{
    g_pGame->GetCurrentSector()->GetAmmoManager()->Create( this );
    m_BulletsToFire--;

    PlayFireSFX();
}

void Weapon::FireBurst( float delta )
{
    m_BurstTimer -= delta;
    if ( m_BurstTimer <= 0.0f )
    {
        m_BulletsToFire--;
        g_pGame->GetCurrentSector()->GetAmmoManager()->Create( this );
        m_BurstTimer = 0.075f;

        PlayFireSFX();
    }
}

void Weapon::FireRandomBurst( float delta )
{
    m_BurstTimer -= delta;
    if ( m_BurstTimer <= 0.0f )
    {
        m_BulletsToFire--;
        g_pGame->GetCurrentSector()->GetAmmoManager()->Create( this, gRand( -22.5f, 22.5f ) );
        m_BurstTimer = 0.075f;

        PlayFireSFX();
    }
}

void Weapon::FireSwarm( float delta )
{
    const float aperture = 90.0f;
    float additionalRotation = -aperture / 2.0f + 180.0f;
    float angleStep = aperture / m_BulletsToFire;
    for ( int i = 0; i < m_BulletsToFire; ++i )
    {
        g_pGame->GetCurrentSector()->GetAmmoManager()->Create( this, additionalRotation );
        additionalRotation += angleStep;
    }
    m_BulletsToFire = 0;

    PlayFireSFX();
}

void Weapon::FireContinuous( float delta )
{
    if ( m_pCurrentAmmo == nullptr )
    {
        m_pCurrentAmmo = g_pGame->GetCurrentSector()->GetAmmoManager()->Create( this );
        m_pContinuousSFX = PlayFireSFX();
    }

    // We have a maximum amount of time a continuous firing weapon can be active, so we don't have
    // ships just firing their ion cannons forever in one endless stream.
    static const float sMaxContinuousFireDuration = 5.0f;
    if ( m_ContinuousFireTimer < sMaxContinuousFireDuration )
    {
        m_ContinuousFireTimer += delta;
    }

    if ( !m_pOwner->ConsumeEnergy( m_pInfo->GetActivationCost( GetOwner() ) * delta ) || m_ContinuousFireTimer >= sMaxContinuousFireDuration )
    {
        StopFiring();
    }
}

Genesis::Sound::SoundInstanceSharedPtr Weapon::PlayFireSFX()
{
    using namespace Genesis;

    ResourceSound* pSFX = GetInfo()->GetOnFireSFX();
    if ( pSFX != nullptr )
    {
        const glm::vec3 position( glm::column( m_WorldTransform, 3 ) );
        const bool isPlayerShip = ( g_pGame->GetPlayer()->GetShip() == m_pOwner ); // Makes the player's ship sound more important and not get drowned by other ships firing
        return FrameWork::GetSoundManager()->CreateSoundInstance( pSFX, Genesis::Sound::SoundBus::Type::SFX, position, isPlayerShip ? 750.0f : 400.0f );
    }

    return nullptr;
}

void Weapon::TurnTowards( const glm::vec3& position )
{
    m_TargetPosition = position;
}

float Weapon::GetAngle() const
{
    return m_AngleWorld;
}

void Weapon::Fire()
{
    if ( !CanFire() )
        return;

    if ( m_FiringMode != WeaponFiringMode::Continuous && !m_pOwner->ConsumeEnergy( m_pInfo->GetActivationCost( GetOwner() ) ) )
        return;

    m_BulletsToFire = m_pInfo->GetBurst();
    m_IsFiring = true;

    // Continuous firing weapons only being their reload -after- they stop firing.
    // Other weapons being their reload timing immediately as I don't want the burst properties / beam decay rate to affect the overall DPS.
    if ( m_FiringMode != WeaponFiringMode::Continuous )
    {
        // The rate of fire increases the more siegebreaker stacks this weapon has.
        const float siegebreakerBonus = logf( 1.0f + static_cast<float>( m_SiegebreakerStacks ) * 0.025f );
        const float rateOfFire = m_RateOfFire * ( 1.0f + siegebreakerBonus );

        // Genesis::FrameWork::GetLogger()->LogInfo( "Siegebreaker stacks: %d | RoF bonus: %.2f | rate of fire: %.2f", m_SiegebreakerStacks, siegebreakerBonus, rateOfFire );

        m_ReloadTimer = 1.0f / rateOfFire;
    }
}

void Weapon::StopFiring()
{
    if ( !IsFiring() )
        return;

    m_IsFiring = false;

    if ( m_pCurrentAmmo != nullptr )
    {
        if ( m_FiringMode == WeaponFiringMode::Continuous )
        {
            m_pCurrentAmmo->Kill();
            m_ReloadTimer = 1.0f;
            m_BulletsToFire = 0;
            m_ContinuousFireTimer = 0.0f;
        }

        m_pCurrentAmmo = nullptr;
    }

    if ( m_pContinuousSFX != nullptr )
    {
        m_pContinuousSFX->Stop();
        m_pContinuousSFX = nullptr;
    }
}

void Weapon::SetupMuzzles()
{
    // Use the module's model if we don't have a turret
    Genesis::ResourceModel* pModel = m_pWeaponModel ? m_pWeaponModel : m_pModule->GetModel();

    for ( int muzzleIndex = 1; muzzleIndex <= 10; ++muzzleIndex )
    {
        std::stringstream ss;
        ss << "muzzle" << muzzleIndex;
        glm::vec3 muzzleOffset;
        if ( pModel->GetDummy( ss.str(), &muzzleOffset ) )
        {
            AddMuzzle( muzzleOffset );
        }
        else
        {
            break;
        }
    }

    // Safeguard if the model has not been set up properly.
    // We always need at least one muzzle for the ammo to ejected from.
    if ( m_MuzzleOffsets.empty() )
    {
        Genesis::FrameWork::GetLogger()->LogInfo( "Weapon '%s' has no valid muzzles!", m_pInfo->GetName().c_str() );
        AddMuzzle( glm::vec3( 0.0f, 0.0f, 0.0f ) );
    }
}

void Weapon::AddMuzzle( const glm::vec3& position )
{
    m_MuzzleOffsets.push_back( position );
}

void Weapon::MarkMuzzleAsUsed()
{
    const size_t validMuzzles = m_MuzzleOffsets.size();
    if ( validMuzzles > 1 && ++m_CurrentMuzzle >= validMuzzles )
    {
        m_CurrentMuzzle = 0;
    }
}

bool Weapon::IsTurnedTowards( const glm::vec3& position, float threshold /* = 10.0f */ ) const
{
    glm::vec3 weaponForward( glm::column( m_WorldTransform, 1 ) );
    glm::vec3 weaponTranslation( glm::column( m_WorldTransform, 3 ) );
    const glm::vec3 forwardToTarget = glm::normalize( position - weaponTranslation );

    const float dot = glm::dot( weaponForward, forwardToTarget );
    const float angle = glm::acos( dot ) * Genesis::kRadToDeg;

    return ( angle < threshold );
}

} // namespace Hexterminate