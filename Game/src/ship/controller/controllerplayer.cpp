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

#include "ship/controller/controllerplayer.h"
#include "hexterminate.h"
#include "perks.h"
#include "player.h"
#include "sector/sector.h"
#include "ship/hyperspacecore.h"
#include "ship/module.h"
#include "ship/moduleinfo.h"
#include "shipyard/shipyard.h"

#include <configuration.h>
#include <genesis.h>
#include <inputmanager.h>

#include <list>

namespace Hexterminate
{

ControllerPlayer::ControllerPlayer( Ship* pShip )
    : Controller( pShip )
    , m_Fire( false )
    , m_DodgeLeftTimer( 0.0f )
    , m_DodgeRightTimer( 0.0f )
{
    using namespace Genesis;
    InputManager* pInputManager = FrameWork::GetInputManager();

    m_ForwardToken = pInputManager->AddKeyboardCallback( std::bind( &ControllerPlayer::OnForwardAction, this ), SDL_SCANCODE_W, ButtonState::Held );
    m_BackwardToken = pInputManager->AddKeyboardCallback( std::bind( &ControllerPlayer::OnBackwardAction, this ), SDL_SCANCODE_S, ButtonState::Held );
    m_SteerLeftToken = pInputManager->AddKeyboardCallback( std::bind( &ControllerPlayer::OnSteerLeftAction, this ), SDL_SCANCODE_A, ButtonState::Held );
    m_SteerRightToken = pInputManager->AddKeyboardCallback( std::bind( &ControllerPlayer::OnSteerRightAction, this ), SDL_SCANCODE_D, ButtonState::Held );
    m_DodgeLeftToken = pInputManager->AddKeyboardCallback( std::bind( &ControllerPlayer::OnDodgeLeftAction, this ), SDL_SCANCODE_Q, ButtonState::Pressed );
    m_DodgeRightToken = pInputManager->AddKeyboardCallback( std::bind( &ControllerPlayer::OnDodgeRightAction, this ), SDL_SCANCODE_E, ButtonState::Pressed );
    m_StrafeLeftToken = pInputManager->AddKeyboardCallback( std::bind( &ControllerPlayer::OnStrafeLeftAction, this ), SDL_SCANCODE_Q, ButtonState::Held );
    m_StrafeRightToken = pInputManager->AddKeyboardCallback( std::bind( &ControllerPlayer::OnStrafeRightAction, this ), SDL_SCANCODE_E, ButtonState::Held );
    m_RammingSpeedToken = pInputManager->AddKeyboardCallback( std::bind( &ControllerPlayer::OnRammingSpeedAction, this ), SDL_SCANCODE_TAB, ButtonState::Pressed );
    m_FlipQuantumStateToken = pInputManager->AddKeyboardCallback( std::bind( &ControllerPlayer::OnFlipQuantumStateAction, this ), SDL_SCANCODE_SPACE, ButtonState::Pressed );
    m_PrimaryFirePressedToken = pInputManager->AddMouseCallback( std::bind( &ControllerPlayer::OnPrimaryFirePressedAction, this ), Genesis::MouseButton::Left, ButtonState::Pressed );
    m_PrimaryFireHeldToken = pInputManager->AddMouseCallback( std::bind( &ControllerPlayer::OnPrimaryFireHeldAction, this ), Genesis::MouseButton::Left, ButtonState::Held );
    m_PrimaryFireReleasedToken = pInputManager->AddMouseCallback( std::bind( &ControllerPlayer::OnPrimaryFireReleasedAction, this ), Genesis::MouseButton::Left, ButtonState::Released );
}

ControllerPlayer::~ControllerPlayer()
{
    using namespace Genesis;
    InputManager* pInputManager = FrameWork::GetInputManager();
    if ( pInputManager != nullptr )
    {
        pInputManager->RemoveKeyboardCallback( m_ForwardToken );
        pInputManager->RemoveKeyboardCallback( m_BackwardToken );
        pInputManager->RemoveKeyboardCallback( m_DodgeLeftToken );
        pInputManager->RemoveKeyboardCallback( m_DodgeRightToken );
        pInputManager->RemoveKeyboardCallback( m_SteerLeftToken );
        pInputManager->RemoveKeyboardCallback( m_SteerRightToken );
        pInputManager->RemoveKeyboardCallback( m_StrafeLeftToken );
        pInputManager->RemoveKeyboardCallback( m_StrafeRightToken );
        pInputManager->RemoveKeyboardCallback( m_RammingSpeedToken );
        pInputManager->RemoveKeyboardCallback( m_FlipQuantumStateToken );
        pInputManager->RemoveMouseCallback( m_PrimaryFirePressedToken );
        pInputManager->RemoveMouseCallback( m_PrimaryFireHeldToken );
        pInputManager->RemoveMouseCallback( m_PrimaryFireReleasedToken );
    }
}

void ControllerPlayer::Update( float delta )
{
    SDL_assert( g_pGame->GetPlayer() != nullptr );
    SDL_assert( g_pGame->GetPlayer()->GetShip() == GetShip() );

    if ( IsSuspended() )
    {
        GetShip()->SetThrust( ShipThrust::None );
        GetShip()->SetSteer( ShipSteer::None );
        GetShip()->SetStrafe( ShipStrafe::None );
        GetShip()->SetDodge( ShipDodge::None );
        return;
    }

    m_DodgeLeftTimer -= delta;
    m_DodgeRightTimer -= delta;

    Genesis::InputManager* pInputManager = Genesis::FrameWork::GetInputManager();
    const glm::vec2& mousePosition = pInputManager->GetMousePosition();
    const glm::vec3 raycastResult = Genesis::FrameWork::GetRenderSystem()->Raycast( mousePosition );
    glm::vec3 weaponTarget( raycastResult.x, raycastResult.y, 0.0f );

    for ( auto& pWeaponModule : GetShip()->GetWeaponModules() )
    {
        Weapon* pWeapon = pWeaponModule->GetWeapon();
        if ( pWeapon == nullptr || pWeaponModule->IsDestroyed() )
        {
            continue;
        }

        WeaponBehaviour behaviour = pWeapon->GetInfo()->GetBehaviour();
        if ( behaviour == WeaponBehaviour::Turret )
        {
            pWeapon->TurnTowards( weaponTarget );

            if ( Genesis::Configuration::GetFireToggle() && m_Fire && pWeapon->CanFire() )
            {
                pWeapon->Fire();
            }
        }

        if ( pWeapon->CanFire() && g_pGame->IsInputBlocked() == false )
        {
            if ( behaviour == WeaponBehaviour::Fixed && pInputManager->IsMouseButtonPressed( Genesis::MouseButton::Right ) )
            {
                pWeapon->Fire();
            }
        }

        if ( pWeapon->IsFiring() )
        {
            if ( behaviour == WeaponBehaviour::Fixed && !pInputManager->IsMouseButtonPressed( Genesis::MouseButton::Right ) )
            {
                pWeapon->StopFiring();
            }
        }
    }

    AddonModuleList addonModules = GetShip()->GetAddonModules();
    SDL_Scancode addonKeys[ sNumShipAddons ] = { SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4, SDL_SCANCODE_5, SDL_SCANCODE_6 };
    static bool addonKeysPressed[ sNumShipAddons ] = { false, false, false, false, false, false };
    const size_t numAddons = addonModules.size();
    SDL_assert( numAddons <= sNumShipAddons );
    int i = 0;

    addonModules.sort( AddonModuleSortPredicate );

    addonModules.remove_if(
        []( const AddonModule* pAddonModule ) {
            return ( (AddonInfo*)( pAddonModule->GetModuleInfo() ) )->GetCategory() == AddonCategory::QuantumStateAlternator;
        } );

    for ( AddonModuleList::const_iterator it = addonModules.begin(); it != addonModules.end(); it++ )
    {
        AddonModule* pAddonModule = *it;
        if ( pAddonModule->IsDestroyed() )
        {
            addonKeysPressed[ i ] = false;
        }
        else
        {
            Addon* pAddon = pAddonModule->GetAddon();
            SDL_assert( pAddon != nullptr );

            if ( pInputManager->IsButtonPressed( addonKeys[ i ] ) && pAddon->CanUse() && addonKeysPressed[ i ] == false )
            {
                AddonInfo* pAddonInfo = static_cast<AddonInfo*>( pAddonModule->GetModuleInfo() );
                if ( pAddonInfo->GetType() == AddonActivationType::Toggle && pAddon->IsActive() )
                {
                    pAddon->Deactivate();
                }
                else
                {
                    pAddonModule->GetAddon()->Activate();
                }
                addonKeysPressed[ i ] = true;
            }
            else if ( pInputManager->IsButtonPressed( addonKeys[ i ] ) == false )
            {
                addonKeysPressed[ i ] = false;
            }
        }
        i++;
    }

    Sector* pSector = g_pGame->GetCurrentSector();
    if ( pSector != nullptr )
    {
        Shipyard* pShipyard = pSector->GetShipyard();
        if ( pShipyard != nullptr )
        {
            float distanceToShipyard = glm::distance( pShipyard->GetPosition(), GetShip()->GetTowerPosition() );
            if ( distanceToShipyard <= pShipyard->GetDockingRange() )
            {
                if ( pInputManager->IsButtonPressed( SDL_SCANCODE_F ) )
                {
                    pSector->GetShipyard()->Dock( GetShip() );
                }
            }
        }
    }
}

void ControllerPlayer::OnForwardAction()
{
    GetShip()->SetThrust( ShipThrust::Forward );
}

void ControllerPlayer::OnBackwardAction()
{
    GetShip()->SetThrust( ShipThrust::Backward );
}

void ControllerPlayer::OnSteerLeftAction()
{
    GetShip()->SetSteer( ShipSteer::Left );
}

void ControllerPlayer::OnSteerRightAction()
{
    GetShip()->SetSteer( ShipSteer::Right );
}

void ControllerPlayer::OnDodgeLeftAction()
{
    if ( m_DodgeLeftTimer > 0.0f )
    {
        GetShip()->SetDodge( ShipDodge::Left );
    }
    else
    {
        m_DodgeLeftTimer = 0.5f;
    }
}

void ControllerPlayer::OnDodgeRightAction()
{
    if ( m_DodgeRightTimer > 0.0f )
    {
        GetShip()->SetDodge( ShipDodge::Right );
    }
    else
    {
        m_DodgeRightTimer = 0.5f;
    }
}

void ControllerPlayer::OnStrafeLeftAction()
{
    GetShip()->SetStrafe( ShipStrafe::Left );
}

void ControllerPlayer::OnStrafeRightAction()
{
    GetShip()->SetStrafe( ShipStrafe::Right );
}

void ControllerPlayer::OnRammingSpeedAction()
{
    // TODO: The ship should have the responsibility of checking its perks
    // to see if ramming speed can be triggered or not, not the controller.
    // This would require each faction to have its own set of perks so
    // ramming speed still works for the Marauders.
    if ( GetShip()->HasPerk( Perk::RammingSpeed ) )
    {
        GetShip()->RammingSpeed();
    }
}

void ControllerPlayer::OnFlipQuantumStateAction()
{
    GetShip()->FlipQuantumState();
}

void ControllerPlayer::OnPrimaryFirePressedAction()
{
    if ( Genesis::Configuration::GetFireToggle() == false || g_pGame->IsInputBlocked() || GetShip()->GetHyperspaceCore()->IsJumping() )
    {
        return;
    }

    m_Fire = !m_Fire;

    for ( auto& pWeaponModule : GetShip()->GetWeaponModules() )
    {
        Weapon* pWeapon = pWeaponModule->GetWeapon();
        if ( pWeapon == nullptr || pWeaponModule->IsDestroyed() )
        {
            continue;
        }

        WeaponBehaviour behaviour = pWeapon->GetInfo()->GetBehaviour();
        if ( behaviour == WeaponBehaviour::Turret )
        {
            if ( m_Fire && pWeapon->CanFire() )
            {
                pWeapon->Fire();
            }
            else if ( m_Fire == false && pWeapon->IsFiring() )
            {
                pWeapon->StopFiring();
            }
        }
    }
}

void ControllerPlayer::OnPrimaryFireHeldAction()
{
    // This is only relevant if the fire toggle is disabled, and primary
    // weapons keep firing until the button is released.
    // The logic for fire toggle is handled in OnPrimaryFirePressedAction().
    if ( Genesis::Configuration::GetFireToggle() )
    {
        return;
    }

    if ( GetShip()->GetHyperspaceCore()->IsJumping() || g_pGame->IsInputBlocked() )
    {
        return;
    }

    for ( auto& pWeaponModule : GetShip()->GetWeaponModules() )
    {
        Weapon* pWeapon = pWeaponModule->GetWeapon();
        if ( pWeapon == nullptr || pWeaponModule->IsDestroyed() )
        {
            continue;
        }

        WeaponBehaviour behaviour = pWeapon->GetInfo()->GetBehaviour();
        if ( pWeapon->CanFire() && behaviour == WeaponBehaviour::Turret )
        {
            pWeapon->Fire();
        }
    }
}

void ControllerPlayer::OnPrimaryFireReleasedAction()
{
    // This is only relevant if the fire toggle is disabled, and primary
    // weapons keep firing until the button is released.
    // The logic for fire toggle is handled in OnPrimaryFirePressedAction().
    if ( Genesis::Configuration::GetFireToggle() )
    {
        return;
    }

    for ( auto& pWeaponModule : GetShip()->GetWeaponModules() )
    {
        Weapon* pWeapon = pWeaponModule->GetWeapon();
        WeaponBehaviour behaviour = pWeapon->GetInfo()->GetBehaviour();
        if ( pWeapon != nullptr && pWeaponModule->IsDestroyed() == false && pWeapon->IsFiring() && behaviour == WeaponBehaviour::Turret )
        {
            pWeapon->StopFiring();
        }
    }
}

} // namespace Hexterminate
