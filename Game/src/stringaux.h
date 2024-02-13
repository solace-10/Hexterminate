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
#include <vector>

#include <genesis.h>

#include "faction/faction.h"
#include "hexterminate.h"
#include "ship/moduleinfo.h"

namespace Hexterminate
{
std::string ToLower( const std::string& str );
std::wstring ToWString( const std::string& str );
std::string ToString( const std::wstring& str );
std::string ToString( const std::string& str );
std::string ToString( int value );
std::string ToString( unsigned int value );
std::string ToString( float value );
std::string ToString( WeaponSystem weaponSystem );
std::string ToString( DamageType damageType );
std::string ToString( ModuleRarity rarity );
std::string ToString( AddonCategory );
std::string ToString( FactionId faction );
std::string ToString( Difficulty difficulty );
std::string ToString( GameMode gameMode );
std::string ToString( FactionPresence presence );
const std::string& ToString( Perk perk );
const std::string& ToString( ShipType shipType );
std::string ToStringPercentage( float value ); // Expects a value [0-1]
std::string ToStringTime( float value );
int CountLines( const std::string& text );
bool StringEndsWith( const std::string& text, const std::string& substring );
bool StringEndsWith( const std::wstring& text, const std::wstring& substring );
std::vector<std::string> Split( const std::string& s, char delim );
std::string BreakdownString( const std::string& text, int maximumCharacters );
} // namespace Hexterminate
