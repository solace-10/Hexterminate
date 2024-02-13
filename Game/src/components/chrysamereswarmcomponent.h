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

#include "component.h"
#include "fleet/fleet.fwd.h"

namespace Hexterminate
{

/////////////////////////////////////////////////////////////////////
// Keeps spawning drone ships in a given sector, as long as the
// Final Chrysamere has not been destroyed.
// The waves get progressively larger as time goes by.
/////////////////////////////////////////////////////////////////////

class ChrysamereSwarmComponent : public Genesis::Component
{
    DECLARE_COMPONENT( ChrysamereSwarmComponent )
public:
    ChrysamereSwarmComponent();
    virtual bool Initialise() override;
    virtual void Update( float delta ) override;

private:
    unsigned int m_Wave;
    float m_Timer;
    FleetList m_TemporaryFleets;
};

} // namespace Hexterminate