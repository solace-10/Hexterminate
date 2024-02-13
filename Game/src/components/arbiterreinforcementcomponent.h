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

#include "fleet/fleet.fwd.h"
#include <component.h>

namespace Hexterminate
{

class Ship;

/////////////////////////////////////////////////////////////////////
// Component for spawning up to 4 arbiters, depending on how many
// have been previously destroyed.
/////////////////////////////////////////////////////////////////////

class ArbiterReinforcementComponent : public Genesis::Component
{
    DECLARE_COMPONENT( ArbiterReinforcementComponent )
public:
    virtual bool Initialise() override;

private:
    void SpawnArbiters();
    FleetSharedPtr m_pTemporaryFleet;
};

} // namespace Hexterminate