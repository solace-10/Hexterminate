// Copyright 2014 Pedro Nunes
//
// This file is part of Genesis.
//
// Genesis is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Genesis is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Genesis. If not, see <http://www.gnu.org/licenses/>.

#include <string>

#include <SDL.h>

#include "math/misc.h"
#include "xml.h"

namespace Xml
{

///////////////////////////////////////////////////////////////////////////////
// Xml
///////////////////////////////////////////////////////////////////////////////

bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, std::string& value )
{
    SDL_assert( pElement != nullptr );

    if ( name != pElement->Value() )
        return false;

    if ( pElement->GetText() == nullptr )
        return false;

    value = pElement->GetText();
    return true;
}

bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, int& value )
{
    SDL_assert( pElement != nullptr );

    if ( name != pElement->Value() )
        return false;

    if ( pElement->GetText() == nullptr )
        return false;

    value = atoi( pElement->GetText() );
    return true;
}

bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, float& value )
{
    SDL_assert( pElement != nullptr );

    if ( name != pElement->Value() )
        return false;

    if ( pElement->GetText() == nullptr )
        return false;

    value = (float)atof( pElement->GetText() );
    return true;
}

bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, bool& value )
{
    SDL_assert( pElement != nullptr );

    if ( name != pElement->Value() )
        return false;

    if ( pElement->GetText() == nullptr )
        return false;

    std::string text = pElement->GetText();
    if ( text == "True" || text == "true" )
    {
        value = true;
    }
    else if ( text == "False" || text == "false" )
    {
        value = false;
    }
    else
    {
        // Invalid value in XML file, defaulting to false.
        SDL_assert( false );
        value = false;
    }

    return true;
}

bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Genesis::Color& value )
{
    SDL_assert( pElement != nullptr );

    if ( name != pElement->Value() )
        return false;

    value.r = gClamp<float>( (float)atof( pElement->Attribute( "r" ) ), 0.0f, 1.0f );
    value.g = gClamp<float>( (float)atof( pElement->Attribute( "g" ) ), 0.0f, 1.0f );
    value.b = gClamp<float>( (float)atof( pElement->Attribute( "b" ) ), 0.0f, 1.0f );
    value.a = gClamp<float>( (float)atof( pElement->Attribute( "a" ) ), 0.0f, 1.0f );

    return true;
}
}