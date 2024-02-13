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

#include <glm/vec3.hpp>

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// Particle
///////////////////////////////////////////////////////////////////////////////

class Particle
{
public:
    Particle();
    ~Particle(){};

    void Reset();

    const glm::vec3& GetPosition() const;
    void SetPosition( const glm::vec3& position );
    void SetLifetime( float value );
    void SetScale( float value );
    bool IsAlive() const;
    void SetVelocity( const glm::vec3& velocity );
    void SetAlpha( float value );

    void Update( float delta );

    float GetCurrentLifetime() const;
    float GetInitialLifetime() const;
    float GetScale() const;
    float GetAlpha() const;

private:
    glm::vec3 m_Position;
    glm::vec3 m_Velocity;
    float m_Lifetime;
    float m_InitialLifetime;
    float m_Scale;
    float m_Alpha;
};

inline const glm::vec3& Particle::GetPosition() const
{
    return m_Position;
}

inline void Particle::SetPosition( const glm::vec3& position )
{
    m_Position = position;
}

inline void Particle::SetLifetime( float value )
{
    m_Lifetime = value;
    m_InitialLifetime = value;
}

inline bool Particle::IsAlive() const
{
    return m_Lifetime > 0.0f;
}

inline float Particle::GetCurrentLifetime() const
{
    return m_Lifetime;
}

inline float Particle::GetInitialLifetime() const
{
    return m_InitialLifetime;
}

inline float Particle::GetScale() const
{
    return m_Scale;
}

inline void Particle::SetScale( float value )
{
    m_Scale = value;
}

inline void Particle::SetVelocity( const glm::vec3& velocity )
{
    m_Velocity = velocity;
}

inline float Particle::GetAlpha() const
{
    return m_Alpha;
}

inline void Particle::SetAlpha( float value )
{
    m_Alpha = value;
}

} // namespace Hexterminate
