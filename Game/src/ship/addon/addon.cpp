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

#include "ship/addon/addon.h"
#include "hexterminate.h"
#include "ship/hyperspacecore.h"
#include "ship/module.h"
#include "ship/moduleinfo.h"
#include "ship/ship.h"
#include "ship/shipinfo.h"
#include "ship/weapon.h"

#include <genesis.h>
#include <math/misc.h>
#include <resources/resourcemodel.h>
#include <resources/resourcesound.h>
#include <sound/soundinstance.h>

namespace Hexterminate
{

bool AddonModuleSortPredicate( AddonModule* pModule1, AddonModule* pModule2 )
{
    return ( pModule1->GetModuleInfo()->GetFullName() < pModule2->GetModuleInfo()->GetFullName() );
}

///////////////////////////////////////////////////////////
// Addon
///////////////////////////////////////////////////////////

Addon::Addon( AddonModule* pModule, Ship* pOwner )
    : m_pModule( pModule )
    , m_pOwner( pOwner )
    , m_Cooldown( 0.0f )
    , m_IsActive( false )
{
    m_pInfo = static_cast<AddonInfo*>( m_pModule->GetModuleInfo() );
}

void Addon::Activate()
{
    // "Trigger" addons consume their energy immediately and deactivate when cooldown finishes
    if ( m_pInfo->GetType() == AddonActivationType::Trigger )
    {
        m_pOwner->ConsumeEnergy( m_pModule->GetModuleInfo()->GetActivationCost() );
        AddonInfo* pInfo = static_cast<AddonInfo*>( m_pModule->GetModuleInfo() );
        m_Cooldown = pInfo->GetCooldown();
    }

    m_IsActive = true;
}

void Addon::Deactivate()
{
    m_IsActive = false;
}

void Addon::Update( float delta )
{
    const bool isUndocked = ( m_pOwner->GetDockingState() == DockingState::Undocked );
    const bool isJumping = ( m_pOwner->GetHyperspaceCore() != nullptr && m_pOwner->GetHyperspaceCore()->IsJumping() );

    if ( IsActive() && ( m_pModule->IsDestroyed() || m_pModule->IsEMPed() || !isUndocked || isJumping ) )
    {
        Deactivate();
    }
    else
    {
        AddonActivationType type = m_pInfo->GetType();
        if ( type == AddonActivationType::Trigger )
        {
            if ( IsActive() && m_Cooldown - delta <= 0.0f )
            {
                Deactivate();
            }

            m_Cooldown = gMax( 0.0f, m_Cooldown - delta );
        }
        else if ( type == AddonActivationType::Toggle )
        {
            if ( IsActive() && m_pOwner->ConsumeEnergy( m_pInfo->GetActivationCost() * delta ) == false )
            {
                Deactivate();
            }
        }
    }
}

bool Addon::CanUse() const
{
    AddonActivationType type = m_pInfo->GetType();
    if ( m_pModule->IsEMPed() || m_pModule->IsDestroyed() )
    {
        return false;
    }
    else if ( type == AddonActivationType::Trigger )
    {
        return ( m_Cooldown <= 0.0f && m_pOwner->GetEnergy() > m_pInfo->GetActivationCost() );
    }
    else if ( type == AddonActivationType::Toggle )
    {
        return ( m_pOwner->GetEnergy() > m_pInfo->GetActivationCost() );
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////////////
// AddonArmourRepairer
///////////////////////////////////////////////////////////

AddonModuleRepairer::AddonModuleRepairer( AddonModule* pModule, Ship* pOwner )
    : Addon( pModule, pOwner )
{
    m_RepairAmount = (float)atof( ( (AddonInfo*)pModule->GetModuleInfo() )->GetParameter().c_str() );
    SDL_assert_release( m_RepairAmount > 0.0f );
}

void AddonModuleRepairer::Activate()
{
    Addon::Activate();
    m_pOwner->Repair( m_RepairAmount );
}

///////////////////////////////////////////////////////////
// AddonHangarBay
///////////////////////////////////////////////////////////

AddonHangarBay::AddonHangarBay( AddonModule* pModule, Ship* pOwner )
    : Addon( pModule, pOwner )
{
}

///////////////////////////////////////////////////////////
// AddonDroneBay
///////////////////////////////////////////////////////////

AddonDroneBay::AddonDroneBay( AddonModule* pModule, Ship* pOwner )
    : Addon( pModule, pOwner )
{
}

///////////////////////////////////////////////////////////
// AddonFuelInjector
///////////////////////////////////////////////////////////

AddonFuelInjector::AddonFuelInjector( AddonModule* pModule, Ship* pOwner )
    : Addon( pModule, pOwner )
{
    AddonInfo* pAddonInfo = static_cast<AddonInfo*>( pModule->GetModuleInfo() );
    m_EnginePowerMultiplier = static_cast<float>( atof( pAddonInfo->GetParameter().c_str() ) );
}

///////////////////////////////////////////////////////////////////////////////
// AddonQuantumStateAlternator
///////////////////////////////////////////////////////////////////////////////

AddonQuantumStateAlternator::AddonQuantumStateAlternator( AddonModule* pModule, Ship* pOwner )
    : Addon( pModule, pOwner )
    , m_pSFX( nullptr )
{
    m_State = QuantumState::Black;

    AddonInfo* pAddonInfo = static_cast<AddonInfo*>( pModule->GetModuleInfo() );
    m_ShieldResistance = static_cast<float>( atof( pAddonInfo->GetParameter().c_str() ) );

    LoadSFX();
}

QuantumState AddonQuantumStateAlternator::GetQuantumState() const
{
    if ( m_pModule->IsDestroyed() )
    {
        m_State = QuantumState::Inactive;
    }

    return m_State;
}

void AddonQuantumStateAlternator::LoadSFX()
{
    using namespace Genesis;
    using namespace std::literals;

    m_pSFX = (ResourceSound*)FrameWork::GetResourceManager()->GetResource( "data/sfx/alternator.wav" );
    if ( m_pSFX != nullptr )
    {
        m_pSFX->Initialise( SOUND_FLAG_3D | SOUND_FLAG_FX );
        m_pSFX->SetInstancingLimit( 250ms );
    }
}

void AddonQuantumStateAlternator::PlaySFX()
{
    if ( m_pSFX )
    {
        using namespace Genesis;

        Sound::SoundInstanceSharedPtr pSoundInstance = FrameWork::GetSoundManager()->CreateSoundInstance( m_pSFX, Genesis::Sound::SoundBus::Type::SFX, m_pModule->GetWorldPosition(), 300.0f );
    }
}

void AddonQuantumStateAlternator::SetState( QuantumState state )
{
    if ( state != QuantumState::Inactive && state != m_State )
    {
        PlaySFX();
    }

    m_State = state;
}

} // namespace Hexterminate