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

#include "ammo/torpedo.h"
#include "trail/trail.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// Torpedo
///////////////////////////////////////////////////////////////////////////////

Torpedo::Torpedo()
{
    SetGlowColor( Genesis::Color( 1.0f, 1.0f, 1.0f, 1.0f ) );
}

MissileType Torpedo::GetType() const
{
    return MissileType::Torpedo;
}

const std::string Torpedo::GetResourceName() const
{
    static const std::string sResourceName( "data/models/ammo/torpedo.tmf" );
    return sResourceName;
}

Trail* Torpedo::CreateTrail() const
{
    return new Trail( 5.0f, 2.0f, Genesis::Color( 0.6f, 0.2f, 0.2f, 0.5f ) );
}

} // namespace Hexterminate
