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

#include "faction/faction.h"

namespace Hexterminate
{

class RequestManager;

///////////////////////////////////////////////////////////////////////////////
// EmpireFaction
// Generates influence over time to give to the player and handles
// giving out ImperialRequests
///////////////////////////////////////////////////////////////////////////////

class EmpireFaction : public Faction
{
public:
    EmpireFaction( const FactionInfo& info );
    virtual ~EmpireFaction();
    virtual void Update( float delta ) override;
    virtual void AddControlledSector( SectorInfo* pSector, bool immediate, bool takenByPlayer ) override;
    RequestManager* GetRequestManager() const;

protected:
    float m_InfluenceTimer;
    RequestManager* m_pRequestManager;
};

inline RequestManager* EmpireFaction::GetRequestManager() const
{
    return m_pRequestManager;
}

} // namespace Hexterminate
