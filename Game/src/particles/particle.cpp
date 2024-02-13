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

#include "particles/particle.h"
#include <math/misc.h>

namespace Hexterminate
{

Particle::Particle()
{
    Reset();
}

void Particle::Reset()
{
    m_InitialLifetime = 0.0f;
    m_Lifetime = 0.0f;
    m_Position = glm::vec3( 0.0f );
    m_Scale = 1.0f;
    m_Velocity = glm::vec3( 0.0f );
    m_Alpha = 1.0f;
}

void Particle::Update( float delta )
{
    m_Lifetime = gMax( 0.0f, m_Lifetime - delta );

    m_Position += m_Velocity * delta;

    float r = gClamp<float>( m_Lifetime / m_InitialLifetime, 0.0f, 1.0f );
    if ( r > 0.25f )
        m_Alpha = 1.0f;
    else
        m_Alpha = r * 4.0f;
}

} // namespace Hexterminate