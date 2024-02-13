// Copyright 2018 Pedro Nunes
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

#include "requests/imperialrequest.h"

namespace Hexterminate
{

class SectorInfo;

static const int sArbiterRequestReward = 1500;

class ArbiterRequest : public ImperialRequest
{
public:
    ArbiterRequest( RequestManager* pRequestManager, SectorInfo* pSectorInfo );

    virtual ImperialRequestType GetType() const override { return ImperialRequestType::Arbiter; }

    virtual void Update( float delta ) override;

    virtual void OnBegin() override;
    virtual void OnSuccess() override;
    virtual void OnFailure() override;
    virtual void OnPlayerEnterSector() override;

    virtual int GetConquestReward( const SectorInfo* pSectorInfo ) const override;

    inline SectorInfo* GetSectorInfo() const { return m_pSectorInfo; }

private:
    RequestGoalSharedPtr m_pGoal;
    SectorInfo* m_pSectorInfo;
    int m_Reward;
    Ship* m_pArbiter;
    FleetSharedPtr m_pTemporaryFleet;
};

} // namespace Hexterminate
