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

#include <array>

#include "requests/imperialrequest.fwd.h"
#include "requests/imperialrequestinfo.h"

namespace Hexterminate
{

class RequestManager
{
public:
    RequestManager();
    ~RequestManager();

    void Update( float delta );
    void Remove( ImperialRequestWeakPtr pRequest );
    int GetActiveCount( ImperialRequestType type ) const;
    float GetPreviousStartTime( ImperialRequestType type ) const;
    const ImperialRequestList& GetRequests() const;

private:
    void ClearRemovedRequests();

    ImperialRequestList m_Requests;
    ImperialRequestList m_RequestsToRemove;
    ImperialRequestInfoList m_Infos;

    std::array<int, (int)ImperialRequestType::Count> m_Active;
    std::array<float, (int)ImperialRequestType::Count> m_StartTime;
    float m_InstancingTimer;
};

inline const ImperialRequestList& RequestManager::GetRequests() const
{
    return m_Requests;
}

} // namespace Hexterminate
