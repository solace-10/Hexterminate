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

#include <cstdint>
#include <memory>
#include <vector>

#include "sector/starinfo.h"

namespace Hexterminate
{

class HyperscapeLocation;
using HyperscapeLocationVector = std::vector<HyperscapeLocation>;

//-----------------------------------------------------------------------------
// HyperscapeLocation
//-----------------------------------------------------------------------------

class HyperscapeLocation
{
public:
    enum class Type
    {
        Visited,
        Scanned,
        Station
    };

    HyperscapeLocation( const HyperscapeLocation& location );
    HyperscapeLocation( Type type, uint32_t seed, uint32_t depth );
    ~HyperscapeLocation();

    bool operator==( const HyperscapeLocation& location ) const;
    HyperscapeLocation& operator=( const HyperscapeLocation& location );

    uint32_t GetSeed() const;
    uint32_t GetDepth() const;
    Type GetType() const;
    StarInfo* GetStarInfo() const;

private:
    uint32_t m_Seed;
    uint32_t m_Depth;
    Type m_Type;
    std::unique_ptr<StarInfo> m_pStarInfo;
};

inline bool HyperscapeLocation::operator==( const HyperscapeLocation& location ) const
{
    return location.GetDepth() == GetDepth() && location.GetSeed() == GetSeed();
}

inline HyperscapeLocation& HyperscapeLocation::operator=( const HyperscapeLocation& location )
{
    if ( this == &location )
    {
        return *this;
    }
    else
    {
        m_Seed = location.m_Seed;
        m_Depth = location.m_Depth;
        m_Type = location.m_Type;
        m_pStarInfo = std::make_unique<StarInfo>( location.m_Seed );
        return *this;
    }
}

inline uint32_t HyperscapeLocation::GetSeed() const
{
    return m_Seed;
}

inline uint32_t HyperscapeLocation::GetDepth() const
{
    return m_Depth;
}

inline HyperscapeLocation::Type HyperscapeLocation::GetType() const
{
    return m_Type;
}

inline StarInfo* HyperscapeLocation::GetStarInfo() const
{
    return m_pStarInfo.get();
}

} // namespace Hexterminate
