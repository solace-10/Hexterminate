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

#pragma once

#include <inputmanager.h>

#include "ship/controller/controller.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// ControllerPlayer
// Base class for Player controlled ships, handling keyboard and mouse input.
///////////////////////////////////////////////////////////////////////////////

class ControllerPlayer : public Controller
{
public:
    ControllerPlayer( Ship* pShip );
    virtual ~ControllerPlayer() override;
    virtual void Update( float delta ) override;

private:
    void OnForwardAction();
    void OnBackwardAction();
    void OnSteerLeftAction();
    void OnSteerRightAction();
    void OnDodgeLeftAction();
    void OnDodgeRightAction();
    void OnStrafeLeftAction();
    void OnStrafeRightAction();
    void OnRammingSpeedAction();
    void OnFlipQuantumStateAction();
    void OnPrimaryFirePressedAction();
    void OnPrimaryFireHeldAction();
    void OnPrimaryFireReleasedAction();

    Genesis::InputCallbackToken m_ForwardToken;
    Genesis::InputCallbackToken m_BackwardToken;
    Genesis::InputCallbackToken m_DodgeLeftToken;
    Genesis::InputCallbackToken m_DodgeRightToken;
    Genesis::InputCallbackToken m_SteerLeftToken;
    Genesis::InputCallbackToken m_SteerRightToken;
    Genesis::InputCallbackToken m_StrafeLeftToken;
    Genesis::InputCallbackToken m_StrafeRightToken;
    Genesis::InputCallbackToken m_RammingSpeedToken;
    Genesis::InputCallbackToken m_FlipQuantumStateToken;
    Genesis::InputCallbackToken m_PrimaryFirePressedToken;
    Genesis::InputCallbackToken m_PrimaryFireHeldToken;
    Genesis::InputCallbackToken m_PrimaryFireReleasedToken;
    bool m_Fire;
    float m_DodgeLeftTimer;
    float m_DodgeRightTimer;
};

} // namespace Hexterminate
