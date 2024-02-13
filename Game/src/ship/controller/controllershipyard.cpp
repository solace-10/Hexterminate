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

#include "ship/controller/controllershipyard.h"
#include "hexterminate.h"
#include "player.h"
#include "sector/sector.h"
#include "ship/ship.h"
#include "shipyard/shipyard.h"

#include <genesis.h>
#include <inputmanager.h>
#include <physics/rigidbody.h>

namespace Hexterminate
{

ControllerShipyard::ControllerShipyard( Ship* pShip )
    : Controller( pShip )
    , m_LerpDuration( 5.0f )
    , m_LerpTimer( 0.0f )
{
    SDL_assert( pShip != nullptr );

    Genesis::Physics::RigidBody* pRigidBody = pShip->GetRigidBody();
    SDL_assert( pRigidBody != nullptr );
    pRigidBody->SetMotionType( Genesis::Physics::MotionType::Static );
    pRigidBody->SetLinearVelocity( glm::vec3( 0.0f, 0.0f, 0.0f ) );
    pRigidBody->SetAngularVelocity( glm::vec3( 0.0f, 0.0f, 0.0f ) );

    Sector* pSector = g_pGame->GetCurrentSector();
    SDL_assert( pSector != nullptr );
    Shipyard* pShipyard = pSector->GetShipyard();
    SDL_assert( pShipyard != nullptr );
    m_StartPosition = pShipyard->GetPosition() - glm::vec3( 0.0f, 400.0f, 0.0f );
}

ControllerShipyard::~ControllerShipyard()
{
    GetShip()->GetRigidBody()->SetMotionType( Genesis::Physics::MotionType::Dynamic );
}

void ControllerShipyard::Update( float delta )
{
    Sector* pSector = g_pGame->GetCurrentSector();
    SDL_assert( pSector != nullptr );
    Shipyard* pShipyard = pSector->GetShipyard();
    SDL_assert( pShipyard != nullptr );

    DockingState dockingState = GetShip()->GetDockingState();
    if ( dockingState == DockingState::Docking )
    {
        m_LerpTimer += delta;
        float t = m_LerpTimer / m_LerpDuration;
        if ( t > 1.0f )
        {
            t = 1.0f;
            GetShip()->NotifyDockingFinished();
        }

        float f = std::pow( t - 1.0f, 3.0f ) + 1.0f;

        glm::vec3 shipPosition = glm::mix( m_StartPosition, pShipyard->GetPosition(), f );

        if ( GetShip() != nullptr && GetShip()->GetRigidBody() != nullptr )
        {
            GetShip()->GetRigidBody()->SetWorldTransform( glm::translate( shipPosition ) );
        }
    }
    else if ( dockingState == DockingState::Docked && GetShip()->IsModuleEditLocked() )
    {
        Genesis::InputManager* pInputManager = Genesis::FrameWork::GetInputManager();
        if ( pInputManager->IsButtonPressed( SDL_SCANCODE_ESCAPE ) )
        {
            pShipyard->Undock();
        }
    }
}

} // namespace Hexterminate