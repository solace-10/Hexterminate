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

#include "ship/controller/controller.h"

// clang-format off
#include <beginexternalheaders.h>
#include <glm/vec3.hpp>
#include <endexternalheaders.h>
// clang-format on

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// ControllerShipyard
// Temporary controller while a ship is using a shipyard, handling docking and
// undocking.
///////////////////////////////////////////////////////////////////////////////

class ControllerShipyard : public Controller
{
public:
    ControllerShipyard( Ship* pShip );
    virtual ~ControllerShipyard();
    virtual void Update( float delta );

private:
    glm::vec3 m_StartPosition;
    float m_LerpDuration;
    float m_LerpTimer;
};

} // namespace Hexterminate
