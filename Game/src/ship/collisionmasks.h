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

namespace Tannhauser
{

// Naiive way of doing this
static const int CM_FACTION_1_SHIP = 1 << 0;
static const int CM_FACTION_1_AMMO = 1 << 1;
static const int CM_FACTION_2_SHIP = 1 << 2;
static const int CM_FACTION_2_AMMO = 1 << 3;
static const int CM_FACTION_3_SHIP = 1 << 4;
static const int CM_FACTION_3_AMMO = 1 << 5;
static const int CM_FACTION_4_SHIP = 1 << 6;
static const int CM_FACTION_5_AMMO = 1 << 7;

enum CollisionDataType
{
    DATA_TYPE_nullptr = 0,
    DATA_TYPE_SHIP,
    DATA_TYPE_AMMO
};

struct CollisionData
{
    CollisionDataType type;
    void* pOwner;
};

} // namespace Tannhauser