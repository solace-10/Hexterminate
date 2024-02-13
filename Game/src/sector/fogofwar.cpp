// Copyright 2020 Pedro Nunes
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

#include "fogofwar.h"
#include "sectorinfo.h"

namespace Hexterminate
{

static const float sRevealDuration = 10.0f;

FogOfWar::FogOfWar()
{
    for ( int x = 0; x < NumSectorsX; ++x )
    {
        for ( int y = 0; y < NumSectorsY; ++y )
        {
            m_Tiles[ x ][ y ] = 0.0f;
        }
    }
}

void FogOfWar::Update( float delta )
{
    for ( int x = 0; x < NumSectorsX; ++x )
    {
        for ( int y = 0; y < NumSectorsY; ++y )
        {
            m_Tiles[ x ][ y ] = std::max( 0.0f, m_Tiles[ x ][ y ] - delta );
        }
    }
}

void FogOfWar::MarkAsVisible( const SectorInfo* pSectorInfo, int radius /* = 0 */ )
{
    SDL_assert( radius >= 0 && radius <= 2 );

    glm::ivec2 coords = pSectorInfo->GetCoordinates();
    MarkAsVisibleSingle( coords.x, coords.y );

    if ( radius == 1 )
    {
        MarkAsVisibleSingle( coords.x - 1, coords.y );
        MarkAsVisibleSingle( coords.x + 1, coords.y );
        MarkAsVisibleSingle( coords.x, coords.y - 1 );
        MarkAsVisibleSingle( coords.x, coords.y + 1 );
    }
    else if ( radius == 2 )
    {
        MarkAsVisibleSingle( coords.x - 2, coords.y );
        MarkAsVisibleSingle( coords.x - 1, coords.y );
        MarkAsVisibleSingle( coords.x - 1, coords.y - 1 );
        MarkAsVisibleSingle( coords.x - 1, coords.y + 1 );
        MarkAsVisibleSingle( coords.x, coords.y - 2 );
        MarkAsVisibleSingle( coords.x, coords.y - 1 );
        MarkAsVisibleSingle( coords.x, coords.y + 1 );
        MarkAsVisibleSingle( coords.x, coords.y + 2 );
        MarkAsVisibleSingle( coords.x + 1, coords.y );
        MarkAsVisibleSingle( coords.x + 1, coords.y - 1 );
        MarkAsVisibleSingle( coords.x + 1, coords.y + 1 );
        MarkAsVisibleSingle( coords.x + 2, coords.y );
    }
}

void FogOfWar::MarkAsVisibleSingle( int x, int y )
{
    if ( x >= 0 && x < NumSectorsX && y >= 0 && y < NumSectorsY )
    {
        m_Tiles[ x ][ y ] = sRevealDuration;
    }
}

bool FogOfWar::IsVisible( const SectorInfo* pSectorInfo ) const
{
    glm::ivec2 coords = pSectorInfo->GetCoordinates();
    return m_Tiles[ coords.x ][ coords.y ] > 0.0f;
}

} // namespace Hexterminate