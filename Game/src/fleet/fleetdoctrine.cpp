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

#include "fleet/fleetdoctrine.h"

// clang-format off
#include <beginexternalheaders.h>
#include <SDL.h>
#include <endexternalheaders.h>
// clang-format on

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FleetDoctrine
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FleetDoctrine::FleetDoctrine()
{
    m_BehaviourType = FleetBehaviourType::None;
    m_Ratio = { 0.0f };

    m_Ratio[ static_cast<size_t>( ShipType::Gunship ) ] = 1.0f;
    m_Ratio[ static_cast<size_t>( ShipType::Battlecruiser ) ] = 0.0f;
    m_Ratio[ static_cast<size_t>( ShipType::Capital ) ] = 0.0f;
}

FleetDoctrine::FleetDoctrine( FleetBehaviourType behaviourType, float gunships, float battlecruisers, float capitals )
{
    m_BehaviourType = behaviourType;
    m_Ratio = { 0.0f };

    const float l = ( gunships + battlecruisers + capitals );
    SDL_assert( l > 0.0f );
    m_Ratio[ static_cast<size_t>( ShipType::Gunship ) ] = gunships / l;
    m_Ratio[ static_cast<size_t>( ShipType::Battlecruiser ) ] = battlecruisers / l;
    m_Ratio[ static_cast<size_t>( ShipType::Capital ) ] = capitals / l;
}

} // namespace Hexterminate
