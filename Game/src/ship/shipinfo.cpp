// Copyright 2016 Pedro Nunes
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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <genesis.h>
#include <logger.h>
#include <xml.h>

#include "fleet/fleet.h"
#include "globals.h"
#include "hexterminate.h"
#include "ship/hexgrid.h"
#include "ship/moduleinfo.h"
#include "ship/shipinfo.h"
#include "stringaux.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// ShipInfoManager
///////////////////////////////////////////////////////////////////////////////

ShipInfoManager::ShipInfoManager()
    : m_Initialised( false )
{
}

ShipInfoManager::~ShipInfoManager()
{
    for ( int i = 0; i < static_cast<int>( FactionId::Count ); ++i )
    {
        for ( auto& pShipInfo : m_Data[ i ] )
        {
            delete pShipInfo;
        }
    }
}

void ShipInfoManager::Initialise()
{
    if ( m_Initialised )
        return;

    // Run through all the files in data/xml/ships and load all the ships that are listed
    const std::filesystem::path basePath( "data/xml/ships" );
    for ( int i = 0; i < static_cast<int>( FactionId::Count ); ++i )
    {
        Faction* pFaction = g_pGame->GetFaction( static_cast<FactionId>( i ) );
        Genesis::FrameWork::GetLogger()->LogInfo( "Loading ships for faction '%s'.", pFaction->GetName().c_str() );

        // These need to be separated into two loops, as the order in which files are listed
        // by the directory_iterator is not guaranteed and XML files reference SHP files which
        // have to be loaded first.
        std::filesystem::path factionPath = basePath / ToLower( pFaction->GetName() );
        for ( const auto& filename : std::filesystem::directory_iterator( factionPath ) )
        {
            const std::filesystem::path& extension = filename.path().extension();
            if ( extension == ".shp" )
            {
                SerialiseHexGrid( pFaction, filename );
            }
        }

        for ( const auto& filename : std::filesystem::directory_iterator( factionPath ) )
        {
            const std::filesystem::path& extension = filename.path().extension();
            if ( extension == ".xml" )
            {
                SerialiseXml( pFaction, filename );
            }
        }
    }

    m_Initialised = true;
}

void ShipInfoManager::SerialiseHexGrid( const Faction* pFaction, const std::filesystem::path& filename )
{

    ModuleInfoManager* pModuleInfoManager = g_pGame->GetModuleInfoManager();
    std::string line;
    std::ifstream fs( filename );
    std::string name = ToString( filename.stem() );
    if ( fs.is_open() )
    {
        ModuleInfoHexGrid* pHexGrid = new ModuleInfoHexGrid();

        int x, y;
        std::string moduleName;
        bool loadedWithWarnings = false;
        while ( fs.good() )
        {
            fs >> x >> y >> moduleName;

            ModuleInfo* pModuleInfo = pModuleInfoManager->GetModuleByName( moduleName );
            if ( pModuleInfo == nullptr )
            {
                Genesis::FrameWork::GetLogger()->LogInfo( "%s - Links to non-existent module '%s'", filename.c_str(), moduleName.c_str() );
                loadedWithWarnings = true;
            }
            else
            {
                pHexGrid->Set( x, y, pModuleInfo );
            }
        }
        fs.close();

        m_Data[ (int)pFaction->GetFactionId() ].push_back( new ShipInfo( name, pHexGrid ) );

        if ( loadedWithWarnings )
        {
            Genesis::FrameWork::GetLogger()->LogWarning( "%s - Loaded with warnings", filename.c_str() );
        }
    }
    else
    {
        Genesis::FrameWork::GetLogger()->LogWarning( "Could not load HexGrid template '%s'", name.c_str() );
    }
}

void ShipInfoManager::SerialiseXml( const Faction* pFaction, const std::filesystem::path& filename )
{
    using namespace tinyxml2;

    std::filesystem::path name = filename.stem();
    ShipInfo* pShipInfo = const_cast<ShipInfo*>( Get( pFaction, ToString( name ) ) );
    if ( pShipInfo == nullptr )
    {
        return;
    }

    FILE* fp = nullptr;

#ifdef _WIN32
    errno_t err = _wfopen_s( &fp, filename.c_str(), L"rb" );
    if ( err != 0 )
    {
        char errorMessage[ 256 ];
        strerror_s( errorMessage, 256, err );
        Genesis::FrameWork::GetLogger()->LogWarning( "Couldn't open file '%s': %s", filename.c_str(), errorMessage );
        return;
    }
#else
    fp = fopen( filename.c_str(), "rb" );
    if ( fp == nullptr )
    {
        Genesis::FrameWork::GetLogger()->LogError( "Couldn't open file '%s'.", filename.c_str() );
        return;
    }
#endif

    tinyxml2::XMLDocument doc;
    if ( doc.LoadFile( fp ) != XML_SUCCESS )
    {
#ifdef _WIN32
        _Analysis_assume_( fp != nullptr );
#endif
        fclose( fp );
        return;
    }

    std::string displayName;
    std::string longDisplayName;
    std::string displayImage;
    std::string defenseText;
    std::string weaponsText;
    int cost = 0;
    int tier = 1;

    XMLElement* pRootElem = doc.FirstChildElement();
    for ( XMLElement* pElem = pRootElem->FirstChildElement(); pElem != nullptr; pElem = pElem->NextSiblingElement() )
    {
        Xml::Serialise( pElem, "DisplayName", displayName );
        Xml::Serialise( pElem, "LongDisplayName", longDisplayName );
        Xml::Serialise( pElem, "DisplayImage", displayImage );
        Xml::Serialise( pElem, "Defense", defenseText );
        Xml::Serialise( pElem, "Weapons", weaponsText );
        Xml::Serialise( pElem, "Cost", cost );
        Xml::Serialise( pElem, "Tier", tier );
    }

    pShipInfo->SetDisplayName( displayName );
    pShipInfo->SetLongDisplayName( longDisplayName );
    pShipInfo->SetDisplayImage( displayImage );
    pShipInfo->SetDefenseText( defenseText );
    pShipInfo->SetWeaponsText( weaponsText );
    pShipInfo->SetCost( cost );
    pShipInfo->SetTier( tier );

#ifdef _WIN32
    _Analysis_assume_( fp != nullptr );
#endif
    fclose( fp );
}

const ShipInfo* ShipInfoManager::Get( const Faction* pFaction, const std::string& shipName ) const
{
    int idx = static_cast<int>( pFaction->GetFactionId() );
    for ( auto& pShipInfo : m_Data[ idx ] )
    {
        if ( shipName == pShipInfo->GetName() )
            return pShipInfo;
    }

    Genesis::FrameWork::GetLogger()->LogError( "Couldn't find ship '%s' for faction '%s'", shipName.c_str(), pFaction->GetName().c_str() );
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// ShipInfo
///////////////////////////////////////////////////////////////////////////////

ShipInfo::ShipInfo( const std::string& name, ModuleInfoHexGrid* pModuleInfoHexGrid )
    : m_Name( name )
    , m_pModuleInfoHexGrid( pModuleInfoHexGrid )
    , m_Points( 0 )
    , m_ThreatValue( 0 )
    , m_ShipType( ShipType::Invalid )
    , m_IsSpecial( false )
    , m_IsFlagship( false )
    , m_Cost( 0 )
    , m_Tier( 1 )
{
    SDL_assert_release( pModuleInfoHexGrid != nullptr );
#ifdef _WIN32
    _Analysis_assume_( pModuleInfoHexGrid != nullptr );
#endif

    m_IsSpecial = ( name.find( "special_" ) != std::string::npos );
    m_IsFlagship = ( name == "special_flagship" );
    m_Points = sCalculatePoints( pModuleInfoHexGrid->GetUsedSlots() );
    m_ThreatValue = sCalculateThreatValue( pModuleInfoHexGrid );
    m_ShipType = sCalculateShipType( pModuleInfoHexGrid->GetUsedSlots() );
}

ShipInfo::~ShipInfo()
{
    delete m_pModuleInfoHexGrid;
}

int ShipInfo::sCalculatePoints( int numSlots )
{
    return numSlots * PointsPerModule;
}

int ShipInfo::sCalculateThreatValue( ModuleInfoHexGrid* pModuleInfoHexGrid )
{
    int threatValue = 0;
    int x1, y1, x2, y2;
    pModuleInfoHexGrid->GetBoundingBox( x1, y1, x2, y2 );

    for ( int x = x1; x <= x2; ++x )
    {
        for ( int y = y1; y <= y2; ++y )
        {
            ModuleInfo* pModuleInfo = pModuleInfoHexGrid->Get( x, y );
            if ( pModuleInfo != nullptr )
            {
                ModuleRarity rarity = pModuleInfo->GetRarity();
                threatValue += ( (int)rarity + 1 ) * PointsPerModule;
            }
        }
    }

    return threatValue;
}

std::optional<Perk> ShipInfo::GetRequiredPerk() const
{
    if ( m_Tier == 2 )
    {
        return Perk::PriorityRequisition;
    }
    else if ( m_Tier == 3 )
    {
        return Perk::PrototypeAccess;
    }
    else
    {
        return std::optional<Perk>();
    }
}

ShipType ShipInfo::sCalculateShipType( int numSlots )
{
    int points = sCalculatePoints( numSlots );
    SDL_assert( points > 0 );
    if ( points <= MaxPointsGunship )
        return ShipType::Gunship;
    else if ( points <= MaxPointsBattlecruiser )
        return ShipType::Battlecruiser;
    else
        return ShipType::Capital;
}

} // namespace Hexterminate