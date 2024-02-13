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
// ControllerKiter
// Kiting behaviour for AI ships.
// Meant for agile ships, it attempts to keep the target at range while
// circling it.
///////////////////////////////////////////////////////////////////////////////

class ControllerKiter : public ControllerAI
{
public:
    ControllerKiter( Ship* pShip );
    virtual ~ControllerKiter(){};
    virtual void HandleOrders( float delta ) override;

private:
    bool OrbitTargetShip();
};

} // namespace Hexterminate
