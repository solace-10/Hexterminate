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

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "faction/faction.h"
#include "perks.h"
#include "ship/moduleinfo.h"
#include "ship/ship.fwd.h"
#include "ship/weapon.h"

namespace Hexterminate
{

class Fleet;
class ShipInfo;
typedef std::vector<const ShipInfo*> ShipInfoVector;

///////////////////////////////////////////////////////////////////////////////
// ShipInfoManager
// Loads the hexgrids from "data/xml/ships", and makes them available by name.
///////////////////////////////////////////////////////////////////////////////

class ShipInfoManager
{
public:
    ShipInfoManager();
    ~ShipInfoManager();
    void Initialise();

    const ShipInfo* Get( const Faction* pFaction, const std::string& name ) const;
    const ShipInfoVector& Get( const Faction* pFaction ) const;

private:
    void SerialiseHexGrid( const Faction* pFaction, const std::filesystem::path& filename );
    void SerialiseXml( const Faction* pFaction, const std::filesystem::path& filename );
    ShipInfoVector m_Data[ static_cast<unsigned int>( FactionId::Count ) ];
    bool m_Initialised;
};

inline const ShipInfoVector& ShipInfoManager::Get( const Faction* pFaction ) const
{
    return m_Data[ static_cast<int>( pFaction->GetFactionId() ) ];
}

///////////////////////////////////////////////////////////////////////////////
// ShipInfo
// Contains the basic information that is necessary to spawn a ship.
// Every ship that is spawned contains a pointer to its ShipInfo.
// The ShipInfo is held by its Fleet.
// A ship is flagged as being "special" if its name is prefixed with "special_"
// and is meant to be manually spawned rather than as part of the fleet
// spawning process.
///////////////////////////////////////////////////////////////////////////////

class ShipInfo
{
public:
    ShipInfo( const std::string& name, ModuleInfoHexGrid* pModuleInfoHexGrid );
    ~ShipInfo();

    ModuleInfoHexGrid* GetModuleInfoHexGrid() const { return m_pModuleInfoHexGrid; }
    const std::string& GetName() const { return m_Name; }
    int GetPoints() const { return m_Points; }
    int GetThreatValue() const { return m_ThreatValue; }
    ShipType GetShipType() const { return m_ShipType; }
    bool IsSpecial() const { return m_IsSpecial; }
    bool IsFlagship() const { return m_IsFlagship; }

    const std::string& GetDisplayName() const { return m_DisplayName; }
    const std::string& GetLongDisplayName() const { return m_LongDisplayName; }
    const std::string& GetDisplayImage() const { return m_DisplayImage; }
    const std::string& GetWeaponsText() const { return m_Weapons; }
    const std::string& GetDefenseText() const { return m_Defense; }
    int GetCost() const { return m_Cost; }
    std::optional<Perk> GetRequiredPerk() const;

    void SetDisplayName( const std::string& name ) { m_DisplayName = name; }
    void SetLongDisplayName( const std::string& name ) { m_LongDisplayName = name; }
    void SetDisplayImage( const std::string& file ) { m_DisplayImage = file; }
    void SetWeaponsText( const std::string& text ) { m_Weapons = text; }
    void SetDefenseText( const std::string& text ) { m_Defense = text; }
    void SetCost( int cost ) { m_Cost = cost; }
    void SetTier( int tier ) { m_Tier = tier; }

    static int sCalculatePoints( int numSlots );
    static int sCalculateThreatValue( ModuleInfoHexGrid* pHexGrid );
    static ShipType sCalculateShipType( int numSlots );

private:
    std::string m_Name;
    ModuleInfoHexGrid* m_pModuleInfoHexGrid;
    int m_Points;
    int m_ThreatValue;
    ShipType m_ShipType;
    bool m_IsSpecial;
    bool m_IsFlagship;

    // Extended information only available through a matching XML
    std::string m_DisplayName;
    std::string m_LongDisplayName;
    std::string m_DisplayImage;
    std::string m_Weapons;
    std::string m_Defense;
    int m_Cost;
    int m_Tier;
};

} // namespace Hexterminate
