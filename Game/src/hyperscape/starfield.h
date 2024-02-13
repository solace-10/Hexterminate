// Copyright 2021 Pedro Nunes
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
#include <memory>

#include "hyperscape/starfieldentry.h"

namespace Hexterminate
{

class StarfieldRep;
using StarfieldEntryArray = std::array<StarfieldEntry, 128>;

//-----------------------------------------------------------------------------
// Starfield
//-----------------------------------------------------------------------------

class Starfield
{
public:
    Starfield();
    ~Starfield();

    void Update( float delta );
    void Show( bool state );
    StarfieldRep* GetRepresentation() const;
    bool IsVisible() const;

    const StarfieldEntryArray& GetEntries() const;

private:
    StarfieldEntry CreateEntry( bool randomStart );

    std::unique_ptr<StarfieldRep> m_pRep;
    StarfieldEntryArray m_Entries;
};

inline StarfieldRep* Starfield::GetRepresentation() const
{
    return m_pRep.get();
}

inline const StarfieldEntryArray& Starfield::GetEntries() const
{
    return m_Entries;
}

} // namespace Hexterminate
