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

#include <cstdint>
#include <random>

namespace Hexterminate
{

//-----------------------------------------------------------------------------
// HyperscapeLocation
//-----------------------------------------------------------------------------

class Random
{
public:
    static void Initialise();

    static uint32_t Next(); // Returns a value between 0 and 2^32-1.
    static uint32_t Next( uint32_t max ); // Returns a value between 0 and max - 1.
    static uint32_t Next( uint32_t min, uint32_t max ); // Returns a value between min and max - 1.
    static float Next( float max ); // Returns a value between 0.0f and max.
    static float Next( float min, float max ); // returns a value between min and max.

private:
    static std::default_random_engine m_Engine;
};

} // namespace Hexterminate
