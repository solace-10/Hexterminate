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

#include <sstream>

#include <glm/gtc/matrix_access.hpp>

#include <genesis.h>
#include <math/misc.h>
#include <physics/rigidbody.h>
#include <physics/shape.h>
#include <resources/resourcemodel.h>
#include <resources/resourcesound.h>
#include <sound/soundinstance.h>
#include <sound/soundmanager.h>

#include "globals.h"
#include "hexterminate.h"
#include "menus/shiptweaks.h"
#include "particles/particleemitter.h"
#include "particles/particlemanager.h"
#include "sector/sector.h"
#include "ship/addon/addonenginedisruptor.h"
#include "ship/addon/addonmissileinterceptor.h"
#include "ship/addon/addonparticleaccelerator.h"
#include "ship/addon/addonphasebarrier.h"
#include "ship/hyperspacecore.h"
#include "ship/module.h"
#include "ship/moduleinfo.h"
#include "ship/ship.h"
#include "ship/shipoutline.h"
#include "ship/weapon.h"
#include "sprite/sprite.h"
#include "sprite/spritemanager.h"
#include "trail/trail.h"
#include "trail/trailmanager.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// Module
///////////////////////////////////////////////////////////////////////////////

Module::Module( ModuleInfo* pInfo )
{
    m_pInfo = pInfo;
    m_pOwner = nullptr;
    m_pModel = (Genesis::ResourceModel*)Genesis::FrameWork::GetResourceManager()->GetResource( pInfo->GetModel() );
    m_pModel->SetFlipAxis( false );
    m_HexGridSlotX = -1;
    m_HexGridSlotY = -1;
    m_pDamageParticleEmitter = nullptr;
    m_pDeathSFX = nullptr;
    m_IsLinked = false;
    m_DestructionTimer = 0.0f;
    m_PlasmaWarheadsTimer = 0.0f;
    m_EMPTimer = 0.0f;
    m_AssemblyPercentage = 1.0f;
    m_pCollisionInfo = nullptr;
    m_pPhysicsShape = nullptr;

    // This health value is temporary. The real health is only set in
    // SetOwner() as perks modify the base health.
    m_Health = pInfo->GetHealth( nullptr );

    LoadDeathSFX();
}

Module::~Module()
{
    if ( m_pDamageParticleEmitter != nullptr && g_pGame->GetCurrentSector() != nullptr && g_pGame->GetCurrentSector()->GetParticleManager() != nullptr )
    {
        m_pDamageParticleEmitter->Stop();
        m_pDamageParticleEmitter = nullptr;
    }
}

void Module::Update( float delta )
{
    if ( m_pDamageParticleEmitter != nullptr )
    {
        glm::vec3 moduleLocalPos = GetLocalPosition() + glm::vec3( 0.0f, 0.0f, 10.0f );
        glm::vec3 moduleWorldPos = glm::vec3( GetOwner()->GetRigidBody()->GetWorldTransform() * glm::vec4( moduleLocalPos, 1.0f ) );
        m_pDamageParticleEmitter->SetPosition( moduleWorldPos );
    }

    HyperspaceCore* pHyperspaceCore = m_pOwner->GetHyperspaceCore();
    if ( pHyperspaceCore != nullptr && pHyperspaceCore->IsJumping() && m_pDamageParticleEmitter != nullptr )
    {
        m_pDamageParticleEmitter->Stop();
        m_pDamageParticleEmitter = nullptr;
    }

    if ( IsDestroyed() )
    {
        m_DestructionTimer += delta;

        if ( m_pDamageParticleEmitter != nullptr && g_pGame->GetCurrentSector() != nullptr && g_pGame->GetCurrentSector()->GetParticleManager() != nullptr )
        {
            m_pDamageParticleEmitter->Stop();
            m_pDamageParticleEmitter = nullptr;
        }
    }

    if ( m_PlasmaWarheadsTimer >= 0.0f )
    {
        m_PlasmaWarheadsTimer -= delta;
    }

    if ( m_EMPTimer >= 0.0f )
    {
        m_EMPTimer -= delta;
    }

    if ( m_AssemblyPercentage < 1.0f )
    {
        m_AssemblyPercentage = 1.0f;
    }
}

void Module::UpdateShipyard( float delta )
{
    if ( m_AssemblyPercentage < 1.0f )
    {
        m_AssemblyPercentage = gMin( m_AssemblyPercentage + delta * 2.0f, 1.0f );
    }
}

void Module::Render()
{
    // Should never be called directly. Use Render( modelTransform, drawOutline ) instead.
    SDL_assert( false );
}

void Module::Render( const glm::mat4& modelTransform, bool drawOutline )
{
    SDL_assert( m_pModel != nullptr );

    ShipOutline* pShipOutline = g_pGame->GetShipOutline();
    if ( drawOutline )
    {
        glm::mat4 outlineTransform = glm::scale( modelTransform, glm::vec3( pShipOutline->GetThickness() ) );
        m_pModel->Render( outlineTransform, pShipOutline->GetOutlineMaterial( GetOwner() ) );
    }
    else
    {
        m_pModel->Render( modelTransform );
    }
}

void Module::Destroy()
{
    ApplyDamage( GetHealth() + 1.0f, DamageType::TrueDamage, nullptr );
}

void Module::ApplyDamage( float amount, DamageType damageType, Ship* pDealtBy )
{
    if ( pDealtBy != nullptr && pDealtBy->HasPerk( Perk::Disruption ) && damageType == DamageType::Energy && gRand() < ( 0.1f / 60.0f ) )
    {
        TriggerEMP();
    }

    if ( damageType == DamageType::Collision )
    {
        TowerBonus bonus;
        float magnitude;
        g_pGame->GetCurrentSector()->GetTowerBonus( GetOwner()->GetFaction(), &bonus, &magnitude );
        if ( bonus == TowerBonus::Ramming )
        {
            amount *= magnitude;
        }
    }

    m_Health = gMax( 0.0f, m_Health - amount );

    if ( m_Health <= 0.0f )
    {
        m_pOwner->OnModuleDestroyed( this );
        OnDeathEffect();
    }
    else
    {
        if ( damageType == DamageType::Kinetic && pDealtBy != nullptr && pDealtBy->HasPerk( Perk::PlasmaWarheads ) )
        {
            m_PlasmaWarheadsTimer = 5.0f;
        }

        OnDamageEffect();
    }
}

void Module::Repair( float amount )
{
    if ( ( IsDestroyed() == false && IsEMPed() == false ) || m_pOwner->GetDockingState() != DockingState::Undocked )
    {
        if ( m_PlasmaWarheadsTimer > 0.0f )
        {
            amount *= 0.3f;
        }

        const float maxHealth = m_pInfo->GetHealth( GetOwner() );
        m_Health = gMin( m_Health + amount, maxHealth );
        m_DestructionTimer = 0.0f;

        if ( m_pDamageParticleEmitter != nullptr && m_Health / maxHealth > 0.55f )
        {
            m_pDamageParticleEmitter->Stop();
            m_pDamageParticleEmitter = nullptr;
        }
    }
}

glm::vec3 Module::GetLocalPosition( Ship* pShip, int x, int y )
{
    glm::vec3 modulePos( (float)x * sModuleHorizontalSpacing, (float)y * sModuleHalfHeight, 0.0f );
    if ( y % 2 == 1 )
    {
        modulePos.x += sModuleHorizontalSpacing / 2.0f;
    }

    if ( pShip != nullptr && pShip->GetRigidBody() != nullptr )
    {
        modulePos -= pShip->GetRigidBody()->GetCentreOfMass();
    }

    return modulePos;
}

glm::vec3 Module::GetLocalPosition() const
{
    SDL_assert( m_HexGridSlotX != -1 && m_HexGridSlotY != -1 );
    return Module::GetLocalPosition( m_pOwner, m_HexGridSlotX, m_HexGridSlotY );
}

glm::vec3 Module::GetWorldPosition() const
{
    glm::vec3 moduleWorldPos = glm::vec3( GetOwner()->GetRigidBody()->GetWorldTransform() * glm::vec4( GetLocalPosition(), 1.0f ) );
    return moduleWorldPos;
}

void Module::OnDeathEffect()
{
    glm::vec3 moduleLocalPos = GetLocalPosition() + glm::vec3( 0.0f, 0.0f, 10.0f );
    glm::vec3 moduleWorldPos = glm::vec3( GetOwner()->GetRigidBody()->GetWorldTransform() * glm::vec4( moduleLocalPos, 1.0f ) );

    ParticleManager* pParticleManager = g_pGame->GetCurrentSector()->GetParticleManager();
    ParticleEmitter* pEmitter = pParticleManager->GetAvailableEmitter();
    pEmitter->SetBlendMode( Genesis::BlendMode::Blend );
    pEmitter->SetParticleCount( 1 );
    pEmitter->SetEmissionDelay( 0.0f );
    pEmitter->SetPosition( moduleWorldPos );
    pEmitter->SetTextureAtlas( ParticleEmitter::GetRandomExplosion(), 512, 512, 64 );
    pEmitter->SetScale( 1.0f, 1.25f );
    pEmitter->SetLifetime( 2.0f, 3.0f );
    pEmitter->Start();

    if ( m_pDamageParticleEmitter != nullptr )
    {
        m_pDamageParticleEmitter->Stop();
    }

    PlayDeathSFX();
}

void Module::OnDamageEffect()
{
    if ( m_pDamageParticleEmitter == nullptr && m_Health / m_pInfo->GetHealth( GetOwner() ) < 0.50f )
    {
        static const int sFxVariants = 2;
        static const std::string atlas[ sFxVariants ] = {
            "data/particles/Smoke_Sprites_Med.png",
            "data/particles/Smoke_Sprites_Dense.png"
        };

        glm::vec3 moduleLocalPos = GetLocalPosition() + glm::vec3( 0.0f, 0.0f, 5.0f );
        glm::vec3 moduleWorldPos = glm::vec3( GetOwner()->GetRigidBody()->GetWorldTransform() * glm::vec4( moduleLocalPos, 1.0f ) );

        ParticleManager* pParticleManager = g_pGame->GetCurrentSector()->GetParticleManager();
        m_pDamageParticleEmitter = pParticleManager->GetAvailableEmitter();
        m_pDamageParticleEmitter->SetBlendMode( Genesis::BlendMode::Blend );
        m_pDamageParticleEmitter->SetParticleCount( sInfiniteParticles );
        m_pDamageParticleEmitter->SetEmissionDelay( 0.1f );
        m_pDamageParticleEmitter->SetPosition( moduleWorldPos );
        m_pDamageParticleEmitter->SetTextureAtlas( atlas[ rand() % sFxVariants ], 512, 512, 64 );
        m_pDamageParticleEmitter->SetScale( 0.5f, 0.75f );
        m_pDamageParticleEmitter->SetLifetime( 1.0f, 1.0f );
        m_pDamageParticleEmitter->SetVelocity( glm::vec3( 0.0f, 0.0f, 100.0f ) );
        m_pDamageParticleEmitter->Start();
    }
}

void Module::LoadDeathSFX()
{
    using namespace Genesis;
    using namespace std::literals;

    std::stringstream ss;
    ss << "data/sfx/large_explosion_" << ( ( rand() % 4 ) + 1 ) << ".wav";

    m_pDeathSFX = (ResourceSound*)FrameWork::GetResourceManager()->GetResource( ss.str() );
    if ( m_pDeathSFX != nullptr )
    {
        m_pDeathSFX->Initialise( SOUND_FLAG_3D | SOUND_FLAG_FX );
        m_pDeathSFX->SetInstancingLimit( 100ms );
    }
}

void Module::PlayDeathSFX()
{
    if ( m_pDeathSFX != nullptr )
    {
        using namespace Genesis;

        Sound::SoundInstanceSharedPtr pSoundInstance = FrameWork::GetSoundManager()->CreateSoundInstance( m_pDeathSFX, Genesis::Sound::SoundBus::Type::SFX, GetWorldPosition(), 400.0f );
    }
}

void Module::TriggerEMP()
{
    m_EMPTimer = 8.0f;
}

void Module::SetOwner( Ship* pShip )
{
    // The module's health needs to be reset when the owner changes,
    // so we can take the Reinforced Bulkheads perk into account.
    if ( pShip != nullptr )
    {
        m_Health = GetModuleInfo()->GetHealth( pShip );
    }

    m_pOwner = pShip;

    m_pCollisionInfo = std::make_unique<ShipCollisionInfo>( pShip, this );
}

Genesis::Physics::ShapeSharedPtr Module::GetPhysicsShape() const
{
    return m_pPhysicsShape;
}

void Module::SetPhysicsShape( Genesis::Physics::ShapeSharedPtr pShape )
{
    m_pPhysicsShape = pShape;
}

///////////////////////////////////////////////////////////////////////////////
// WeaponModule
///////////////////////////////////////////////////////////////////////////////

WeaponModule::WeaponModule( ModuleInfo* pInfo )
    : Module( pInfo )
{
    WeaponInfo* pWeaponInfo = (WeaponInfo*)pInfo;
    m_fDamage = pWeaponInfo->GetDamage();
    m_pWeapon = nullptr;
}

WeaponModule::~WeaponModule()
{
    if ( m_pWeapon != nullptr )
    {
        delete m_pWeapon;
    }
}

void WeaponModule::Initialise( Ship* pShip )
{
    WeaponInfo* pInfo = static_cast<WeaponInfo*>( GetModuleInfo() );

    WeaponHardpoint hardpoint;
    hardpoint.isTurret = ( pInfo->GetBehaviour() == WeaponBehaviour::Turret );
    hardpoint.offset = glm::vec3( 0.0f, 0.0f, 0.0f );

    // The module might optionally have an extra offset, where the weapon should be mounted to.
    const Genesis::ResourceModel* pModel = GetModel();
    SDL_assert_release( pModel != nullptr );
    glm::vec3 hardpointOffset;
    if ( pModel->GetDummy( "hardpoint", &hardpointOffset ) )
    {
        hardpoint.offset = hardpointOffset;
    }
    else if ( hardpoint.isTurret )
    {
        Genesis::FrameWork::GetLogger()->LogInfo( "Turret weapon '%s' has no hardpoint dummy", pInfo->GetFullName().c_str() );
    }

    m_pWeapon = new Weapon( pShip, this, pInfo, hardpoint );
}

void WeaponModule::Update( float delta )
{
    Module::Update( delta );

    Weapon* pWeapon = GetWeapon();
    if ( pWeapon != nullptr )
    {
        if ( ( IsDestroyed() || IsEMPed() ) && pWeapon->IsFiring() )
        {
            pWeapon->StopFiring();
        }
        else if ( IsDestroyed() == false && IsEMPed() == false )
        {
            GetWeapon()->Update( delta );
        }
    }
}

void WeaponModule::Render( const glm::mat4& modelTransform, bool drawOutline )
{
    Module::Render( modelTransform, drawOutline );
    GetWeapon()->Render( modelTransform );
}

///////////////////////////////////////////////////////////////////////////////
// ArmourModule
///////////////////////////////////////////////////////////////////////////////

ArmourModule::ArmourModule( ModuleInfo* pInfo )
    : Module( pInfo )
    , m_IsRegenerative( false )
    , m_RegenerationRate( 0.0f )
    , m_DamageTimer( 100.0f )
    , m_OverlayIntensity( 0.0f )
    , m_UnbrokenCooldown( 0.0f )
    , m_UnbrokenTimer( 0.0f )
{
}

void ArmourModule::SetOwner( Ship* pShip )
{
    Module::SetOwner( pShip );

    ArmourInfo* pArmourInfo = static_cast<ArmourInfo*>( GetModuleInfo() );
    if ( pArmourInfo->IsRegenerative() )
    {
        m_RegenerationRate += pArmourInfo->GetHealth( pShip ) * RegenerationRate / RepairArmourMultiplier;
    }

    if ( GetOwner()->HasPerk( Perk::Nanobots ) )
    {
        m_RegenerationRate += pArmourInfo->GetHealth( pShip ) * RegenerationRate / RepairArmourMultiplier;
    }

    m_IsRegenerative = ( m_RegenerationRate > 0.0f );
}

void ArmourModule::ApplyDamage( float amount, DamageType damageType, Ship* pDealtBy )
{
    if ( damageType == DamageType::TrueDamage ) // True damage, not modified by any resistances or perks
    {
        Module::ApplyDamage( amount, damageType, pDealtBy );
    }
    else
    {
        float damageToApply = 0.0f;
        if ( IsUnbrokenStateActive() == false )
        {
            if ( damageType == DamageType::Kinetic || damageType == DamageType::Collision )
            {
                // Projectile and missile weapons, as well as collisions, have their damage reduced (%-wise) by the armour's kinetic resistance.
                const float kineticResistance = static_cast<ArmourInfo*>( GetModuleInfo() )->GetKineticResistance();
                damageToApply = amount * ( 1.0f - kineticResistance );

            }
            else if ( damageType == DamageType::Energy )
            {
                // Energy based weapons have their damage reduced (%-wise) by the armour's energy resistance.
                const float energyResistance = static_cast<ArmourInfo*>( GetModuleInfo() )->GetEnergyResistance();
                damageToApply = amount * ( 1.0f - energyResistance );
            }
        }

        // Would taking this much damage trigger the Unbroken perk on this module?
        if ( m_pOwner->HasPerk( Perk::Unbroken ) && m_UnbrokenCooldown <= 0.0f && GetHealth() <= damageToApply )
        {
            damageToApply = 0.0f;
            m_UnbrokenCooldown = 30.0f;
            m_UnbrokenTimer = 10.0f;
        }

        Module::ApplyDamage( damageToApply, damageType, pDealtBy );
    }

    m_DamageTimer = 0.0f;
}

void ArmourModule::Repair( float amount )
{
    // Repairs are more effective on armour modules
    Module::Repair( amount * RepairArmourMultiplier );
}

void ArmourModule::OnDamageEffect()
{
    // Purposefully empty as we don't want any of the effects of the base class for armour modules

    m_DamageTimer = 0.0f;
}

void ArmourModule::Update( float delta )
{
    Module::Update( delta );

    if ( m_IsRegenerative )
    {
        Repair( m_RegenerationRate * delta );
    }

    if ( m_pOwner->HasPerk( Perk::Unbroken ) )
    {
        if ( m_UnbrokenCooldown > 0.0f )
        {
            m_UnbrokenCooldown -= delta;
        }

        if ( m_UnbrokenTimer > 0.0f )
        {
            m_UnbrokenTimer -= delta;
        }
    }

    m_DamageTimer += delta;

    CalculateOverlayIntensity();
}

void ArmourModule::CalculateOverlayIntensity()
{
    float intensityFromRegeneration = 0.0f;
    float intensityFromDamage = 0.0f;
    if ( m_RegenerationRate > 0.0f )
    {
        intensityFromRegeneration = ( GetHealth() < GetModuleInfo()->GetHealth( GetOwner() ) ) ? 1.0f : 0.0f;
    }

    // The intensity of the overlay effect decays over time, using the function "-((x/3)^4 + 1"
    // http://fooplot.com/#W3sidHlwZSI6MCwiZXEiOiItKCh4LzMpXjQpKzEiLCJjb2xvciI6IiMwMDAwMDAifSx7InR5cGUiOjEwMDAsIndpbmRvdyI6WyIwIiwiNSIsIjAiLCIxLjUiXX1d
    static const float sMaxOverlayTime = 3.0f;
    const float currentOverlayTime = gClamp<float>( m_DamageTimer, 0.0f, sMaxOverlayTime );
    intensityFromDamage = -powf( currentOverlayTime / sMaxOverlayTime, 4 ) + 1.0f;

    m_OverlayIntensity = std::max( intensityFromRegeneration, intensityFromDamage );
}

const glm::vec4 ArmourModule::GetOverlayColour() const
{
    if ( IsUnbrokenStateActive() )
    {
        return glm::vec4( 1.0f, 1.0f, 1.0f, 0.7f );
    }
    else
    {
        glm::vec4 overlayColour = m_pInfo->GetOverlayColour().glm();
        overlayColour.a *= m_OverlayIntensity;
        return overlayColour;
    }
}

bool ArmourModule::IsUnbrokenStateActive() const
{
    return ( m_UnbrokenTimer > 0.0f );
}

///////////////////////////////////////////////////////////////////////////////
// ShieldModule
///////////////////////////////////////////////////////////////////////////////

ShieldModule::ShieldModule( ModuleInfo* pInfo )
    : Module( pInfo )
    , m_fCapacity( 0.0f )
    , m_fPeakRecharge( 0.0f )
{
    ShieldInfo* pShieldInfo = (ShieldInfo*)pInfo;
    m_fCapacity = pShieldInfo->GetCapacity();
    m_fPeakRecharge = pShieldInfo->GetPeakRecharge();
}

///////////////////////////////////////////////////////////////////////////////
// EngineModule
///////////////////////////////////////////////////////////////////////////////

EngineModule::EngineModule( ModuleInfo* pInfo )
    : Module( pInfo )
    , m_pTrail( nullptr )
    , m_Output( 0.0f )
    , m_FlickerDuration( 0.0f )
    , m_TimeToNextFlicker( 0.0f )
    , m_RammingSpeedScaling( 1.0f )
    , m_Enabled( true )
{
    glm::vec3 offset;
    if ( GetModel()->GetDummy( "Trail", &offset ) )
    {
        m_TrailOffset = offset;
    }
    else
    {
        m_TrailOffset = glm::vec3();
    }
}

EngineModule::~EngineModule()
{
    // If the current sector is null then it has been destroyed, and with it the
    // trail manager and any associated trails, so this trail would no longer be
    // valid.
    RemoveTrail();
}

void EngineModule::Update( float delta )
{
    Module::Update( delta );

    if ( m_Enabled == false )
    {
        return;
    }

    if ( IsDestroyed() == false && m_pOwner != nullptr )
    {
        const bool tryingToMove = m_pOwner->GetThrust() != ShipThrust::None || m_pOwner->GetSteer() != ShipSteer::None || m_pOwner->GetStrafe() != ShipStrafe::None;

        if ( tryingToMove && IsEMPed() == false )
        {
            m_Output = gMin( m_Output + delta / 4.0f, 1.0f );

            if ( m_pOwner->AreEnginesDisrupted() )
            {
                m_Output = gMin( m_Output, 0.15f );

                // Make the engines flicker "randomly" when disrupted
                if ( m_FlickerDuration <= 0.0f && m_TimeToNextFlicker <= 0.0f )
                {
                    m_TimeToNextFlicker = gRand( 0.1f, 0.3f );
                }
                else if ( m_TimeToNextFlicker > 0.0f )
                {
                    m_TimeToNextFlicker -= delta;
                    if ( m_TimeToNextFlicker <= 0.0f )
                    {
                        m_FlickerDuration = gRand( 0.05f, 0.1f );
                    }
                }
            }
        }
        else
        {
            m_Output = gMax( m_Output - delta / 2.0f, 0.0f );
        }

        UpdateTrail();
        UpdateGlow( delta );
    }
}

void EngineModule::Enable()
{
    m_Enabled = true;
}

void EngineModule::Disable()
{
    m_Enabled = false;
    m_Output = 0.0f;
    UpdateTrail();
    UpdateGlow( 0.0f );
}

void EngineModule::UpdateTrail()
{
    Ship* pShip = GetOwner();
    if ( pShip != nullptr && m_pTrail == nullptr && m_Output > 0.0f )
    {
        m_pTrail = new Trail( 10.0f, 3.5f, pShip->GetFaction()->GetColour( pShip->IsFlagship() ? FactionColourId::GlowFlagship : FactionColourId::Glow ) );
        g_pGame->GetCurrentSector()->GetTrailManager()->Add( m_pTrail );
    }
    else if ( m_pTrail != nullptr && m_Output <= 0.0f )
    {
        m_pTrail->SetOrphan();
        m_pTrail = nullptr;
    }
    else if ( m_pTrail != nullptr )
    {
        m_pTrail->SetInitialWidth( pShip->IsRammingSpeedEnabled() ? 12.0f : 8.0f );
    }
}

void EngineModule::UpdateGlow( float delta )
{
    if ( m_Output > 0.0f )
    {
        if ( m_FlickerDuration >= 0.0f )
        {
            m_FlickerDuration -= delta;
            return;
        }

        if ( m_pOwner->IsRammingSpeedEnabled() && m_RammingSpeedScaling < 1.5f )
        {
            m_RammingSpeedScaling += delta;
        }
        else if ( m_RammingSpeedScaling > 1.0f )
        {
            m_RammingSpeedScaling -= delta;
        }
        m_RammingSpeedScaling = gClamp<float>( m_RammingSpeedScaling, 1.0f, 1.5f );

        const float intensity = ( -powf( m_Output - 1.0f, 6.0f ) + 1.0f ) * m_RammingSpeedScaling;
        const float size = 80.0f * intensity;
        Genesis::Physics::RigidBody* pRigidBody = m_pOwner->GetRigidBody();
        glm::vec3 forward = pRigidBody ? glm::vec3( glm::column( pRigidBody->GetWorldTransform(), 1 ) ) : glm::vec3( 0.0f, 1.0f, 0.0f );

        glm::vec3 source = GetWorldPosition() - glm::vec3( size / 2.0f, 0.0f, 0.0f );
        source -= forward * 10.0f;
        glm::vec3 destination = source + glm::vec3( size, 0.0f, 0.0f );

        Sprite sprite(
            source,
            destination,
            Genesis::Color( 1.0f, 1.0f, 1.0f, 1.0f ),
            size,
            0 );

        g_pGame->GetCurrentSector()->GetSpriteManager()->AddSprite( sprite );
    }
}

void EngineModule::RemoveTrail()
{
    if ( m_pTrail != nullptr && g_pGame != nullptr && g_pGame->GetCurrentSector() != nullptr )
    {
        TrailManager* pTrailManager = g_pGame->GetCurrentSector()->GetTrailManager();
        if ( pTrailManager != nullptr )
        {
            if ( m_pTrail->IsOrphan() )
            {
                pTrailManager->Remove( m_pTrail );
            }
            else
            {
                pTrailManager->Remove( m_pTrail );
                delete m_pTrail;
            }
        }
    }

    m_pTrail = nullptr;
}

void EngineModule::TriggerEMP()
{
    EngineInfo* pInfo = static_cast<EngineInfo*>( m_pInfo );
    if ( pInfo->HasBonus( EngineBonus::EMPResistant ) == false )
    {
        Module::TriggerEMP();
    }
}

///////////////////////////////////////////////////////////////////////////////
// ReactorModule
///////////////////////////////////////////////////////////////////////////////

ReactorModule::ReactorModule( ModuleInfo* pInfo )
    : Module( pInfo )
    , m_CapacitorStorage( 0.0f )
    , m_CapacitorRechargeRate( 0.0f )
{
    ReactorInfo* pReactorInfo = (ReactorInfo*)pInfo;
    m_CapacitorStorage = pReactorInfo->GetCapacitorStorage();
    m_CapacitorRechargeRate = pReactorInfo->GetCapacitorRechargeRate();
}

float ReactorModule::GetCapacitorStorage() const
{
    return IsEMPed() ? 0.0f : m_CapacitorStorage;
}

float ReactorModule::GetCapacitorRechargeRate() const
{
    return IsEMPed() ? 0.0f : m_CapacitorRechargeRate;
}

void ReactorModule::OnDeathEffect()
{
    if (  static_cast<ReactorInfo*>( GetModuleInfo() )->GetVariant() != ReactorVariant::Unstable )
    {
        Module::OnDamageEffect();
    }
    else
    {
        const ModuleHexGrid& hexGrid = GetOwner()->GetModuleHexGrid();
        for ( int x = m_HexGridSlotX - 1; x <= m_HexGridSlotX + 1; ++x )
        {
            for ( int y = m_HexGridSlotY - 1; y <= m_HexGridSlotY + 1; ++y )
            {
                if ( x == m_HexGridSlotX && y == m_HexGridSlotY )
                {
                    continue;
                }

                Module* pModule = hexGrid.Get( x, y );
                if ( pModule == nullptr || pModule->IsDestroyed() )
                {
                    continue;
                }

                pModule->Destroy();
            }
        }

        const glm::vec3 moduleLocalPos = GetLocalPosition() + glm::vec3( 0.0f, 0.0f, 10.0f );
        const glm::vec3 moduleWorldPos = glm::vec3( GetOwner()->GetRigidBody()->GetWorldTransform() * glm::vec4( moduleLocalPos, 1.0f ) );
        const unsigned int atlasElementSize = 512;
        ParticleManager* pParticleManager = g_pGame->GetCurrentSector()->GetParticleManager();
        ParticleEmitter* pEmitter = pParticleManager->GetAvailableEmitter();
        pEmitter->SetBlendMode( Genesis::BlendMode::Add );
        pEmitter->SetParticleCount( 1 );
        pEmitter->SetEmissionDelay( 0.0f );
        pEmitter->SetPosition( moduleWorldPos );
        pEmitter->SetTextureAtlas( "data/particles/Fire_Ring.png", atlasElementSize, atlasElementSize, 50 );
        pEmitter->SetScale( 1.25f, 1.5f );
        pEmitter->SetLifetime( 1.0f, 1.5f );
        pEmitter->Start();

        PlayDeathSFX();
    }
}

///////////////////////////////////////////////////////////////////////////////
// AddonModule
///////////////////////////////////////////////////////////////////////////////

AddonModule::AddonModule( ModuleInfo* pInfo )
    : Module( pInfo )
    , m_pAddon( nullptr )
{
}

AddonModule::~AddonModule()
{
    delete m_pAddon;
}

void AddonModule::SetOwner( Ship* pShip )
{
    Module::SetOwner( pShip );
    CreateAddon();
}

void AddonModule::CreateAddon()
{
    delete m_pAddon;

    AddonInfo* pAddonInfo = static_cast<AddonInfo*>( GetModuleInfo() );
    AddonCategory category = pAddonInfo->GetCategory();
    SDL_assert_release( category != AddonCategory::Invalid );
    if ( category == AddonCategory::ModuleRepairer )
    {
        m_pAddon = new AddonModuleRepairer( this, GetOwner() );
    }
    else if ( category == AddonCategory::HangarBay )
    {
        m_pAddon = new AddonHangarBay( this, GetOwner() );
    }
    else if ( category == AddonCategory::DroneBay )
    {
        m_pAddon = new AddonDroneBay( this, GetOwner() );
    }
    else if ( category == AddonCategory::PhaseBarrier )
    {
        m_pAddon = new AddonPhaseBarrier( this, GetOwner() );
    }
    else if ( category == AddonCategory::FuelInjector )
    {
        m_pAddon = new AddonFuelInjector( this, GetOwner() );
    }
    else if ( category == AddonCategory::ParticleAccelerator )
    {
        m_pAddon = new AddonParticleAccelerator( this, GetOwner() );
    }
    else if ( category == AddonCategory::MissileInterceptor )
    {
        m_pAddon = new AddonMissileInterceptor( this, GetOwner() );
    }
    else if ( category == AddonCategory::EngineDisruptor )
    {
        m_pAddon = new AddonEngineDisruptor( this, GetOwner() );
    }
    else if ( category == AddonCategory::QuantumStateAlternator )
    {
        m_pAddon = new AddonQuantumStateAlternator( this, GetOwner() );
    }

    if ( m_pAddon == nullptr )
    {
        Genesis::FrameWork::GetLogger()->LogError( "Couldn't create Addon, unrecognised type." );
    }
    else
    {
        m_pAddon->Initialise();
    }
}

void AddonModule::Update( float delta )
{
    Module::Update( delta );

    if ( m_pAddon != nullptr )
    {
        m_pAddon->Update( delta );
    }
}

void AddonModule::Render( const glm::mat4& modelTransform, bool drawOutline )
{
    Module::Render( modelTransform, drawOutline );

    if ( m_pAddon != nullptr )
    {
        m_pAddon->Render( modelTransform );
    }
}

///////////////////////////////////////////////////////////////////////////////
// TowerModule
///////////////////////////////////////////////////////////////////////////////

TowerModule::TowerModule( ModuleInfo* pInfo )
    : Module( pInfo )
{
}

void TowerModule::Initialise( Ship* pShip )
{
    Module::Initialise( pShip );
    AddBonus();
}

void TowerModule::ApplyDamage( float amount, DamageType damageType, Ship* pDealtBy )
{
    // If the ship begins the jump sequence the tower can no longer be damaged,
    // effectively making it invulnerable until it leaves the sector.
    HyperspaceCore* pHyperspaceCore = GetOwner()->GetHyperspaceCore();
    if ( pHyperspaceCore != nullptr && pHyperspaceCore->IsJumping() )
        return;

    if ( IsDestroyed() == false && GetHealth() - amount <= 0.0f )
    {
        GetOwner()->OnShipDestroyed();
        RemoveBonus();
    }

    Module::ApplyDamage( amount, damageType, pDealtBy );
}

void TowerModule::OnDamageEffect()
{
    if ( m_pDamageParticleEmitter == nullptr && m_Health / m_pInfo->GetHealth( GetOwner() ) < 0.1f )
    {
        static const int sFxVariants = 1;
        static const std::string atlas[ sFxVariants ] = {
            "data/particles/Fire_Sprites_v2.png"
        };

        glm::vec3 moduleLocalPos = GetLocalPosition() + glm::vec3( 0.0f, 0.0f, 5.0f );
        glm::vec3 moduleWorldPos = glm::vec3( GetOwner()->GetRigidBody()->GetWorldTransform() * glm::vec4( moduleLocalPos, 1.0f ) );

        ParticleManager* pParticleManager = g_pGame->GetCurrentSector()->GetParticleManager();
        m_pDamageParticleEmitter = pParticleManager->GetAvailableEmitter();
        m_pDamageParticleEmitter->SetBlendMode( Genesis::BlendMode::Add );
        m_pDamageParticleEmitter->SetParticleCount( sInfiniteParticles );
        m_pDamageParticleEmitter->SetEmissionDelay( 0.1f );
        m_pDamageParticleEmitter->SetPosition( moduleWorldPos );
        m_pDamageParticleEmitter->SetTextureAtlas( atlas[ rand() % sFxVariants ], 512, 512, 64 );
        m_pDamageParticleEmitter->SetScale( 0.25f, 0.25f );
        m_pDamageParticleEmitter->SetLifetime( 1.25f, 1.25f );
        m_pDamageParticleEmitter->SetVelocity( glm::vec3( 0.0f, 0.0f, 50.0f ) );
        m_pDamageParticleEmitter->Start();
    }
}

// When a tower module is instantiated we add its bonus to the sector if it is more powerful than what is currently
// available. Ideally the sector would keep a list of per-faction bonuses, but at the moment the design doesn't require
// this feature as only flagships have bonused bridges and each faction only has a single flagship in play at any time.
void TowerModule::AddBonus()
{
    TowerInfo* pTowerInfo = static_cast<TowerInfo*>( GetModuleInfo() );
    Sector* pCurrentSector = g_pGame->GetCurrentSector();

    TowerBonus currentSectorBonus = TowerBonus::None;
    float currentSectorBonusMagnitude = 0.0f;
    pCurrentSector->GetTowerBonus( GetOwner()->GetFaction(), &currentSectorBonus, &currentSectorBonusMagnitude );

    if ( currentSectorBonus == TowerBonus::None && pTowerInfo->GetBonusType() != TowerBonus::None )
    {
        pCurrentSector->SetTowerBonus( GetOwner()->GetFaction(), pTowerInfo->GetBonusType(), pTowerInfo->GetBonusMagnitude() );
    }
    else if ( currentSectorBonus != TowerBonus::None && currentSectorBonusMagnitude < pTowerInfo->GetBonusMagnitude() )
    {
        pCurrentSector->SetTowerBonus( GetOwner()->GetFaction(), pTowerInfo->GetBonusType(), pTowerInfo->GetBonusMagnitude() );
    }
}

// When the bridge is destroyed, if it has a bonus then it needs to be removed from the sector.
// Read comment above in AddBonus()
void TowerModule::RemoveBonus()
{
    TowerInfo* pTowerInfo = static_cast<TowerInfo*>( GetModuleInfo() );
    if ( pTowerInfo->GetBonusType() != TowerBonus::None )
    {
        g_pGame->GetCurrentSector()->SetTowerBonus( GetOwner()->GetFaction(), TowerBonus::None, 0.0f );
    }
}

const glm::vec4 TowerModule::GetOverlayColour( Ship* pShip ) const
{
    const float ratio = GetHealth() / GetModuleInfo()->GetHealth( pShip );
    const glm::vec4 red( 1.0f, 0.0f, 0.0f, 0.8f );
    const glm::vec4 yellow( 1.0f, 1.0f, 0.0f, 0.5f );
    const glm::vec4 green( 0.0f, 1.0f, 0.0f, 0.4f );

    if ( ratio > 0.5f )
    {
        return glm::mix( yellow, green, ratio * 2.0f - 1.0f );
    }
    else
    {
        return glm::mix( red, yellow, ratio * 2.0f );
    }
}

} // namespace Hexterminate