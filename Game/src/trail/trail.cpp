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

#include "trail/trail.h"

namespace Hexterminate
{

Trail::Trail( float initialWidth, float decay, const Genesis::Color& colour )
    : m_InitialWidth( initialWidth )
    , m_Decay( decay )
    , m_Colour( colour )
    , m_IsOrphan( false )
    , m_ActivePoints( 0 )
{
}

void Trail::AddPoint( const glm::vec3& position )
{
    if ( m_Data.size() < 2 )
    {
        m_Data.push_front( TrailPointData( position, m_InitialWidth ) );
    }
    else
    {
        // The first point always remains at the source of the trail
        TrailPointData point( position, m_InitialWidth );
        m_Data.front() = point;

        // If the first and second points are too far apart, we place a new one
        TrailPointDataList::iterator secondPointIt = ++( m_Data.begin() );
        static const float sThreshold = 10.0f;
        if ( glm::distance( secondPointIt->GetPosition(), point.GetPosition() ) >= sThreshold )
        {
            m_Data.push_front( point );
        }
    }
}

// The trail decays over time.
// The decay is faster if the trail has been orphaned, so it doesn't linger around if
// the source of the trail no longer exists or is no longer powered.
void Trail::Update( float delta )
{
    m_ActivePoints = 0;
    if ( m_Data.empty() == false )
    {
        for ( TrailPointDataList::iterator it = m_Data.begin(), itEnd = m_Data.end(); it != itEnd; )
        {
            TrailPointData& point = *it;
            const float pointWidth = point.GetWidth();
            if ( pointWidth > 0.0f )
            {
                const float fadedWidth = pointWidth - m_Decay * delta * ( IsOrphan() ? 3.0f : 1.0f );
                if ( fadedWidth <= 0.0f )
                {
                    it = m_Data.erase( it );
                }
                else
                {
                    point.SetWidth( fadedWidth );
                    m_ActivePoints++;
                    it++;
                }
            }
        }
    }
}

} // namespace Hexterminate