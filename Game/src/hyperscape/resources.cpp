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

#include "hyperscape/resources.h"

namespace Hexterminate
{

//-----------------------------------------------------------------------------
// Resource
//-----------------------------------------------------------------------------

Resource::Resource( Type type, const std::string& name, unsigned int quantity )
    : m_Type( type )
    , m_Name( name )
    , m_Quantity( quantity )
{
}

//-----------------------------------------------------------------------------
// CommonMetalsResource
//-----------------------------------------------------------------------------

CommonMetalsResource::CommonMetalsResource( unsigned int quantity /* = 0 */ )
    : Resource( Type::CommonMetals, "Common metals", quantity )
{
}

//-----------------------------------------------------------------------------
// RareMetalsResource
//-----------------------------------------------------------------------------

RareMetalsResource::RareMetalsResource( unsigned int quantity /* = 0 */ )
    : Resource( Type::RareMetals, "Rare metals", quantity )
{
}

//-----------------------------------------------------------------------------
// Helium3Resource
//-----------------------------------------------------------------------------

Helium3Resource::Helium3Resource( unsigned int quantity /* = 0 */ )
    : Resource( Type::Helium3, "Helium-3", quantity )
{
}

//-----------------------------------------------------------------------------
// QuantumCoreResource
//-----------------------------------------------------------------------------

QuantumCoreResource::QuantumCoreResource( unsigned int quantity /* = 0 */ )
    : Resource( Type::QuantumCore, "Quantum core", quantity )
{
}

} // namespace Hexterminate
