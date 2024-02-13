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

#include "misc/randomshuffle.h"

namespace Hexterminate
{

std::default_random_engine RandomShuffle::m_Engine = std::default_random_engine();

void RandomShuffle::Initialise()
{
    const unsigned int seed = static_cast<unsigned int>( std::chrono::system_clock::now().time_since_epoch().count() );
    m_Engine = std::default_random_engine( seed );
}

} // namespace Hexterminate
