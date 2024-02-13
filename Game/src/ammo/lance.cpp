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

#include <cassert>

#include <math/misc.h>
#include <rendersystem.h>
#include <resources/resourceimage.h>

#include "ammo/lance.h"

namespace Hexterminate
{

Lance::Lance()
    : m_Lifetime( FLT_MAX )
    , m_MaxLifetime( FLT_MAX )
{
}

void Lance::Create( Weapon* pWeapon, float additionalRotation /* = 0.0f */ )
{
    Beam::Create( pWeapon, additionalRotation );

    m_MaxLifetime = pWeapon->GetInfo()->GetBeamLifetime();
    m_Lifetime = m_MaxLifetime;
}

void Lance::Update( float delta )
{
    Beam::Update( delta );

    m_Lifetime -= delta;
    if ( m_Lifetime < 0.0f )
    {
        Kill();
    }
}

float Lance::GetOpacity() const
{
    float r = 1.0f - m_Lifetime / m_MaxLifetime;
    float opacity = 1.0f;
    if ( r > 0.5f )
    {
        opacity = 1.0f - ( r - 0.5f ) * ( r - 0.5f ) * 4.0f;
        opacity = gClamp<float>( opacity, 0.0, 1.0f );
    }

    return opacity;
}

} // namespace Hexterminate