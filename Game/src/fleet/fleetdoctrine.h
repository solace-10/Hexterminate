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

#pragma once

#include <array>

#include "fleet/fleetbehaviour.h"
#include "ship/ship.fwd.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FleetDoctrine
// This represents the ideal fleet composition for a certain faction.
// E.g. a fleet can be capital heavy or have a preference for large numbers of gunships.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FleetDoctrine
{
public:
    FleetDoctrine();
    FleetDoctrine( FleetBehaviourType behaviourType, float gunships, float battlecruisers, float capitals );
    ~FleetDoctrine(){};

    float GetRatio( ShipType type ) const;
    FleetBehaviourType GetBehaviourType() const { return m_BehaviourType; }

private:
    std::array<float, static_cast<size_t>( ShipType::Count )> m_Ratio;
    FleetBehaviourType m_BehaviourType;
};

inline float FleetDoctrine::GetRatio( ShipType type ) const
{
    return m_Ratio[ static_cast<int>( type ) ];
}

} // namespace Hexterminate
