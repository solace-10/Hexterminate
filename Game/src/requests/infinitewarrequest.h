// Copyright 2020 Pedro Nunes
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

class InfiniteWarRequest : public ImperialRequest
{
public:
    InfiniteWarRequest( RequestManager* pRequestManager );

    virtual ImperialRequestType GetType() const override { return ImperialRequestType::InfiniteWar; }

    virtual void OnBegin() override;
    virtual void OnSuccess() override;

    virtual void Update( float delta ) override;

private:
    void UpdateHomeworldsArc( float delta );
    void AddHomeworldGoal( FactionId factionId );

    float m_UpdateInterval;
};

} // namespace Hexterminate
