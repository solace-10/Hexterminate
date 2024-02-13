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

#include "player.h"
#include "hexterminate.h"
#include "menus/intelwindow.h"
#include "perks.h"
#include "ship/inventory.h"
#include "ship/moduleinfo.h"
#include "ship/ship.h"
#include "ship/shipinfo.h"
#include "xmlaux.h"
#include <xml.h>

namespace Hexterminate
{

Player::Player()
    : m_pShip( nullptr )
    , m_pPerks( nullptr )
    , m_pInventory( nullptr )
    , m_Influence( 0 )
    , m_PerkPoints( 0 )
    , m_PerkPointsParts( 0 )
{
    m_pInventory = new Inventory();
    m_pPerks = new Perks();
}

Player::Player( const ShipCustomisationData& shipCustomisationData, const std::string& companionShipTemplate )
    : m_ShipCustomisationData( shipCustomisationData )
    , m_pShip( nullptr )
    , m_pPerks( nullptr )
    , m_pInventory( nullptr )
    , m_CompanionShipTemplate( companionShipTemplate )
    , m_Influence( 0 )
    , m_PerkPoints( 0 )
    , m_PerkPointsParts( 0 )
{
    m_pInventory = new Inventory();
    AssembleDefaultInventory();

    m_pPerks = new Perks();
}

Player::~Player()
{
    delete m_pInventory;
    delete m_pPerks;
}

Ship* Player::CreateShip( float spawnPointX, float spawnPointY )
{
    SDL_assert( m_pShip == nullptr );

    ShipSpawnData spawnData;
    spawnData.m_PositionX = spawnPointX;
    spawnData.m_PositionY = spawnPointY;

    m_pShip = new Ship();
    m_pShip->SetInitialisationParameters(
        g_pGame->GetPlayerFaction(),
        g_pGame->GetPlayerFleet(),
        m_ShipCustomisationData,
        spawnData,
        nullptr );

    m_pShip->Initialize();

    return m_pShip;
}

void Player::AssembleDefaultInventory()
{
    ModuleInfoHexGrid* pSrc = m_ShipCustomisationData.m_pModuleInfoHexGrid;
    ModuleInfoHexGrid* pDst = m_pInventory->GetHexGrid();
    if ( pSrc != nullptr )
    {
        *pDst = *pSrc;

        // Add all the modules that the ship itself contains into the inventory
        // We add them as quantity "0" because they are already in use by the ship
        int x1, y1, x2, y2;
        pSrc->GetBoundingBox( x1, y1, x2, y2 );
        for ( int y = y1; y <= y2; ++y )
        {
            for ( int x = x1; x <= x2; ++x )
            {
                ModuleInfo* pInfo = pSrc->Get( x, y );
                if ( pInfo != nullptr )
                    m_pInventory->AddModule( pInfo->GetName(), 0 );
            }
        }
    }
    m_ShipCustomisationData.m_pModuleInfoHexGrid = m_pInventory->GetHexGrid();
}

bool Player::Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement )
{
    tinyxml2::XMLElement* pElement = xmlDoc.NewElement( "Player" );
    pRootElement->LinkEndChild( pElement );

    Xml::Write( xmlDoc, pElement, "Version", GetVersion() );
    Xml::Write( xmlDoc, pElement, "CaptainName", m_ShipCustomisationData.m_CaptainName );
    Xml::Write( xmlDoc, pElement, "ShipName", m_ShipCustomisationData.m_ShipName );
    Xml::Write( xmlDoc, pElement, "PlayedTime", g_pGame->GetPlayedTime() );
    Xml::Write( xmlDoc, pElement, "CompanionShipTemplate", m_CompanionShipTemplate );
    Xml::Write( xmlDoc, pElement, "RequisitionUnits", m_Influence );
    Xml::Write( xmlDoc, pElement, "PerkPoints", m_PerkPoints );
    Xml::Write( xmlDoc, pElement, "PerkPointsParts", m_PerkPointsParts );

    bool result = true;
    result &= GetInventory()->Write( xmlDoc, pElement );
    result &= GetPerks()->Write( xmlDoc, pElement );

    return result;
}

bool Player::Read( tinyxml2::XMLElement* pRootElement )
{
    m_CompanionShipTemplate = "lancer"; // Fallback for old savegames not containing a CompanionShipTemplate entry

    float playedTime = 0.0f;
    for ( tinyxml2::XMLElement* pElement = pRootElement->FirstChildElement(); pElement != nullptr; pElement = pElement->NextSiblingElement() )
    {
        Xml::Serialise( pElement, "CaptainName", m_ShipCustomisationData.m_CaptainName );
        Xml::Serialise( pElement, "ShipName", m_ShipCustomisationData.m_ShipName );
        Xml::Serialise( pElement, "PlayedTime", playedTime );
        Xml::Serialise( pElement, "CompanionShipTemplate", m_CompanionShipTemplate );
        Xml::Serialise( pElement, "RequisitionUnits", m_Influence );
        Xml::Serialise( pElement, "PerkPoints", m_PerkPoints );
        Xml::Serialise( pElement, "PerkPointsParts", m_PerkPointsParts );

        if ( std::string( pElement->Value() ) == "Inventory" )
        {
            m_pInventory->Read( pElement );
        }
        else if ( std::string( pElement->Value() ) == "Perks" )
        {
            m_pPerks->Read( pElement );
        }
    }

    m_ShipCustomisationData.m_pModuleInfoHexGrid = m_pInventory->GetHexGrid();

    g_pGame->SetPlayedTime( playedTime );

    return true;
}

unsigned int Player::GetFleetMaxShips() const
{
    if ( m_pPerks->IsEnabled( Perk::PrimaryFleet ) )
    {
        return 5;
    }
    else if ( m_pPerks->IsEnabled( Perk::SupportRequest ) )
    {
        return 4;
    }
    else
    {
        return 3;
    }
}

} // namespace Hexterminate