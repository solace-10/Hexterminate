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

///////////////////////////////////////////////////////////////////////////////
// NeutralFaction
// The neutral faction processes its sectors, but doesn't attempt to build
// any fleets.
// Note that it doesn't call Faction::ProcessTurn.
///////////////////////////////////////////////////////////////////////////////

class NeutralFaction : public Faction
{
public:
    NeutralFaction( const FactionInfo& info );
    virtual ~NeutralFaction(){};

protected:
    virtual void ProcessTurn();
};

} // namespace Hexterminate
