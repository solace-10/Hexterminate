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

#include "ship/controller/controllerai.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// ControllerAssault
// Assault behaviour for AI ships.
// It will always try and get close to the target ship.
///////////////////////////////////////////////////////////////////////////////

class ControllerAssault : public ControllerAI
{
public:
    ControllerAssault( Ship* pShip );
    virtual ~ControllerAssault(){};
    virtual void HandleOrders( float delta ) override;

private:
    bool ApproachTargetShip();
    
    bool m_HasRammingProws;
};

} // namespace Hexterminate
