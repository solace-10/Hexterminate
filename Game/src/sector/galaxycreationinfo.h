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

#include "faction/faction.h"

namespace Hexterminate
{

class GalaxyCreationInfo
{
public:
    enum class CreationMode
    {
        Empty, // Used when loading a previously existing save game. The galaxy will then be overwritten with the save data.
        Campaign, // Campaign mode, FactionPresence is not used.
        InfiniteWar // Procedurally generated galaxy, uses FactionPresence to populate sectors.
    };

    enum class GenerationType
    {
        Expansion,
        Scattered
    };

    GalaxyCreationInfo( CreationMode creationMode );
    CreationMode GetMode() const;
    void SetFaction( FactionId factionId, FactionPresence presence, GenerationType generationType, bool hasHomeworld );
    FactionPresence GetFactionPresence( FactionId factionId ) const;
    void SetFactionPresence( FactionId factionId, FactionPresence presence );
    GenerationType GetGenerationType( FactionId factionId ) const;
    bool HasHomeworld( FactionId factionId ) const;

private:
    CreationMode m_CreationMode;

    struct Data
    {
        FactionPresence presence;
        GenerationType generationType;
        bool hasHomeworld;
    };

    std::array<Data, static_cast<size_t>( FactionId::Count )> m_Data;
};

} // namespace Hexterminate
