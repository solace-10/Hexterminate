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

namespace Hexterminate
{

class SectorInfo;

class StrongholdRequestInfo : public ImperialRequestInfo
{
public:
    virtual ImperialRequestType GetType() const override { return ImperialRequestType::Stronghold; }
    virtual int GetMaximumRequests() const override { return 3; }
    virtual float GetMinimumTimeBetweenRequests() const override { return 120.0f; }
    virtual float GetMinimumTimeBeforeRequest() const override { return 120.0f; }

    virtual ImperialRequestSharedPtr TryInstantiate( RequestManager* pRequestManager ) const override;

private:
    SectorInfo* FindSector() const;
};

} // namespace Hexterminate
