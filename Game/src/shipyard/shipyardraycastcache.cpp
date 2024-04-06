// Copyright 2024 Pedro Nunes
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

#include <SDL.h>

#include "shipyard/shipyardraycastcache.h"

#include "ship/module.h"

namespace Hexterminate
{

ShipyardRaycastCache::ShipyardRaycastCache( Ship* pShip, const glm::vec3& shipyardPosition )
{
    for ( int x = 0; x < sHexGridWidth; x++ )
    {
        for ( int y = 0; y < sHexGridHeight; y++ )
        {
            m_Positions[ x ][ y ] = Module::GetLocalPosition( pShip, x, y ) + shipyardPosition;
        }
    }
}

ShipyardRaycastCache::~ShipyardRaycastCache()
{
}

const glm::vec3& ShipyardRaycastCache::Get( int x, int y ) const
{
    SDL_assert( x >= 0 );
    SDL_assert( x < sHexGridWidth );
    SDL_assert( y >= 0 );
    SDL_assert( y < sHexGridHeight );
    return m_Positions[ x ][ y ];
}

} // namespace Hexterminate