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
#include <physics/raytestresult.h>
#include <physics/rigidbody.h>
#include <physics/shape.h>
#include <physics/simulation.h>
#include <resources/resourcemodel.h>
#include <resources/resourcesound.h>
#include <sound/soundinstance.h>
#include <sound/soundmanager.h>

#include "ammo/ammo.h"
#include "ammo/ammomanager.h"
#include "ammo/antiproton.h"
#include "ammo/beam.h"
#include "ammo/lance.h"
#include "ammo/missile.h"
#include "ammo/projectile.h"
#include "ammo/rocket.h"
#include "ammo/torpedo.h"
#include "hexterminate.h"
#include "particles/particleemitter.h"
#include "particles/particlemanager.h"
#include "sector/sector.h"
#include "ship/collisionmasks.h"

namespace Hexterminate
{

AmmoManager::AmmoManager()
    : m_Idx( 0 )
{
    const size_t sInitialCapacity = 1024u;
    m_Ammo.resize( sInitialCapacity );
    for ( int i = 0; i < sInitialCapacity; ++i )
    {
        m_Ammo[ i ] = nullptr;
    }

    m_RayTestResults.reserve( 64 );
};

AmmoManager::~AmmoManager()
{
    for ( int i = 0; i < m_Ammo.size(); ++i )
    {
        delete m_Ammo[ i ];
    }
}

AmmoHandle AmmoManager::GetFreeAmmo()
{
    AmmoHandle handle = InvalidAmmoHandle;

    const AmmoSizeType bufferSize = static_cast<AmmoSizeType>( m_Ammo.size() );
    const AmmoSizeType minIdx = m_Idx;
    const AmmoSizeType maxIdx = m_Idx + bufferSize;
    for ( AmmoHandle i = minIdx; i < maxIdx; i++ )
    {
        const AmmoHandle idx = i % bufferSize;
        if ( m_Ammo[ idx ] == nullptr || !m_Ammo[ idx ]->IsAlive() )
        {
            if ( m_Ammo[ idx ] && !m_Ammo[ idx ]->IsAlive() )
            {
                delete m_Ammo[ idx ];
                m_Ammo[ idx ] = nullptr;
            }

            m_Idx = idx;
            return idx;
        }
    }

    // If we get here, we've been through the full buffer and failed to find an empty slot.
    AmmoSizeType newBufferSize = bufferSize * 2;
    Genesis::FrameWork::GetLogger()->LogInfo( "AmmoManager buffer full, extended from %d to %d.", bufferSize, newBufferSize );
    m_Ammo.resize( newBufferSize );

    for ( AmmoSizeType idx = bufferSize; idx < newBufferSize; idx++ )
    {
        m_Ammo[ idx ] = nullptr;
    }

    m_Idx = bufferSize;
    return static_cast<AmmoHandle>( bufferSize );
}

AmmoHandle AmmoManager::Create( Weapon* pWeapon, float additionalRotation /* = 0.0f */ )
{
    AmmoHandle handle = GetFreeAmmo();

    WeaponSystem weaponSystem = pWeapon->GetInfo()->GetSystem();
    if ( weaponSystem == WeaponSystem::Projectile )
    {
        m_Ammo[ handle ] = new Projectile();
    }
    else if ( weaponSystem == WeaponSystem::Missile )
    {
        m_Ammo[ handle ] = new Missile();
    }
    else if ( weaponSystem == WeaponSystem::Rocket )
    {
        m_Ammo[ handle ] = new Rocket();
    }
    else if ( weaponSystem == WeaponSystem::Torpedo )
    {
        m_Ammo[ handle ] = new Torpedo();
    }
    else if ( weaponSystem == WeaponSystem::Ion )
    {
        m_Ammo[ handle ] = new Beam();
    }
    else if ( weaponSystem == WeaponSystem::Lance )
    {
        m_Ammo[ handle ] = new Lance();
    }
    else if ( weaponSystem == WeaponSystem::Antiproton )
    {
        m_Ammo[ handle ] = new Antiproton();
    }
    else
    {
        SDL_assert_release( false ); // Not implemented!
    }

    m_Ammo[ handle ]->Create( pWeapon, additionalRotation );
    return handle;
}

void AmmoManager::Update( float delta )
{
    if ( g_pGame->IsPaused() )
    {
        return;
    }

    for ( Ammo* pAmmo : m_Ammo )
    {
        if ( pAmmo && pAmmo->IsAlive() )
        {
            pAmmo->Update( delta );
        }
    }

    Genesis::Physics::Simulation* pPhysicsSimulation = g_pGame->GetPhysicsSimulation();
    for ( Ammo* pAmmo : m_Ammo )
    {
        if ( pAmmo != nullptr && pAmmo->IsAlive() )
        {
            if ( pAmmo->WasIntercepted() )
            {
                CreateHitEffect( pAmmo->GetSource(), glm::vec3( 1.0f, 0.0f, 0.0f ), pAmmo->GetOwner() );
                pAmmo->Kill();
                continue;
            }

            pPhysicsSimulation->RayTest( pAmmo->GetSource(), pAmmo->GetDestination(), m_RayTestResults );

            for ( auto& result : m_RayTestResults )
            {
                // If we've collided with a compound shape (the only shape that has child shapes) then the child
                // shape is the one that has relevant collision info.
                Genesis::Physics::ShapeSharedPtr pShape = !result.GetChildShape().expired() ? result.GetChildShape().lock() : result.GetShape().lock();
                SDL_assert( pShape != nullptr );

                // Only collide with hostile ships.
                ShipCollisionInfo* pCollisionInfo = reinterpret_cast<ShipCollisionInfo*>( pShape->GetUserData() );
                if ( pCollisionInfo == nullptr )
                {
                    continue;
                }

                Ship* pShip = pCollisionInfo->GetShip();
                Weapon* pOwnerWeapon = pAmmo->GetOwner();
                Ship* pOwnerShip = pOwnerWeapon->GetOwner();
                if ( Faction::sIsEnemyOf( pShip->GetFaction(), pOwnerShip->GetFaction() ) == false )
                {
                    continue;
                }

                if ( pCollisionInfo->GetType() == ShipCollisionType::Shield && pAmmo->CanBypassShields() )
                {
                    continue;
                }

                const glm::vec3 hitPosition = glm::mix( pAmmo->GetSource(), pAmmo->GetDestination(), result.GetFraction() );
                CreateHitEffect( hitPosition, result.GetNormal(), pAmmo->GetOwner() );

                bool stopProcessing = false;
                if ( pCollisionInfo->GetType() == ShipCollisionType::Module )
                {
                    Module* pModule = pCollisionInfo->GetModule();
                    int slotX, slotY;
                    pModule->GetHexGridSlot( slotX, slotY );
                    pShip->DamageModule( pOwnerWeapon, pCollisionInfo->GetModule(), delta );
                    stopProcessing = true;
                }
                else if ( pCollisionInfo->GetType() == ShipCollisionType::Shield )
                {
                    if ( pOwnerWeapon->GetInfo()->GetSystem() == WeaponSystem::Antiproton )
                    {
                        AddonQuantumStateAlternator* pAlternator = pShip->GetQuantumStateAlternator();
                        Antiproton* pAntiprotonAmmo = static_cast<Antiproton*>( pAmmo );
                        if ( pAlternator == nullptr || pAlternator->GetQuantumState() != pAntiprotonAmmo->GetQuantumState() )
                        {
                            continue;
                        }
                        else
                        {
                            stopProcessing = true;
                        }
                    }
                    else
                    {
                        pShip->DamageShield( pOwnerWeapon, delta, hitPosition );
                        stopProcessing = true;
                    }
                }
                else if ( pCollisionInfo->GetType() == ShipCollisionType::PhaseBarrier )
                {
                    stopProcessing = true;
                }

                pAmmo->SetHitFraction( result.GetFraction() );

                if ( pAmmo->GetDiesOnHit() )
                {
                    if ( pOwnerWeapon->GetInfo()->GetDamageType() == DamageType::Kinetic && pOwnerShip->HasPerk( Perk::Siegebreaker ) )
                    {
                        pAmmo->GetOwner()->AddSiegebreakerStack();
                    }

                    pAmmo->Kill();
                }

                if ( stopProcessing )
                {
                    break;
                }
            }
        }
    }
}

void AmmoManager::Render()
{
    Genesis::FrameWork::GetRenderSystem()->SetRenderTarget( Genesis::RenderTargetId::Glow );
    for ( Ammo* pAmmo : m_Ammo )
    {
        if ( pAmmo && pAmmo->IsAlive() && pAmmo->IsGlowSource() )
        {
            pAmmo->Render();
        }
    }

    Genesis::FrameWork::GetRenderSystem()->SetRenderTarget( Genesis::RenderTargetId::Default );
    for ( Ammo* pAmmo : m_Ammo )
    {
        if ( pAmmo && pAmmo->IsAlive() )
        {
            pAmmo->Render();
        }
    }
}

void AmmoManager::CreateHitEffect( const glm::vec3& position, const glm::vec3& hitNormal, Weapon* pWeapon )
{
    ParticleManager* pParticleManager = g_pGame->GetCurrentSector()->GetParticleManager();

    WeaponSystem weaponSystem = pWeapon->GetInfo()->GetSystem();
    if ( weaponSystem == WeaponSystem::Torpedo )
    {
        ParticleEmitter* pEmitter = pParticleManager->GetAvailableEmitter();
        pEmitter->SetBlendMode( Genesis::BlendMode::Blend );
        pEmitter->SetParticleCount( 1 );
        pEmitter->SetEmissionDelay( 0.0f );
        pEmitter->SetPosition( position );
        pEmitter->SetTextureAtlas( ParticleEmitter::GetRandomExplosion(), 512, 512, 64 );
        pEmitter->SetScale( 1.0f, 1.25f );
        pEmitter->SetLifetime( 2.0f, 2.5f );
        pEmitter->Start();
    }
    else if ( weaponSystem == WeaponSystem::Missile || weaponSystem == WeaponSystem::Rocket )
    {
        ParticleEmitter* pEmitter = pParticleManager->GetAvailableEmitter();
        pEmitter->SetBlendMode( Genesis::BlendMode::Blend );
        pEmitter->SetParticleCount( 1 );
        pEmitter->SetEmissionDelay( 0.0f );
        pEmitter->SetPosition( position );
        pEmitter->SetTextureAtlas( ParticleEmitter::GetRandomExplosion(), 512, 512, 64 );
        pEmitter->SetScale( 0.5f, 0.7f );
        pEmitter->SetLifetime( 1.0f, 1.5f );
        pEmitter->Start();
    }
    else
    {
        ParticleEmitter* pEmitter = pParticleManager->GetAvailableEmitter();
        pEmitter->SetBlendMode( Genesis::BlendMode::Add );
        pEmitter->SetParticleCount( 1 );
        pEmitter->SetEmissionDelay( 0.0f );
        pEmitter->SetPosition( position );
        pEmitter->SetTextureAtlas( "data/particles/Fire_Sprites_v3.png", 512, 512, 64 );
        pEmitter->SetScale( 0.2f, 0.4f );
        pEmitter->SetLifetime( 0.5f, 0.75f );
        pEmitter->Start();
    }

    PlayHitSFX( position, pWeapon );
}

void AmmoManager::PlayHitSFX( const glm::vec3& position, Weapon* pWeapon )
{
    using namespace Genesis;

    ResourceSound* pSFX = pWeapon->GetInfo()->GetOnHitSFX();
    if ( pSFX != nullptr )
    {
        Sound::SoundInstanceSharedPtr pSoundInstance = FrameWork::GetSoundManager()->CreateSoundInstance( pSFX, Genesis::Sound::SoundBus::Type::SFX, position, pWeapon->GetInfo()->GetOnHitSFXDistance() );
    }
}

void AmmoManager::GetInterceptables( AmmoVector& vec ) const
{
    for ( Ammo* pAmmo : m_Ammo )
    {
        if ( pAmmo != nullptr && pAmmo->IsAlive() && pAmmo->CanBeIntercepted() && pAmmo->WasIntercepted() == false )
        {
            vec.push_back( pAmmo );
        }
    }
}

} // namespace Hexterminate