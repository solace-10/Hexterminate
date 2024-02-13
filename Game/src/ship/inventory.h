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

#include <unordered_map>

// clang-format off
#include <beginexternalheaders.h>
#include <tinyxml2.h>
#include <endexternalheaders.h>
// clang-format on

#include "serialisable.h"
#include "ship/moduleinfo.h"

struct ItemData
{
    ItemData()
        : quantity( 0u )
        , cached( 0u )
    {
    }

    ItemData( unsigned int _quantity, unsigned int _cached )
        : quantity( _quantity )
        , cached( _cached )
    {
    }

    unsigned int quantity;
    unsigned int cached;
};

using InventoryItemMap = std::unordered_map<std::string, ItemData>;

namespace Hexterminate
{

class Inventory : public Serialisable
{
public:
    Inventory(){};
    ~Inventory(){};

    ModuleInfoHexGrid* GetHexGrid() { return &m_HexGrid; }
    void AddModule( const std::string& moduleName, unsigned int quantity = 1, unsigned int cached = 0, bool triggersNotification = false );
    void RemoveModule( const std::string& moduleName );
    const InventoryItemMap& GetItems() const;
    int GetModuleCount( const std::string& moduleName );
    void ClearCachedModules();
    void DeductCachedModules();

    // Serialisable
    virtual bool Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement ) override;
    virtual bool Read( tinyxml2::XMLElement* pRootElement ) override;
    virtual int GetVersion() const override { return 2; }
    virtual void UpgradeFromVersion( int version ) override;

private:
    bool ReadItems( tinyxml2::XMLElement* pRootElement );

    ModuleInfoHexGrid m_HexGrid;
    InventoryItemMap m_Items;
};

inline const InventoryItemMap& Inventory::GetItems() const
{
    return m_Items;
}

} // namespace Hexterminate
