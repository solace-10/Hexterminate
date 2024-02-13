// Copyright 2021 Pedro Nunes
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

#include <memory>

#include <configuration.h>

namespace Genesis
{
struct Material;
}

namespace Hexterminate
{

class Ship;
class ShipOutline;
using ShipOutlineUniquePtr = std::unique_ptr<ShipOutline>;

class ShipOutline
{
public:
    ShipOutline();
    ~ShipOutline();

    void DrawShipOutlineTweaks();

    bool IsEnabled() const;
    float GetThickness() const;
    bool AffectsShip( Ship* pShip ) const;
    Genesis::Material* GetOutlineMaterial( Ship* pShip ) const;

private:
    float m_Thickness;
    Genesis::Material* m_pMaterialPlayer;
    Genesis::Material* m_pMaterialFriendly;
    Genesis::Material* m_pMaterialHostile;
};

inline bool ShipOutline::IsEnabled() const
{
    return Genesis::Configuration::GetOutlines();
}

inline float ShipOutline::GetThickness() const
{
    return m_Thickness;
}

} // namespace Hexterminate
