// Copyright 2022 Pedro Nunes
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

#include "stringaux.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace Hexterminate
{

std::string ToLower( const std::string& str )
{
    std::string loweredStr( str );
    std::transform( loweredStr.begin(), loweredStr.end(), loweredStr.begin(), []( char c ) -> char { return static_cast<char>( std::tolower( c ) ); } );
    return loweredStr;
}

///*
// Transform any std string or string view
// into any of the 4 std string types,
//(c) 2018-2021 by dbj at dbj dot org
// https://dbj.org/license_dbj
//*/
// template<typename T, typename F>
// inline T
//	transform_to(F str) noexcept
//{
//	// note: F has to have
//	// the empty() method
//	if (str.empty())
//		return {};
//	// note: F must be able to work
//	// with std begin and end
//	return { std::begin(str), std::end(str) };
//	// also the above line requires, T has a constructor
//	// that will take begin and end values of type F.
// };

std::wstring ToWString( const std::string& str )
{
    std::wstring result;
    std::copy( str.begin(), str.end(), std::back_inserter( result ) );
    return result;
}

std::string ToString( const std::wstring& str )
{
#if _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4244 ) // warning C4244: 'argument': conversion from 'wchar_t' to 'const _Elem', possible loss of data
#endif

    return std::string( str.begin(), str.end() );

#if _MSC_VER
#pragma warning( pop )
#endif
}

std::string ToString( const std::string& str )
{
    return str;
}

std::string ToString( int value )
{
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

std::string ToString( unsigned int value )
{
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

std::string ToString( float value )
{
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

std::string ToString( DamageType damageType )
{
    if ( damageType == DamageType::Collision )
    {
        return "Collision";
    }
    else if ( damageType == DamageType::EMP )
    {
        return "EMP";
    }
    else if ( damageType == DamageType::Energy )
    {
        return "Energy";
    }
    else if ( damageType == DamageType::Kinetic )
    {
        return "Kinetic";
    }
    else if ( damageType == DamageType::TrueDamage )
    {
        return "True damage";
    }
    else
    {
        SDL_assert( false );
        return "Unknown";
    }
}

std::string ToString( WeaponSystem ws )
{
    if ( ws == WeaponSystem::Projectile )
        return "Projectile";
    else if ( ws == WeaponSystem::Missile )
        return "Missile";
    else if ( ws == WeaponSystem::Rocket )
        return "Rocket";
    else if ( ws == WeaponSystem::Torpedo )
        return "Torpedo";
    else if ( ws == WeaponSystem::Lance )
        return "Lance";
    else if ( ws == WeaponSystem::Ion )
        return "Ion";
    else if ( ws == WeaponSystem::Antiproton )
        return "Antiproton";
    else if ( ws == WeaponSystem::Universal )
        return "Universal";
    else
        return "";
}

std::string ToString( ModuleRarity rarity )
{
    if ( rarity == ModuleRarity::Trash )
        return "Trash";
    else if ( rarity == ModuleRarity::Common )
        return "Common";
    else if ( rarity == ModuleRarity::Uncommon )
        return "Uncommon";
    else if ( rarity == ModuleRarity::Rare )
        return "Rare";
    else if ( rarity == ModuleRarity::Artifact )
        return "Artifact";
    else if ( rarity == ModuleRarity::Legendary )
        return "Legendary";
    else
        return "";
}

std::string ToString( AddonCategory category )
{
    if ( category == AddonCategory::DroneBay )
        return "Drone bay";
    else if ( category == AddonCategory::FuelInjector )
        return "Fuel injector";
    else if ( category == AddonCategory::HangarBay )
        return "Hangar bay";
    else if ( category == AddonCategory::MissileInterceptor )
        return "Missile interceptor";
    else if ( category == AddonCategory::ModuleRepairer )
        return "Module repairer";
    else if ( category == AddonCategory::PhaseBarrier )
        return "Phase barrier";
    else if ( category == AddonCategory::EngineDisruptor )
        return "Engine disruptor";
    else if ( category == AddonCategory::ParticleAccelerator )
        return "Railgun";
    else if ( category == AddonCategory::QuantumStateAlternator )
        return "Quantum alternator";
    else
        return "";
}

std::string ToString( FactionId factionId )
{
    if ( factionId == FactionId::Ascent )
        return "Ascent";
    else if ( factionId == FactionId::Empire )
        return "Empire";
    else if ( factionId == FactionId::Iriani )
        return "Iriani";
    else if ( factionId == FactionId::Marauders )
        return "Marauders";
    else if ( factionId == FactionId::Neutral )
        return "Neutral";
    else if ( factionId == FactionId::Pirate )
        return "Pirates";
    else if ( factionId == FactionId::Player )
        return "Player";
    else if ( factionId == FactionId::Special )
        return "Special";
    else if ( factionId == FactionId::Hegemon )
        return "Hegemon";
    else
        return "<unknown faction>";
}

std::string ToString( Difficulty difficulty )
{
    if ( difficulty == Difficulty::Easy )
        return "Easy";
    else if ( difficulty == Difficulty::Normal )
        return "Normal";
    else if ( difficulty == Difficulty::Hardcore )
        return "Hardcore";
    else
        return "Not set";
}

std::string ToString( GameMode gameMode )
{
    if ( gameMode == GameMode::Campaign )
        return "Campaign";
    else if ( gameMode == GameMode::InfiniteWar )
        return "Infinite War";
    else
        return "Not set";
}

std::string ToString( FactionPresence presence )
{
    if ( presence == FactionPresence::None )
    {
        return "None";
    }
    else if ( presence == FactionPresence::Light )
    {
        return "Light";
    }
    else if ( presence == FactionPresence::Standard )
    {
        return "Standard";
    }
    else if ( presence == FactionPresence::Heavy )
    {
        return "Heavy";
    }
    else
    {
        return "Not set";
    }
}

const std::string& ToString( Perk perk )
{
    static const std::array<std::string, static_cast<size_t>( Perk::Count )> perkNames = {
        "Engine remapping",
        "Evasion protocols",
        "Thrust vectoring",
        "Ramming speed",
        "Gunship construction",
        "Battlecruiser construction",
        "Battleship construction",
        "Dreadnaught construction",
        "Support request",
        "Primary fleet",
        "Priority requisition",
        "Prototype access",
        "Magnetic loaders",
        "Advanced electrocoils",
        "Plasma warheads",
        "Siegebreaker",
        "Advanced heatsinks",
        "Phase synchronisation",
        "Disruption",
        "Overload",
        "Frequency cycling",
        "Kinetic hardening",
        "Superconductors",
        "Emergency capacitors",
        "Reclaimed sectors",
        "Shared glory",
        "Sword of the Empire",
        "Unity is Strength",
        "Lighter materials",
        "Nanobots",
        "Reinforced bulkheads",
        "Unbroken",
    };

    return perkNames[ static_cast<size_t>( perk ) ];
}

const std::string& ToString( ShipType shipType )
{
    static const std::array<std::string, static_cast<size_t>( ShipType::Count )> shipTypeNames = {
        "Invalid",
        "Gunship",
        "Battlecruiser",
        "Capital",
    };

    return shipTypeNames[ static_cast<size_t>( shipType ) ];
}

std::string ToStringPercentage( float value )
{
    SDL_assert( value >= 0.0f && value <= 1.0f );
    int percentage = static_cast<int>( value * 100.0f );
    return ToString( percentage ) + "%";
}

std::string ToStringTime( float value )
{
    SDL_assert( value >= 0.0f );
    int t = (int)value;
    int s = t % 60;
    int m = ( t / 60 ) % 60;
    int h = ( t / 3600 ) % 60;
    std::ostringstream ss;
    if ( h > 0 )
        ss << h << ":";
    if ( m < 10 )
        ss << "0" << m << ":";
    else
        ss << m << ":";
    if ( s < 10 )
        ss << "0" << s;
    else
        ss << s;
    return ss.str();
}

int CountLines( const std::string& text )
{
    if ( text.empty() )
        return 0;

    int lines = 1;
    size_t l = text.length();
    for ( size_t i = 0; i < l; ++i )
    {
        if ( text[ i ] == '\n' )
            lines++;
    }
    return lines;
}

bool StringEndsWith( const std::string& text, const std::string& substring )
{
    const size_t textLength = text.length();
    const size_t substringLength = substring.length();
    return ( textLength >= substringLength && text.substr( textLength - substringLength ) == substring );
}

bool StringEndsWith( const std::wstring& text, const std::wstring& substring )
{
    const size_t textLength = text.length();
    const size_t substringLength = substring.length();
    return ( textLength >= substringLength && text.substr( textLength - substringLength ) == substring );
}

std::vector<std::string>& Split( const std::string& s, char delim, std::vector<std::string>& elems )
{
    std::stringstream ss( s );
    std::string item;
    while ( std::getline( ss, item, delim ) )
    {
        elems.push_back( item );
    }
    return elems;
}

std::vector<std::string> Split( const std::string& s, char delim )
{
    std::vector<std::string> elems;
    Split( s, delim, elems );
    return elems;
}

std::string BreakdownString( const std::string& text, int maximumCharacters )
{
    std::stringstream ss;
    std::vector<std::string> words = Split( text, ' ' );
    size_t wordCount = words.size();
    size_t lineLength = 0;
    for ( size_t i = 0; i < wordCount; ++i )
    {
        const size_t wordLength = words[ i ].length();
        if ( lineLength + wordLength > maximumCharacters )
        {
            ss << "\n"
               << words[ i ] << " ";
            lineLength = wordLength + 1;
        }
        else
        {
            ss << words[ i ] << " ";
            lineLength += wordLength + 1;
        }
    }

    return ss.str();
}

} // namespace Hexterminate