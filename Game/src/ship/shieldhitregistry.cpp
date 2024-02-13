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

#include <cmath>

#include <math/constants.h>
#include <math/misc.h>
#include <render/debugrender.h>

#include "ship/shieldhitregistry.h"

#include "hexterminate.h"

namespace Hexterminate
{

ShieldHitRegistry::ShieldHitRegistry()
{
    m_HitRegistry.resize( sMaxShieldHitRegistryPoints );
    for ( int i = 0; i < sMaxShieldHitRegistryPoints; ++i )
    {
        m_HitRegistry[ i ] = 0.0f;
    }
}

void ShieldHitRegistry::Update( float delta )
{
    const float decay = 0.7f * delta;
    for ( int i = 0; i < sMaxShieldHitRegistryPoints; ++i )
    {
        m_HitRegistry[ i ] = gMax( m_HitRegistry[ i ] - decay, 0.0f );
    }
}

void ShieldHitRegistry::Hit( float angle, float damage )
{
    const float impact = gMax( damage / 20.0f, 5.0f ); // Keeps the minimum impact to a decent size - it looks too jaggy otherwise
    const int mean = GetIndex( angle );
    const int deviation = (int)( impact * gRand( 0.75f, 1.25f ) );
    const int minimumIndex = mean - deviation * 3; // 99.7% of the values in a gaussian curve are within 3 standard deviations
    const int maximumIndex = mean + deviation * 3;

    for ( int i = minimumIndex; i < maximumIndex; ++i )
    {
        int idx = i % sMaxShieldHitRegistryPoints;
        if ( idx < 0 ) // Wrap around for the modulus of negative numbers
            idx = sMaxShieldHitRegistryPoints + idx;
        m_HitRegistry[ idx ] = std::max( EvaluateGaussian( (float)i, (float)mean, (float)deviation ), m_HitRegistry[ idx ] );
    }
}

float ShieldHitRegistry::SampleAt( int index ) const
{
    return m_HitRegistry[ index % sMaxShieldHitRegistryPoints ];
}

int ShieldHitRegistry::GetIndex( float angle ) const
{
    angle = fmodf( angle, 2.0f * Genesis::kPi );
    if ( angle < 0.0f )
        angle += 2.0f * Genesis::kPi;

    const float r = angle / ( 2.0f * Genesis::kPi );
    const int index = static_cast<int>( floor( r * static_cast<float>( sMaxShieldHitRegistryPoints ) ) );
    return index;
}

void ShieldHitRegistry::DebugDraw( float xOrigin, float yOrigin )
{
    Genesis::Render::DebugRender* pDbgRender = Genesis::FrameWork::GetDebugRender();

    const float width = 360.0f;
    const float height = 100.0f;

    glm::vec2 origin( xOrigin, yOrigin );
    glm::vec2 xAxis = origin + glm::vec2( width, 0.0f );
    glm::vec2 yAxis = origin + glm::vec2( 0.0f, height );

    const glm::vec3 white( 1.0f, 1.0f, 1.0f );
    const glm::vec3 cyan( 0.0f, 1.0f, 1.0f );

    pDbgRender->DrawLine( origin, xAxis, white );
    pDbgRender->DrawLine( origin, yAxis, white );

    const float xStep = width / static_cast<float>( sMaxShieldHitRegistryPoints );
    float xOffset = 0.0f;
    for ( int i = 0; i < sMaxShieldHitRegistryPoints - 1; ++i )
    {
        float y1 = m_HitRegistry.at( i ) * height;
        float y2 = m_HitRegistry.at( i + 1 ) * height;
        pDbgRender->DrawLine(
            glm::vec2( origin.x + xOffset, origin.y + y1 ),
            glm::vec2( origin.x + xOffset + xStep, origin.y + y2 ),
            cyan );

        xOffset += xStep;
    }
}

} // namespace Hexterminate