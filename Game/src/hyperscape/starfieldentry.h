// Copyright 2021 Pedro Nunes
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

#include <glm/common.hpp>
#include <glm/vec3.hpp>

#include "misc/random.h"

namespace Hexterminate
{

//-----------------------------------------------------------------------------
// StarfieldEntry
//-----------------------------------------------------------------------------

class StarfieldEntry
{
public:
    StarfieldEntry();
    StarfieldEntry( const glm::vec3& startPos, const glm::vec3& endPos, bool randomProgress );
    void Update( float delta );
    bool IsFinished() const;
    const glm::vec3& GetPosition() const;
    const float GetSize() const;
    const float GetProgress() const;

private:
    glm::vec3 m_StartPos;
    glm::vec3 m_EndPos;
    glm::vec3 m_Position;
    float m_Fraction;
    float m_Speed;
    float m_Size;
};

inline StarfieldEntry::StarfieldEntry()
    : m_StartPos( 0.0f, 0.0f, 0.0f )
    , m_EndPos( 0.0f, 0.0f, 0.0f )
    , m_Position( 0.0f, 0.0f, 0.0f )
    , m_Fraction( 0.0f )
    , m_Speed( 0.0f )
    , m_Size( 0.0f )
{
}

inline StarfieldEntry::StarfieldEntry( const glm::vec3& startPos, const glm::vec3& endPos, bool randomProgress )
    : m_StartPos( startPos )
    , m_EndPos( endPos )
    , m_Position( startPos )
    , m_Fraction( 0.0f )
    , m_Speed( 1.0f )
    , m_Size( 0.0f )
{
    m_Fraction = randomProgress ? Random::Next( 1.0f ) : 0.0f;
    m_Speed = 1.0f + Random::Next( 1.0f );
    m_Size = 16.0f + Random::Next( 16.0f, 32.0f );
}

inline void StarfieldEntry::Update( float delta )
{
    m_Fraction = glm::min( m_Fraction + delta / 20.0f * m_Speed, 1.0f );
    m_Position = glm::mix( m_StartPos, m_EndPos, m_Fraction );
    m_Position.z = m_Fraction;
}

inline bool StarfieldEntry::IsFinished() const
{
    return m_Fraction >= 1.0f;
}

inline const glm::vec3& StarfieldEntry::GetPosition() const
{
    return m_Position;
}

inline const float StarfieldEntry::GetSize() const
{
    return m_Size * m_Fraction;
}

inline const float StarfieldEntry::GetProgress() const
{
    return m_Fraction;
}

} // namespace Hexterminate
