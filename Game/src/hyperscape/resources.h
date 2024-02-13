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

#include <string>

namespace Hexterminate
{

//-----------------------------------------------------------------------------
// Resource
//-----------------------------------------------------------------------------

class Resource
{
public:
    enum class Type
    {
        CommonMetals,
        RareMetals,
        Helium3,
        QuantumCore,

        Count
    };

    Type GetType() const;
    const std::string& GetName() const;
    unsigned int GetQuantity() const;
    void SetQuantity( unsigned int quantity );

protected:
    Resource( Type type, const std::string& name, unsigned int quantity );

private:
    Type m_Type;
    std::string m_Name;
    unsigned int m_Quantity;
};

inline Resource::Type Resource::GetType() const
{
    return m_Type;
}

inline const std::string& Resource::GetName() const
{
    return m_Name;
}

inline unsigned int Resource::GetQuantity() const
{
    return m_Quantity;
}

inline void Resource::SetQuantity( unsigned int quantity )
{
    m_Quantity = quantity;
}

//-----------------------------------------------------------------------------
// CommonMetalsResource
//-----------------------------------------------------------------------------

class CommonMetalsResource : public Resource
{
public:
    CommonMetalsResource( unsigned int quantity = 0 );
};

//-----------------------------------------------------------------------------
// RareMetalsResource
//-----------------------------------------------------------------------------

class RareMetalsResource : public Resource
{
public:
    RareMetalsResource( unsigned int quantity = 0 );
};

//-----------------------------------------------------------------------------
// Helium3Resource
//-----------------------------------------------------------------------------

class Helium3Resource : public Resource
{
public:
    Helium3Resource( unsigned int quantity = 0 );
};

//-----------------------------------------------------------------------------
// QuantumCoreResource
//-----------------------------------------------------------------------------

class QuantumCoreResource : public Resource
{
public:
    QuantumCoreResource( unsigned int quantity = 0 );
};

} // namespace Hexterminate
