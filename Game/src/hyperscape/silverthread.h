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

#include <vector>

#include <SDL.h>

#include "hyperscape/hyperscapelocation.h"

namespace Hexterminate
{

class SilverThreadRep;

//-----------------------------------------------------------------------------
// SilverThread
//-----------------------------------------------------------------------------

class SilverThread
{
public:
    SilverThread();
    ~SilverThread();

    const HyperscapeLocation& GetCurrentLocation() const;
    const HyperscapeLocationVector& GetLocations() const;

    void Add( const HyperscapeLocation& location );
    void Scan( size_t count );
    size_t GetScannedLocationCount() const;
    void JumpToScannedLocation( size_t index );
    void JumpToScannedLocation( const HyperscapeLocation& location );
    void CallStation();

private:
    void RebuildLocation( size_t index, HyperscapeLocation::Type newType );

    HyperscapeLocationVector m_Locations;
    size_t m_CurrentLocationIndex;
    size_t m_Scanned;
    std::unique_ptr<SilverThreadRep> m_pRep;
};

inline const HyperscapeLocation& SilverThread::GetCurrentLocation() const
{
    SDL_assert( m_Locations.empty() == false );
    return m_Locations[ m_CurrentLocationIndex ];
}

inline const HyperscapeLocationVector& SilverThread::GetLocations() const
{
    return m_Locations;
}

inline size_t SilverThread::GetScannedLocationCount() const
{
    return m_Scanned;
}

} // namespace Hexterminate
