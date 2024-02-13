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

#include <genesis.h>
#include <logger.h>

#include "hexterminate.h"
#include "ship/hexgrid.h"
#include "ship/moduleinfo.h"
#include "xmlaux.h"

namespace Hexterminate
{

bool WriteHexGridModuleInfo( HexGrid<ModuleInfo*>* pHexGrid, tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement )
{
    using namespace tinyxml2;

    XMLElement* pElement = xmlDoc.NewElement( "HexGrid" );
    pRootElement->LinkEndChild( pElement );

    Xml::Write( xmlDoc, pElement, "Version", pHexGrid->GetVersion() );

    int x1, y1, x2, y2;
    pHexGrid->GetBoundingBox( x1, y1, x2, y2 );

    for ( int x = x1; x <= x2; ++x )
    {
        for ( int y = y1; y <= y2; ++y )
        {
            ModuleInfo* pModuleInfo = pHexGrid->Get( x, y );
            if ( pModuleInfo != nullptr )
            {
                XMLElement* pModuleElement = xmlDoc.NewElement( "Module" );
                pElement->LinkEndChild( pModuleElement );

                pModuleElement->SetAttribute( "x", x );
                pModuleElement->SetAttribute( "y", y );
                pModuleElement->SetText( pModuleInfo->GetName().c_str() );
            }
        }
    }

    return true;
}

bool ReadHexGridModuleInfo( HexGrid<ModuleInfo*>* pHexGrid, tinyxml2::XMLElement* pRootElement )
{
    Genesis::FrameWork::GetLogger()->LogInfo( "Loading hexgrid..." );

    for ( tinyxml2::XMLElement* pElement = pRootElement->FirstChildElement(); pElement != nullptr; pElement = pElement->NextSiblingElement() )
    {
        if ( std::string( pElement->Value() ) == "Module" )
        {
            std::string moduleName = pElement->GetText();
            ModuleInfo* pModuleInfo = g_pGame->GetModuleInfoManager()->GetModuleByName( moduleName );

            if ( pModuleInfo == nullptr )
            {
                Genesis::FrameWork::GetLogger()->LogWarning( "Unable to find module '%s', skipping.", moduleName.c_str() );
                return false;
            }
            else
            {
                int x, y;
                pElement->QueryIntAttribute( "x", &x );
                pElement->QueryIntAttribute( "y", &y );

                pHexGrid->Set( x, y, pModuleInfo );
            }
        }
    }

    return true;
}

#ifndef _MSC_VER

template <>
int HexGrid<ModuleInfo*>::GetVersion() const
{
    return 1;
}

template <>
bool HexGrid<ModuleInfo*>::Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement )
{
    return WriteHexGridModuleInfo( this, xmlDoc, pRootElement );
}

template <>
bool HexGrid<ModuleInfo*>::Read( tinyxml2::XMLElement* pRootElement )
{
    return ReadHexGridModuleInfo( this, pRootElement );
}

#endif

} // namespace Hexterminate