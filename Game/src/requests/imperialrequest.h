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

#include <memory>

#include "faction/faction.h"
#include "requests/imperialrequest.fwd.h"
#include "requests/requestgoal.h"

namespace Hexterminate
{

class RequestManager;
class SectorInfo;

class ImperialRequest : public std::enable_shared_from_this<ImperialRequest>
{
public:
    ImperialRequest( RequestManager* pRequestManager );
    virtual ~ImperialRequest() {}

    virtual ImperialRequestType GetType() const = 0;

    virtual void Update( float delta ) {}

    virtual void OnBegin();
    virtual void OnSuccess();
    virtual void OnFailure();
    virtual void OnPlayerEnterSector() {}

    virtual int GetConquestReward( const SectorInfo* pSectorInfo ) const;

    float GetStartTime() const;
    const RequestGoalSharedPtrList& GetGoals() const;
    RequestManager* GetRequestManager() const;
    unsigned int GetThreatScore() const;

    bool GoalExists( const SectorInfo* pSectorInfo ) const;

protected:
    void AddGoal( RequestGoalSharedPtr pGoal );
    void RemoveGoal( RequestGoalSharedPtr pGoal );
    void RemoveGoal( const SectorInfo* pSectorInfo );
    void SetThreatScore( unsigned int value );
    unsigned int GetShipThreatScore( const std::string& ship, FactionId factionId ) const;

private:
    RequestManager* m_pRequestManager;
    float m_StartTime;
    RequestGoalSharedPtrList m_Goals;
    unsigned int m_ThreatScore;
};

inline float ImperialRequest::GetStartTime() const
{
    return m_StartTime;
}

inline const RequestGoalSharedPtrList& ImperialRequest::GetGoals() const
{
    return m_Goals;
}

inline RequestManager* ImperialRequest::GetRequestManager() const
{
    return m_pRequestManager;
}

inline int ImperialRequest::GetConquestReward( const SectorInfo* pSectorInfo ) const
{
    return 0;
}

inline unsigned int ImperialRequest::GetThreatScore() const
{
    return m_ThreatScore;
}

inline void ImperialRequest::SetThreatScore( unsigned int value )
{
    m_ThreatScore = value;
}

} // namespace Hexterminate