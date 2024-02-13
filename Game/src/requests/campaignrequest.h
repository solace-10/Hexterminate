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

#include <string>

#include "faction/faction.h"
#include "requests/imperialrequest.h"

namespace Hexterminate
{

class Faction;

class CampaignRequest : public ImperialRequest
{
public:
    CampaignRequest( RequestManager* pRequestManager );

    virtual ImperialRequestType GetType() const override { return ImperialRequestType::Campaign; }

    virtual void OnBegin() override;
    virtual void OnSuccess() override;
    virtual void OnPlayerEnterSector() override;

    virtual void Update( float delta ) override;

private:
    void UpdateExpansionArc( float delta );
    void UpdatePirateArc( float delta );
    void UpdateMarauderArc( float delta );
    void UpdateAscentArc( float delta );
    void UpdateIrianiArc( float delta );
    void UpdateChrysamereArc( float delta );
    void ClearGoals( RequestGoalSharedPtrList& list );
    void SpawnShip( const std::string& ship, FactionId factionId, float x, float y );

    float m_UpdateInterval;
    RequestGoalSharedPtr m_pExpansionGoal;
    RequestGoalSharedPtrList m_PirateSectorGoals;
    RequestGoalSharedPtr m_pFleetGoal;
};

} // namespace Hexterminate
