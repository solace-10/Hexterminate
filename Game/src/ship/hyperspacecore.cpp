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

#include <algorithm>

#include <math/misc.h>

#include "achievements.h"
#include "hexterminate.h"
#include "player.h"
#include "sector/sector.h"
#include "ship/controller/controller.h"
#include "ship/hyperspacecore.h"
#include "ship/hyperspacegate.h"
#include "ship/module.h"
#include "ship/moduleinfo.h"
#include "ship/ship.h"

namespace Hexterminate
{

HyperspaceCore::HyperspaceCore( Ship* pOwner )
    : m_pOwner( pOwner )
    , m_Charging( false )
    , m_Jumping( false )
    , m_ChargeTimer( HyperspaceCoreChargeDuration )
    , m_JumpDirection( HyperspaceJumpDirection::JumpInvalid )
    , m_pHyperspaceGate( nullptr )
    , m_ChargeupReduced( false )
{
    m_pHyperspaceGate = new HyperspaceGate( pOwner );
    m_pHyperspaceGate->Initialise();
}

HyperspaceCore::~HyperspaceCore()
{
    delete m_pHyperspaceGate;
}

void HyperspaceCore::Update( float delta )
{
    // If we have a gate, that still needs to update, or we end up with some visual glitches due as the clipping plane
    // ceases to be valid.
    m_pHyperspaceGate->Update( delta );

    if ( m_pOwner->IsDestroyed() == false )
    {
        if ( m_Charging )
        {
            float chargeTimerDecrease = delta;

            // Some engines decrease the hyperspace chargeup. This effect does not stack.
            if ( m_ChargeupReduced )
            {
                chargeTimerDecrease *= 1.5f;
            }

            m_ChargeTimer = std::max( 0.0f, m_ChargeTimer - chargeTimerDecrease );
            if ( m_ChargeTimer <= 0.0f )
            {
                m_Charging = false;
                BeginJump();
            }
        }

        if ( m_Jumping && m_pHyperspaceGate->HasClosed() )
        {
            EndJump();
        }
    }
}

void HyperspaceCore::EnterHyperspace( bool noChargeUp /* = false */ )
{
    if ( m_Charging )
        return;

    if ( m_pOwner->IsDestroyed() )
        return;

    m_ChargeTimer = noChargeUp ? 0.0f : HyperspaceCoreChargeDuration;
    m_Charging = true;
    m_JumpDirection = HyperspaceJumpDirection::JumpOut;

    m_ChargeupReduced = false;
    for ( Module* pModule : m_pOwner->GetModules() )
    {
        if ( pModule->GetModuleInfo()->GetType() == ModuleType::Engine )
        {
            EngineInfo* pEngineInfo = static_cast<EngineInfo*>( pModule->GetModuleInfo() );
            if ( pEngineInfo->HasBonus( EngineBonus::ReducesHyperspaceChargeup ) )
            {
                m_ChargeupReduced = true;
                break;
            }
        }
    }
}

void HyperspaceCore::ExitHyperspace()
{
    if ( m_pOwner->IsDestroyed() )
        return;

    m_ChargeTimer = 0.0f;
    m_Charging = false;
    m_JumpDirection = HyperspaceJumpDirection::JumpIn;
    BeginJump();
}

void HyperspaceCore::BeginJump()
{
    if ( m_Jumping )
        return;

    m_pHyperspaceGate->Open( m_JumpDirection );
    m_Jumping = true;

    m_pOwner->GetController()->Suspend( true );
}

void HyperspaceCore::EndJump()
{
    if ( m_JumpDirection == HyperspaceJumpDirection::JumpIn )
    {
        m_Jumping = false;
        m_pOwner->GetController()->Suspend( false );
    }
    else
    {
        if ( m_pOwner == g_pGame->GetPlayer()->GetShip() )
        {
            // Unlock the "Tis but a scratch" achievement if the player jumps out with less than 25% HP.
            if ( m_pOwner->GetIntegrity() < 25 )
            {
                g_pGame->GetAchievementsManager()->UnlockAchievement( ACH_TIS_BUT_A_SCRATCH );
            }

            g_pGame->ExitSector();
        }
        else
        {
            g_pGame->GetCurrentSector()->RemoveShip( m_pOwner );
        }
    }
}

float HyperspaceCore::GetChargePercentage() const
{
    return gClamp<float>( 1.0f - m_ChargeTimer / HyperspaceCoreChargeDuration, 0.0f, 1.0f );
}

} // namespace Hexterminate