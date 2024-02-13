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

#pragma once

#include <string>

#include "beginexternalheaders.h"
#include "tinyxml2.h"
#include "endexternalheaders.h"

#include "color.h"

///////////////////////////////////////////////////////////////////////////////
// Xml namespace
// Set of auxiliary XML functions to help with serialisation of data to avoid
// needless repetition of boilerplate code.
// The namespace is meant to be extended to support additional overloads
///////////////////////////////////////////////////////////////////////////////

namespace Xml
{
bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, std::string& value );
bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, int& value );
bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, float& value );
bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, bool& value );
bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Genesis::Color& value );
}
