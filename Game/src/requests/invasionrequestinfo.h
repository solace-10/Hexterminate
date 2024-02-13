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

#pragma once

#include "requests/imperialrequestinfo.h"

#include <string>

#include "faction/faction.h"

namespace Hexterminate
{

class SectorInfo;

class InvasionRequestInfo : public ImperialRequestInfo
{
public:
    virtual ImperialRequestType GetType() const override { return ImperialRequestType::Invasion; }
    virtual int GetMaximumRequests() const override { return 15; }
    virtual float GetMinimumTimeBetweenRequests() const override { return 15.0f; }
    virtual float GetMinimumTimeBeforeRequest() const override { return 0.0f; }

    virtual ImperialRequestSharedPtr TryInstantiate( RequestManager* pRequestManager ) const override;

    static std::string GetBlackboardTag( FactionId faction );

private:
    SectorInfo* FindSector() const;
    int GetActiveInvasions( RequestManager* pRequestManager, FactionId faction ) const;
};

} // namespace Hexterminate
