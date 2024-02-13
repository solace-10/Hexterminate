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

namespace Hexterminate
{

class ImperialRequest;

typedef std::shared_ptr<ImperialRequest> ImperialRequestSharedPtr;
typedef std::unique_ptr<ImperialRequest> ImperialRequestUniquePtr;
typedef std::weak_ptr<ImperialRequest> ImperialRequestWeakPtr;
typedef std::list<ImperialRequestSharedPtr> ImperialRequestList;
typedef std::list<ImperialRequestWeakPtr> ImperialRequestWeakPtrList;

// Warning: do not change the order of the requests, it is used in serialisation
enum class ImperialRequestType
{
    Invalid,

    Campaign,
    Expand,
    Defend,
    ListeningStation,
    TradeShip,
    Gauntlet,
    Stronghold,
    Invasion,
    Arbiter,
    InfiniteWar,

    Count
};

} // namespace Hexterminate