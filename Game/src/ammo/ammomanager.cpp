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
    for ( int i = 0; i < AmmoManagerCapacity; ++i )
    {
        m_pAmmo[ i ] = nullptr;
    }

    m_RayTestResults.reserve( 64 );
};

AmmoManager::~AmmoManager()
{
    for ( int i = 0; i < AmmoManagerCapacity; ++i )
    {
        delete m_pAmmo[ i ];
    }
}

Ammo** AmmoManager::GetFreeAmmo()
{
    Ammo** ppAmmo = &m_pAmmo[ m_Idx ];

    // Has ammo already been created into this slot?
    if ( *ppAmmo != nullptr )
    {
        // A weapon is actually still using this one, search our buffer for a free slot
        // If the buffer is completely full with active ammo, then we'll return a nullptr
        if ( ( *ppAmmo )->IsAlive() )
        {
            ppAmmo = nullptr;

            for ( int i = 1; i < AmmoManagerCapacity; ++i )
            {
                if ( ++m_Idx == AmmoManagerCapacity )
                    m_Idx = 0;

                if ( m_pAmmo[ m_Idx ] == nullptr || m_pAmmo[ m_Idx ]->IsAlive() == false )
                {
                    ppAmmo = &m_pAmmo[ m_Idx ];
                    break;
                }
            }
        }

        // If there was a dead ammo in our slot we can now remove it
        if ( ppAmmo != nullptr && *ppAmmo != nullptr )
        {
            delete *ppAmmo;

            if ( ++m_Idx == AmmoManagerCapacity )
                m_Idx = 0;
        }
    }

    return ppAmmo;
}

Ammo* AmmoManager::Create( Weapon* pWeapon, float additionalRotation /* = 0.0f */ )
{
    Ammo** ppAmmo = GetFreeAmmo();
    if ( ppAmmo == nullptr )
    {
        Genesis::FrameWork::GetLogger()->LogError( "AmmoManager capacity exceeded!" );
        return nullptr;
    }

    WeaponSystem weaponSystem = pWeapon->GetInfo()->GetSystem();
    if ( weaponSystem == WeaponSystem::Projectile )
    {
        *ppAmmo = new Projectile();
    }
    else if ( weaponSystem == WeaponSystem::Missile )
    {
        *ppAmmo = new Missile();
    }
    else if ( weaponSystem == WeaponSystem::Rocket )
    {
        *ppAmmo = new Rocket();
    }
    else if ( weaponSystem == WeaponSystem::Torpedo )
    {
        *ppAmmo = new Torpedo();
    }
    else if ( weaponSystem == WeaponSystem::Ion )
    {
        *ppAmmo = new Beam();
    }
    else if ( weaponSystem == WeaponSystem::Lance )
    {
        *ppAmmo = new Lance();
    }
    else if ( weaponSystem == WeaponSystem::Antiproton )
    {
        *ppAmmo = new Antiproton();
    }
    else
    {
        SDL_assert_release( false ); // Not implemented!
    }

    ( *ppAmmo )->Create( pWeapon, additionalRotation );
    return *ppAmmo;
}

void AmmoManager::Update( float delta )
{
    if ( g_pGame->IsPaused() )
    {
        return;
    }

    for ( int i = 0; i < AmmoManagerCapacity; ++i )
    {
        if ( m_pAmmo[ i ] != nullptr && m_pAmmo[ i ]->IsAlive() )
            m_pAmmo[ i ]->Update( delta );
    }

    Genesis::Physics::Simulation* pPhysicsSimulation = g_pGame->GetPhysicsSimulation();
    for ( int i = 0; i < AmmoManagerCapacity; ++i )
    {
        Ammo* pAmmo = m_pAmmo[ i ];
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
    for ( auto& pAmmo : m_pAmmo )
    {
        if ( pAmmo && pAmmo->IsAlive() && pAmmo->IsGlowSource() )
            pAmmo->Render();
    }

    Genesis::FrameWork::GetRenderSystem()->SetRenderTarget( Genesis::RenderTargetId::Default );
    for ( auto& pAmmo : m_pAmmo )
    {
        if ( pAmmo && pAmmo->IsAlive() )
            pAmmo->Render();
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
    for ( int i = 0; i < AmmoManagerCapacity; ++i )
    {
        Ammo* pAmmo = m_pAmmo[ i ];
        if ( pAmmo != nullptr && pAmmo->IsAlive() && pAmmo->CanBeIntercepted() && pAmmo->WasIntercepted() == false )
        {
            vec.push_back( pAmmo );
        }
    }
}

} // namespace Hexterminate