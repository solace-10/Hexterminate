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

#include "xmlaux.h"

#include <array>
#include <sstream>
#include <utility>

namespace Xml
{

bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Hexterminate::ModuleRarity& value )
{
    SDL_assert( pElement != nullptr );

    if ( pElement == nullptr || name != pElement->Value() )
    {
        return false;
    }

    using namespace Hexterminate;

    static const std::string rarity[ static_cast<size_t>( ModuleRarity::Count ) ] = {
        "Trash",
        "Common",
        "Uncommon",
        "Rare",
        "Artifact",
        "Legendary"
    };

    for ( int i = 0; i < static_cast<int>( ModuleRarity::Count ); ++i )
    {
        if ( rarity[ i ] == pElement->GetText() )
        {
            value = static_cast<ModuleRarity>( i );
            return true;
        }
    }

    return false;
}

bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Hexterminate::WeaponBehaviour& value )
{
    SDL_assert( pElement != nullptr );
    if ( pElement == nullptr || name != pElement->Value() )
    {
        return false;
    }

    using namespace Hexterminate;
    const std::string text( pElement->GetText() );
    static const std::array<std::pair<std::string, WeaponBehaviour>, 2> toEnum =
    { {
        { "Fixed", WeaponBehaviour::Fixed },
        { "Turret", WeaponBehaviour::Turret }
    } };

    for ( int i = 0; i < toEnum.size(); ++i )
    {
        if ( text == toEnum[i].first )
        {
            value = toEnum[i].second;
            return true;
        }
    }

    return false;
}

bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Hexterminate::TowerBonus& value )
{
    SDL_assert( pElement != nullptr );
    if ( pElement == nullptr || name != pElement->Value() )
    {
        return false;
    }

    using namespace Hexterminate;
    const std::string text( pElement->GetText() );
    static const std::array<std::pair<std::string, TowerBonus>, 7> toEnum =
    { {
        { "None", TowerBonus::None },
        { "Damage", TowerBonus::Damage },
        { "Movement", TowerBonus::Movement },
        { "Shields", TowerBonus::Shields },
        { "Sensors", TowerBonus::Sensors },
        { "HyperspaceImmunity", TowerBonus::HyperspaceImmunity },
        { "Ramming", TowerBonus::Ramming }
    } };

    for ( int i = 0; i < toEnum.size(); ++i )
    {
        if ( text == toEnum[i].first )
        {
            value = toEnum[i].second;
            return true;
        }
    }

    return false;
}

bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Hexterminate::ReactorVariant& value )
{
    SDL_assert( pElement != nullptr );
    if ( pElement == nullptr || name != pElement->Value() )
    {
        return false;
    }

    using namespace Hexterminate;
    const std::string text( pElement->GetText() );
    static const std::array<std::pair<std::string, ReactorVariant>, 3> toEnum =
    { {
        { "Standard", ReactorVariant::Standard },
        { "Unstable", ReactorVariant::Unstable },
        { "HighCapacity", ReactorVariant::HighCapacity },
    } };

    for ( int i = 0; i < toEnum.size(); ++i )
    {
        if ( text == toEnum[i].first )
        {
            value = toEnum[i].second;
            return true;
        }
    }

    return false;
}

bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Hexterminate::FleetState& value )
{
    SDL_assert( pElement != nullptr );
    if ( pElement == nullptr || name != pElement->Value() )
    {
        return false;
    }

    using namespace Hexterminate;
    const std::string text( pElement->GetText() );
    static const std::array<std::pair<std::string, FleetState>, 4> toEnum =
    { {
        { "Idle", FleetState::Idle },
        { "Engaged", FleetState::Engaged },
        { "Moving", FleetState::Moving },
        { "Arrived", FleetState::Arrived }
    } };

    for ( int i = 0; i < toEnum.size(); ++i )
    {
        if ( text == toEnum[i].first )
        {
            value = toEnum[i].second;
            return true;
        }
    }

    return false;
}

bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Hexterminate::Difficulty& value )
{
    SDL_assert( pElement != nullptr );
    if ( pElement == nullptr || name != pElement->Value() )
    {
        return false;
    }

    using namespace Hexterminate;
    const std::string text( pElement->GetText() );
    static const std::array<std::pair<std::string, Difficulty>, 3> toEnum =
    { {
        { "Easy", Difficulty::Easy },
        { "Normal", Difficulty::Normal },
        { "Hardcore", Difficulty::Hardcore }
    } };

    for ( int i = 0; i < toEnum.size(); ++i )
    {
        if ( text == toEnum[i].first )
        {
            value = toEnum[i].second;
            return true;
        }
    }

    return false;
}

bool Serialise( tinyxml2::XMLElement* pElement, const std::string& name, Hexterminate::GameMode& value )
{
    SDL_assert( pElement != nullptr );
    if ( pElement == nullptr || name != pElement->Value() )
    {
        return false;
    }

    using namespace Hexterminate;
    const std::string text( pElement->GetText() );
    static const std::array<std::pair<std::string, GameMode>, 3> toEnum =
    { {
        { "Campaign", GameMode::Campaign },
        { "Infinite War", GameMode::InfiniteWar },
        { "Hyperscape", GameMode::Hyperscape }
    } };

    for ( int i = 0; i < toEnum.size(); ++i )
    {
        if ( text == toEnum[i].first )
        {
            value = toEnum[i].second;
            return true;
        }
    }

    return false;
}

void Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement, const std::string& elementName, const std::string& content )
{
    tinyxml2::XMLElement* pElement = xmlDoc.NewElement( elementName.c_str() );
    pElement->SetText( content.c_str() );
    pRootElement->InsertEndChild( pElement );
}

void Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement, const std::string& elementName, int content )
{
    Write( xmlDoc, pRootElement, elementName, std::to_string( content ) );
}

void Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement, const std::string& elementName, float content )
{
    Write( xmlDoc, pRootElement, elementName, std::to_string( content ) );
}

void Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement, const std::string& elementName, bool content )
{
    Write( xmlDoc, pRootElement, elementName, content ? std::string( "true" ) : std::string( "false" ) );
}

void Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement, const std::string& elementName, Hexterminate::FleetState content )
{
    using namespace Hexterminate;

    if ( content == FleetState::Idle )
        Write( xmlDoc, pRootElement, elementName, std::string( "Idle" ) );
    else if ( content == FleetState::Engaged )
        Write( xmlDoc, pRootElement, elementName, std::string( "Engaged" ) );
    else if ( content == FleetState::Moving )
        Write( xmlDoc, pRootElement, elementName, std::string( "Moving" ) );
    else if ( content == FleetState::Arrived )
        Write( xmlDoc, pRootElement, elementName, std::string( "Arrived" ) );
}

} // namespace Xml