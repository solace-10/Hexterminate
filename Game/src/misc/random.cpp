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

#include <chrono>

#include <SDL_assert.h>

#include "misc/random.h"

namespace Hexterminate
{

//-----------------------------------------------------------------------------
// Random
//-----------------------------------------------------------------------------

std::default_random_engine Random::m_Engine = std::default_random_engine();

void Random::Initialise()
{
    const unsigned int seed = static_cast<unsigned int>( std::chrono::system_clock::now().time_since_epoch().count() );
    m_Engine = std::default_random_engine( seed );
}

uint32_t Random::Next()
{
    return static_cast<uint32_t>( m_Engine() % UINT32_MAX );
}

uint32_t Random::Next( uint32_t max )
{
    return static_cast<uint32_t>( m_Engine() % max );
}

uint32_t Random::Next( uint32_t min, uint32_t max )
{
    SDL_assert( min < max );
    return min + static_cast<uint32_t>( m_Engine() % ( max - min ) );
}

float Random::Next( float max )
{
    SDL_assert( max > 0.0f );
    std::uniform_real_distribution<float> dist( 0.0f, max );
    return dist( m_Engine );
}

float Random::Next( float min, float max )
{
    SDL_assert( min < max );
    std::uniform_real_distribution<float> dist( min, max );
    return dist( m_Engine );
}

} // namespace Hexterminate
