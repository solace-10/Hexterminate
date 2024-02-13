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

#include "requests/imperialrequest.fwd.h"

namespace Hexterminate
{

class ImperialRequestInfo;
class RequestManager;
class SectorInfo;

typedef std::list<ImperialRequestInfo*> ImperialRequestInfoList;

/////////////////////////////////////////////////////////////////////
// ImperialRequestInfo
// Contains the basic information that is used by the RequestManager
// to evaluate whether a new ImperialRequest of a particular type can
// be instantiated or not.
// This allows each request to be self-contained with regards to the
// logic it runs and the RequestManager to only have generic logic.
/////////////////////////////////////////////////////////////////////

class ImperialRequestInfo
{
public:
    enum class AllowedGameMode
    {
        Campaign,
        InfiniteWar,
        Any
    };

    ImperialRequestInfo() {}
    virtual ~ImperialRequestInfo() {}

    virtual ImperialRequestType GetType() const { return ImperialRequestType::Invalid; }

    virtual AllowedGameMode GetAllowedGameMode() const { return AllowedGameMode::Any; }

    // How many requests of this type can be active in-game at the
    // same time.
    virtual int GetMaximumRequests() const { return 1; }

    // In seconds
    virtual float GetMinimumTimeBetweenRequests() const { return 60.0f; }

    // In seconds, gameplay time before this request can trigger
    virtual float GetMinimumTimeBeforeRequest() const { return 0.0f; }

    // TryInstantiate() should return an empty shared ptr if the
    // conditions needed to create a new request aren't met.
    virtual ImperialRequestSharedPtr TryInstantiate( RequestManager* pRequestManager ) const { return ImperialRequestSharedPtr(); }

protected:
    // Always call in a child class' TryInstantiate()
    bool CommonInstantiationChecks( RequestManager* pRequestManager ) const;

    // Checks if there is another request which is already using this sector
    bool IsDuplicateSector( RequestManager* pRequestManager, SectorInfo* pSectorInfo ) const;
};

} // namespace Hexterminate