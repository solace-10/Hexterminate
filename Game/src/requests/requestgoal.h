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

#pragma once

#include <list>
#include <memory>
#include <string>

#include <color.h>

#include "fleet/fleet.fwd.h"
#include "requests/requestgoalrep.fwd.h"

using Genesis::Color;

namespace Hexterminate
{

class SectorInfo;
class RequestGoal;
typedef std::shared_ptr<RequestGoal> RequestGoalSharedPtr;
typedef std::weak_ptr<RequestGoal> RequestGoalWeakPtr;
typedef std::list<RequestGoalSharedPtr> RequestGoalSharedPtrList;

/////////////////////////////////////////////////////////////////////
// RequestGoal
// Information for a goal that needs to be accomplished by the player
// as part of an Imperial request.
// Contains a representation to use in the Galaxy view.
/////////////////////////////////////////////////////////////////////

class RequestGoal
{
public:
    RequestGoal( FleetWeakPtr pFleet, const std::string& description, const Color& colour = Color( 0.0f, 0.6f, 0.6f, 0.6f ) );
    RequestGoal( SectorInfo* pSectorInfo, const std::string& description, const Color& colour = Color( 0.0f, 0.6f, 0.6f, 0.6f ) );

    FleetWeakPtr GetFleet() const;
    SectorInfo* GetSectorInfo() const;
    const std::string& GetDescription() const;
    RequestGoalRepSharedPtr GetRepresentation() const;

private:
    FleetWeakPtr m_pFleet;
    SectorInfo* m_pSectorInfo;
    std::string m_Description;
    RequestGoalRepSharedPtr m_pRepresentation;
};

inline FleetWeakPtr RequestGoal::GetFleet() const
{
    return m_pFleet;
}

inline SectorInfo* RequestGoal::GetSectorInfo() const
{
    return m_pSectorInfo;
}

inline const std::string& RequestGoal::GetDescription() const
{
    return m_Description;
}

inline RequestGoalRepSharedPtr RequestGoal::GetRepresentation() const
{
    return m_pRepresentation;
}

} // namespace Hexterminate
