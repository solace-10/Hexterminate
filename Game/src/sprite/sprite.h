// Copyright 2016 Pedro Nunes
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

#include <color.h>

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// Sprite
// It needs to be passed to the SpriteManager every frame it needs to be
// rendered and deals entirely in world space coordinates.
///////////////////////////////////////////////////////////////////////////////

class Sprite
{
public:
    Sprite()
        : m_Source( 0.0f, 0.0f, 0.0f )
        , m_Destination( 1.0f, 0.0f, 0.0f )
        , m_Colour( 1.0f, 1.0f, 1.0f, 1.0f )
        , m_Width( 1.0f )
        , m_SpriteIdx( 0 )
    {
    }

    Sprite( const glm::vec3& source, const glm::vec3& destination, const Genesis::Color& colour, float width, unsigned int spriteIdx )
        : m_Source( source )
        , m_Destination( destination )
        , m_Colour( colour )
        , m_Width( width )
        , m_SpriteIdx( spriteIdx )
    {
    }

    const glm::vec3& GetSource() const;
    const glm::vec3& GetDestination() const;
    const Genesis::Color& GetColour() const;
    float GetWidth() const;
    unsigned int GetIndex() const;

private:
    glm::vec3 m_Source;
    glm::vec3 m_Destination;
    Genesis::Color m_Colour;
    float m_Width;
    unsigned int m_SpriteIdx;
};

inline const glm::vec3& Sprite::GetSource() const
{
    return m_Source;
}

inline const glm::vec3& Sprite::GetDestination() const
{
    return m_Destination;
}

inline const Genesis::Color& Sprite::GetColour() const
{
    return m_Colour;
}

inline float Sprite::GetWidth() const
{
    return m_Width;
}

inline unsigned int Sprite::GetIndex() const
{
    return m_SpriteIdx;
}

} // namespace Hexterminate