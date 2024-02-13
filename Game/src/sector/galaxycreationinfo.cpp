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

#include "galaxycreationinfo.h"

namespace Hexterminate
{

GalaxyCreationInfo::GalaxyCreationInfo( CreationMode creationMode )
{
    m_CreationMode = creationMode;

    for ( int i = 0; i < static_cast<int>( FactionId::Count ); ++i )
    {
        SetFaction( static_cast<FactionId>( i ), FactionPresence::Standard, GenerationType::Expansion, true );
    }
    SetFaction( FactionId::Neutral, FactionPresence::None, GenerationType::Expansion, false );
    SetFaction( FactionId::Player, FactionPresence::None, GenerationType::Expansion, false );
    SetFaction( FactionId::Special, FactionPresence::None, GenerationType::Expansion, false );
    SetFaction( FactionId::Pirate, FactionPresence::Standard, GenerationType::Scattered, false );
}

GalaxyCreationInfo::CreationMode GalaxyCreationInfo::GetMode() const
{
    return m_CreationMode;
}

void GalaxyCreationInfo::SetFaction( FactionId factionId, FactionPresence presence, GenerationType generationType, bool hasHomeworld )
{
    if ( presence != FactionPresence::None && generationType == GenerationType::Expansion && hasHomeworld == false )
    {
        Genesis::FrameWork::GetLogger()->LogError( "A faction can only use the Expansion generation type if it has a homeworld." );
    }

    Data& data = m_Data[ static_cast<size_t>( factionId ) ];
    data.presence = presence;
    data.generationType = generationType;
    data.hasHomeworld = hasHomeworld;
}

FactionPresence GalaxyCreationInfo::GetFactionPresence( FactionId factionId ) const
{
    return m_Data[ static_cast<size_t>( factionId ) ].presence;
}

void GalaxyCreationInfo::SetFactionPresence( FactionId factionId, FactionPresence presence )
{
    m_Data[ static_cast<size_t>( factionId ) ].presence = presence;
}

GalaxyCreationInfo::GenerationType GalaxyCreationInfo::GetGenerationType( FactionId factionId ) const
{
    return m_Data[ static_cast<size_t>( factionId ) ].generationType;
}

bool GalaxyCreationInfo::HasHomeworld( FactionId factionId ) const
{
    return m_Data[ static_cast<size_t>( factionId ) ].hasHomeworld;
}

} // namespace Hexterminate
