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

#include <vector>

namespace Hexterminate
{

typedef std::vector<float> FloatVector;

static const int sMaxShieldHitRegistryPoints = 180;

class ShieldHitRegistry
{
public:
    ShieldHitRegistry();

    void Update( float delta );
    void Hit( float angle, float damage );
    float SampleAt( int index ) const;

    void DebugDraw( float x, float y );

private:
    float EvaluateGaussian( float x, float mean, float deviation ) const;
    int GetIndex( float angle ) const;
    int GetNextIndex( int index ) const;
    int GetPreviousIndex( int index ) const;

    FloatVector m_HitRegistry;
};

inline int ShieldHitRegistry::GetNextIndex( int index ) const
{
    return ( index < sMaxShieldHitRegistryPoints - 1 ) ? index + 1 : 0;
}

inline int ShieldHitRegistry::GetPreviousIndex( int index ) const
{
    return ( index <= 0 ) ? sMaxShieldHitRegistryPoints - 1 : index - 1;
}

inline float ShieldHitRegistry::EvaluateGaussian( float x, float mean, float deviation ) const
{
    return exp( -pow( x - mean, 2.0f ) / ( 2.0f * deviation * deviation ) );
}

} // namespace Hexterminate