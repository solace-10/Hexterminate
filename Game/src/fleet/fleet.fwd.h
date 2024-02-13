#pragma once

#include <list>
#include <memory>

namespace Hexterminate
{

class Fleet;

typedef std::shared_ptr<Fleet> FleetSharedPtr;
typedef std::unique_ptr<Fleet> FleetUniquePtr;
typedef std::weak_ptr<Fleet> FleetWeakPtr;
typedef std::list<FleetSharedPtr> FleetList;
typedef std::list<FleetWeakPtr> FleetWeakPtrList;

} // namespace Hexterminate