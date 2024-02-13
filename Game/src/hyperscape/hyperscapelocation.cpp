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

#include "hyperscape/hyperscapelocation.h"

namespace Hexterminate
{

//-----------------------------------------------------------------------------
// HyperscapeLocation
//-----------------------------------------------------------------------------

HyperscapeLocation::HyperscapeLocation( const HyperscapeLocation& location )
    : m_Seed( location.m_Seed )
    , m_Depth( location.m_Depth )
    , m_Type( location.m_Type )
{
    m_pStarInfo = std::make_unique<StarInfo>( location.m_Seed );
}

HyperscapeLocation::HyperscapeLocation( Type type, uint32_t seed, uint32_t depth )
    : m_Seed( seed )
    , m_Depth( depth )
    , m_Type( type )
{
    m_pStarInfo = std::make_unique<StarInfo>( seed );
}

HyperscapeLocation::~HyperscapeLocation()
{
}

} // namespace Hexterminate
