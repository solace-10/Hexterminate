// Copyright 2017 Pedro Nunes
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
#include <array>
#include <imgui/imgui_impl.h>
#include <list>

namespace Hexterminate
{

class Ship;

/////////////////////////////////////////////////////////////////////
// DamageTracker
// Accumulates all the damage this ship has been dealt by different
// factions.
/////////////////////////////////////////////////////////////////////

class DamageTracker
{
public:
    DamageTracker( Ship* pShip );
    ~DamageTracker();

    Ship* GetShip() const;
    float GetDamage( FactionId id ) const;
    void AddDamage( FactionId id, float amount );
    void GetHighestContributor( FactionId& id, float& amount ) const;
    int GetID() const;

private:
    Ship* m_pShip;
    std::array<float, static_cast<size_t>( FactionId::Count )> m_Damage;
    int m_ID;
    static int sID;
};

/////////////////////////////////////////////////////////////////////
// DamageTrackerDebugWindow
// Monostate which provides information about all DamageTrackers.
/////////////////////////////////////////////////////////////////////

class DamageTrackerDebugWindow
{
public:
    static void Register();
    static void Unregister();
    static void Update();
    static void AddDamageTracker( DamageTracker* pTracker );
    static void RemoveDamageTracker( DamageTracker* pTracker );

private:
    static bool m_Open;
    static std::list<DamageTracker*> m_Trackers;
};

} // namespace Hexterminate