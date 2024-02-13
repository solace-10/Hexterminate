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

#include <array>

#include "globals.h"

namespace Hexterminate
{

class SectorInfo;

class FogOfWar
{
public:
    FogOfWar();
    void Update( float delta );
    void MarkAsVisible( const SectorInfo* pSectorInfo, int radius = 0 );
    bool IsVisible( const SectorInfo* pSectorInfo ) const;

private:
    void MarkAsVisibleSingle( int x, int y );
    std::array<std::array<float, NumSectorsY>, NumSectorsX> m_Tiles;
};

} // namespace Hexterminate