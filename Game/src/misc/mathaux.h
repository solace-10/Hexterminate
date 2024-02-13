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

#include <cmath>

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Math
{

// Rotates a vector around the Z axis, a simpler version of Havok's glm::vec3f::_setRotatedDir
// since the gameplay all happens on the XY plane.
inline void RotateVector( glm::vec3& vec, float angle )
{
    const float cs = cosf( angle );
    const float sn = sinf( angle );
    vec.x = vec.x * cs - vec.y * sn;
    vec.y = vec.x * sn + vec.y * cs;
}

// Returns the angle in radians between two vectors.
inline float AngleBetween( const glm::vec2& v1, const glm::vec2& v2 )
{
    const glm::vec2 dir = glm::normalize( v2 - v1 );
    const float dot = glm::dot( glm::vec2( 1.0f, 0.0f ), dir );
    return glm::acos( dot );
}

class IPoint2
{
public:
    IPoint2()
        : x( 0 )
        , y( 0 ){};
    IPoint2( int _x, int _y )
        : x( _x )
        , y( _y ){};
    int x;
    int y;
};

class FPoint2
{
public:
    FPoint2()
        : x( 0.0f )
        , y( 0.0f ){};
    FPoint2( float _x, float _y )
        : x( _x )
        , y( _y ){};
    float x;
    float y;
};

inline bool IntersectCircleRect( const glm::vec2& circleOrigin, float circleRadius, const glm::vec2& rectTopLeft, const glm::vec2& rectBottomRight )
{
    // Find the closest point to the circle within the rectangle
    const float closestX = glm::clamp( circleOrigin.x, rectTopLeft.x, rectBottomRight.x );
    const float closestY = glm::clamp( circleOrigin.y, rectBottomRight.y, rectTopLeft.y );

    // Calculate the distance between the circle's center and this closest point
    const float distanceX = circleOrigin.x - closestX;
    const float distanceY = circleOrigin.y - closestY;

    // If the distance is less than the circle's radius, an intersection occurs
    const float distanceSquared = ( distanceX * distanceX ) + ( distanceY * distanceY );
    return distanceSquared < ( circleRadius * circleRadius );
}

} // namespace Math