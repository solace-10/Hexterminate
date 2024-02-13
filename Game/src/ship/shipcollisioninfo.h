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

#include <coredefines.h>

namespace Hexterminate
{

class AddonPhaseBarrier;
class Module;
class Shield;
class Ship;

enum class ShipCollisionType
{
    Module,
    Shield,
    PhaseBarrier
};

///////////////////////////////////////////////////////////////////////////////
// ShipCollisionInfo
// This is passed as the user data in a Shape and can then be retrieved as
// the result of a raycast or collision callback.
///////////////////////////////////////////////////////////////////////////////

class ShipCollisionInfo
{
public:
    ShipCollisionInfo( Ship* pShip, Module* pModule );
    ShipCollisionInfo( Ship* pShip, AddonPhaseBarrier* pAddonPhaseBarrier );
    ShipCollisionInfo( Ship* pShip, Shield* pShield );

    inline Ship* GetShip() const { return m_pShip; }
    inline ShipCollisionType GetType() { return m_Type; }
    inline Module* GetModule() const { return m_pModule; }
    inline AddonPhaseBarrier* GetAddonPhaseBarrier() const { return m_pAddonPhaseBarrier; }
    inline Shield* GetShield() const { return m_pShield; }

private:
    Ship* m_pShip;
    Module* m_pModule;
    AddonPhaseBarrier* m_pAddonPhaseBarrier;
    Shield* m_pShield;
    ShipCollisionType m_Type;
};
GENESIS_DECLARE_SMART_PTR( ShipCollisionInfo );

} // namespace Hexterminate