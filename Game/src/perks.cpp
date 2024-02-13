// Copyright 2014 Pedro Nunes
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

#include <xml.h>

#include "perks.h"
#include "xmlaux.h"

namespace Hexterminate
{

Perks::Perks()
{
    Reset();
}

bool Perks::Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement )
{
    using namespace tinyxml2;
    XMLElement* pPerksElement = xmlDoc.NewElement( "Perks" );
    pRootElement->LinkEndChild( pPerksElement );

    Xml::Write( xmlDoc, pPerksElement, "Version", GetVersion() );
    Xml::Write( xmlDoc, pPerksElement, "Bitset", m_Bitset.to_string() );

    return true;
}

bool Perks::Read( tinyxml2::XMLElement* pRootElement )
{
    int version = 0;
    std::string bitset;

    for ( tinyxml2::XMLElement* pChildElement = pRootElement->FirstChildElement(); pChildElement != nullptr; pChildElement = pChildElement->NextSiblingElement() )
    {
        Xml::Serialise( pChildElement, "Version", version );
        Xml::Serialise( pChildElement, "Bitset", bitset );
    }

    SDL_assert_release( version == GetVersion() );
    SDL_assert_release( bitset.length() == sMaxPerks );
    SDL_assert_release( (unsigned int)Perk::Count <= sMaxPerks );

    m_Bitset = std::bitset<sMaxPerks>( bitset );

    return true;
}

} // namespace Hexterminate
