// Copyright 2015 Pedro Nunes
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

#include "ammo/missile.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// Rocket
///////////////////////////////////////////////////////////////////////////////

class Rocket : public Missile
{
public:
    Rocket();

protected:
    virtual MissileType GetType() const override;
    virtual const std::string GetResourceName() const override;
    virtual Trail* CreateTrail() const override;
};

} // namespace Hexterminate
