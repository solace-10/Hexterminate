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

#include "ship/shipcollisioninfo.h"
#include "ship/addon/addonphasebarrier.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// ShipCollisionInfo
///////////////////////////////////////////////////////////////////////////////

ShipCollisionInfo::ShipCollisionInfo( Ship* pShip, Module* pModule )
    : m_pShip( pShip )
    , m_pModule( pModule )
    , m_pAddonPhaseBarrier( nullptr )
    , m_pShield( nullptr )
    , m_Type( ShipCollisionType::Module )
{
}

ShipCollisionInfo::ShipCollisionInfo( Ship* pShip, AddonPhaseBarrier* pAddonPhaseBarrier )
    : m_pShip( pShip )
    , m_pModule( nullptr )
    , m_pAddonPhaseBarrier( pAddonPhaseBarrier )
    , m_pShield( nullptr )
    , m_Type( ShipCollisionType::PhaseBarrier )
{
    m_pModule = pAddonPhaseBarrier->GetModule();
}

ShipCollisionInfo::ShipCollisionInfo( Ship* pShip, Shield* pShield )
    : m_pShip( pShip )
    , m_pModule( nullptr )
    , m_pAddonPhaseBarrier( nullptr )
    , m_pShield( pShield )
    , m_Type( ShipCollisionType::Shield )
{
}

} // namespace Hexterminate