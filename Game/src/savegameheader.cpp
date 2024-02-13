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

#include "savegameheader.h"
#include "xmlaux.h"
#include <xml.h>

namespace Hexterminate
{

SaveGameHeader::SaveGameHeader( const std::filesystem::path& filename )
    : m_Filename( filename )
    , m_PlayedTime( 0.0f )
    , m_Alive( false )
    , m_Difficulty( Difficulty::Normal )
    , m_GameMode( GameMode::Campaign )
    , m_Error( SaveGameHeaderError::Uninitialised )
{
}

bool SaveGameHeader::Read( tinyxml2::XMLDocument& xmlDoc )
{
    using namespace tinyxml2;

    bool captainNameSerialised = false;
    bool shipNameSerialised = false;
    bool playedTimeSerialised = false;
    bool aliveSerialised = false;

    XMLElement* pRootElem = xmlDoc.FirstChildElement();
    if ( pRootElem == nullptr )
    {
        return false;
    }

    for ( XMLElement* pElem = pRootElem->FirstChildElement(); pElem != nullptr; pElem = pElem->NextSiblingElement() )
    {
        aliveSerialised |= Xml::Serialise( pElem, "Alive", m_Alive );
        Xml::Serialise( pElem, "Difficulty", m_Difficulty );
        Xml::Serialise( pElem, "GameMode", m_GameMode );

        if ( std::string( pElem->Value() ) == "Player" )
        {
            for ( XMLElement* pChildElem = pElem->FirstChildElement(); pChildElem != nullptr; pChildElem = pChildElem->NextSiblingElement() )
            {
                captainNameSerialised |= Xml::Serialise( pChildElem, "CaptainName", m_CaptainName );
                shipNameSerialised |= Xml::Serialise( pChildElem, "ShipName", m_ShipName );
                playedTimeSerialised |= Xml::Serialise( pChildElem, "PlayedTime", m_PlayedTime );
            }
        }
    }

    bool allContentSerialised = captainNameSerialised && shipNameSerialised && playedTimeSerialised && aliveSerialised;
    m_Error = allContentSerialised ? SaveGameHeaderError::NoError : SaveGameHeaderError::ContentError;
    return ( m_Error == SaveGameHeaderError::NoError );
}

} // namespace Hexterminate