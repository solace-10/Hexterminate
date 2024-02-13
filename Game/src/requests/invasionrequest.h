// Copyright 2016 Pedro Nunes
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

#include "fleet/fleet.h"
#include "requests/imperialrequest.h"

namespace Hexterminate
{

class SectorInfo;

static const int sInvasionRequestReward = 1000;
static const float sInvasionRequestTimer = 5.0f * 60.0f; // Time until this sector's invasion succeeds and it is lost

class InvasionRequest : public ImperialRequest
{
public:
    InvasionRequest( RequestManager* pRequestManager, SectorInfo* pSectorInfo, FactionId invadingFaction );

    virtual ImperialRequestType GetType() const override { return ImperialRequestType::Invasion; }

    virtual void Update( float delta ) override;

    virtual void OnBegin() override;
    virtual void OnSuccess() override;
    virtual void OnFailure() override;
    virtual void OnPlayerEnterSector() override;

    virtual int GetConquestReward( const SectorInfo* pSectorInfo ) const override;

    inline SectorInfo* GetSectorInfo() const { return m_pSectorInfo; }
    inline FactionId GetInvadingFaction() const { return m_InvadingFaction; }

private:
    ShipInfoList GetShipsToSpawn() const;

    RequestGoalSharedPtr m_pGoal;
    SectorInfo* m_pSectorInfo;
    int m_Reward;
    float m_FailureTimer;
    FactionId m_InvadingFaction;
};

} // namespace Hexterminate
