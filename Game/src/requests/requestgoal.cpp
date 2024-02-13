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

#include "requests/requestgoal.h"
#include "requests/requestgoalrep.h"

namespace Hexterminate
{

RequestGoal::RequestGoal( FleetWeakPtr pFleet, const std::string& description, const Color& colour )
    : m_pFleet( pFleet )
    , m_pSectorInfo( nullptr )
    , m_Description( description )
{
    m_pRepresentation = std::make_shared<RequestGoalRep>( this, colour );
}

RequestGoal::RequestGoal( SectorInfo* pSectorInfo, const std::string& description, const Color& colour )
    : m_pSectorInfo( pSectorInfo )
    , m_Description( description )
{
    m_pRepresentation = std::make_shared<RequestGoalRep>( this, colour );
}

} // namespace Hexterminate
