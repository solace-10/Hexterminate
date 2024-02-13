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

#pragma once

#include <string>

// clang-format off
#include <beginexternalheaders.h>
#include <tinyxml2.h>
#include <endexternalheaders.h>
// clang-format on

#include "fleet/fleet.h"
#include "hexterminate.h"
#include "ship/moduleinfo.h"

namespace Xml
{

bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Hexterminate::ModuleRarity& value );
bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Hexterminate::WeaponBehaviour& value );
bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Hexterminate::TowerBonus& value );
bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Hexterminate::FleetState& value );
bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Hexterminate::Difficulty& value );
bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Hexterminate::GameMode& value );
bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Hexterminate::ReactorVariant& value );

void Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement, const std::string& elementName, const std::string& content );
void Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement, const std::string& elementName, int content );
void Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement, const std::string& elementName, float content );
void Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement, const std::string& elementName, bool content );
void Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement, const std::string& elementName, Hexterminate::FleetState content );

} // namespace Xml
