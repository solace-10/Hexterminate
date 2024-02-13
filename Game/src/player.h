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

#pragma once

#include "serialisable.h"
#include "ship/ship.h"
#include <list>

namespace Hexterminate
{

class Inventory;
class Perks;

class Player : public Serialisable
{
public:
    Player();
    Player( const ShipCustomisationData& shipCustomisationData, const std::string& companionShipTemplate );
    ~Player();

    Ship* CreateShip( float spawnPointX, float spawnPointY );

    ShipCustomisationData& GetShipCustomisationData() { return m_ShipCustomisationData; }
    Ship* GetShip() const { return m_pShip; }
    void UnassignShip() { m_pShip = nullptr; }
    Inventory* GetInventory() const { return m_pInventory; }
    Perks* GetPerks() const { return m_pPerks; }
    const std::string& GetCompanionShipTemplate() const;
    int GetInfluence() const;
    void SetInfluence( int value );
    int GetPerkPoints() const;
    void SetPerkPoints( int value );
    int GetPerkPointsParts() const;
    void SetPerkPointsParts( int value );
    unsigned int GetFleetMaxShips() const;

    // Serialisable
    virtual bool Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement ) override;
    virtual bool Read( tinyxml2::XMLElement* pRootElement ) override;
    virtual int GetVersion() const override { return 2; }
    virtual void UpgradeFromVersion( int version ) override {}

private:
    void AssembleDefaultInventory();

    ShipCustomisationData m_ShipCustomisationData;
    Ship* m_pShip;
    Perks* m_pPerks;
    Inventory* m_pInventory;
    std::string m_CompanionShipTemplate;
    int m_Influence;
    int m_PerkPoints;
    int m_PerkPointsParts;
};

inline const std::string& Player::GetCompanionShipTemplate() const
{
    return m_CompanionShipTemplate;
}

inline int Player::GetInfluence() const
{
    return m_Influence;
}

inline void Player::SetInfluence( int value )
{
    // The amount of influence units the player can have is capped in order to fit the user interface
    m_Influence = std::min( value, 999999 );
}

inline int Player::GetPerkPoints() const
{
    return m_PerkPoints;
}

inline void Player::SetPerkPoints( int value )
{
    m_PerkPoints = value;
}

inline int Player::GetPerkPointsParts() const
{
    return m_PerkPointsParts;
}

inline void Player::SetPerkPointsParts( int value )
{
    m_PerkPointsParts = value;
}

} // namespace Hexterminate
