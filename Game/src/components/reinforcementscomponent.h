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
#include <vector>

namespace Hexterminate
{

class Faction;
class ShipInfo;
using ShipInfoVector = std::vector<const ShipInfo*>;

/////////////////////////////////////////////////////////////////////
// Any sector with this component will receive reinforcements based
// on how many friendly sectors there are around it.
// This is different from the concept of "waves" as the behaviour
// is predetermined, and the list of reinforcement ships is set
// from the data/xml/reinforcements folder.
/////////////////////////////////////////////////////////////////////

class ReinforcementsComponent : public Genesis::Component
{
    DECLARE_COMPONENT( ReinforcementsComponent )
public:
    virtual bool Initialise() override;
    virtual void Update( float delta ) override;

private:
    void LoadReinforcements();
    ShipInfoVector LoadShipListFile( Faction* pFaction, const std::string& filename ) const;
    void SpawnReinforcements( const ShipInfoVector& shipInfos );

    // Time between reinforcements.
    float m_Timer;

    // Which set of ships from m_Reinforcements to be used to reinforce the sector.
    int m_CurrentReinforcements;

    // Used to create temporary fleets as reinforcements.
    using ReinforcementsVector = std::vector<ShipInfoVector>;
    ReinforcementsVector m_Reinforcements;
    Faction* m_pReinforcementsFaction;

    FleetList m_TemporaryFleets;
};

} // namespace Hexterminate