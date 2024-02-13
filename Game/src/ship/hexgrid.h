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

#include <cstddef>

#include <SDL.h>

#include "serialisable.h"

namespace Hexterminate
{

class ModuleInfo;

///////////////////////////////////////////////////////////////////////////////
// HexGrid
// Templated random access container which doesn't keep the ownership of the
// objects inside it.
///////////////////////////////////////////////////////////////////////////////

static const int sHexGridWidth = 4;
static const int sHexGridHeight = 18;

template <typename T>
class HexGrid : public Serialisable
{
public:
    HexGrid()
    {
        Clear();

        m_Unused = true;
        m_UsedSlots = 0;
    }

    virtual ~HexGrid()
    {
    }

    T Get( int x, int y ) const
    {
        if ( x < 0 || y < 0 || x >= sHexGridWidth || y >= sHexGridHeight )
        {
            return nullptr;
        }
        else
        {
            return m_HexGrid[ x ][ y ];
        }
    }

    void Set( int x, int y, T pElement )
    {
        SDL_assert( x >= 0 && x < sHexGridWidth );
        SDL_assert( y >= 0 && y < sHexGridHeight );
#ifdef _WIN32
        _Analysis_assume_( x >= 0 && x < sHexGridWidth );
        _Analysis_assume_( y >= 0 && y < sHexGridHeight );
#endif

        if ( pElement != m_HexGrid[ x ][ y ] )
        {
            m_HexGrid[ x ][ y ] = pElement;

            if ( pElement == nullptr )
            {
                m_UsedSlots--;
                SDL_assert( m_UsedSlots >= 0 );
            }
            else
            {
                m_UsedSlots++;
                Expand( x, y );
            }
        }
    }

    void Copy( HexGrid<T>* pSrc )
    {
        for ( int x = 0; x < sHexGridWidth; ++x )
        {
            for ( int y = 0; y < sHexGridHeight; ++y )
            {
                Set( x, y, pSrc->Get( x, y ) );
            }
        }
    }

    void Clear()
    {
        for ( int x = 0; x < sHexGridWidth; ++x )
        {
            for ( int y = 0; y < sHexGridHeight; ++y )
            {
                m_HexGrid[ x ][ y ] = nullptr;
            }
        }

        for ( int i = 0; i < 4; ++i )
        {
            m_BoundingBox[ i ] = -1;
        }
    }

    void GetBoundingBox( int& x1, int& y1, int& x2, int& y2 ) const
    {
        x1 = m_BoundingBox[ 0 ];
        y1 = m_BoundingBox[ 1 ];
        x2 = m_BoundingBox[ 2 ];
        y2 = m_BoundingBox[ 3 ];
    }

    int GetUsedSlots() const
    {
        return m_UsedSlots;
    }

    // Serialisable
    virtual bool Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement ) { return false; }
    virtual bool Read( tinyxml2::XMLElement* pRootElement ) { return false; }
    virtual int GetVersion() const { return 1; }
    virtual void UpgradeFromVersion( int version ) {}

private:
    void Expand( int x, int y )
    {
        if ( m_Unused )
        {
            m_BoundingBox[ 0 ] = m_BoundingBox[ 2 ] = x;
            m_BoundingBox[ 1 ] = m_BoundingBox[ 3 ] = y;
            m_Unused = false;
        }
        else
        {
            if ( x < m_BoundingBox[ 0 ] )
                m_BoundingBox[ 0 ] = x;
            else if ( x > m_BoundingBox[ 2 ] )
                m_BoundingBox[ 2 ] = x;

            if ( y < m_BoundingBox[ 1 ] )
                m_BoundingBox[ 1 ] = y;
            else if ( y > m_BoundingBox[ 3 ] )
                m_BoundingBox[ 3 ] = y;
        }
    }

    T m_HexGrid[ sHexGridWidth ][ sHexGridHeight ];
    int m_BoundingBox[ 4 ];
    int m_UsedSlots;
    bool m_Unused;
};

///////////////////////////////////////////////////////////////////////////////
// HexGrid<ModuleInfo*>
// Template specialisation so we can serialise it
///////////////////////////////////////////////////////////////////////////////

bool WriteHexGridModuleInfo( HexGrid<ModuleInfo*>* pHexGrid, tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement );
bool ReadHexGridModuleInfo( HexGrid<ModuleInfo*>* pHexGrid, tinyxml2::XMLElement* pRootElement );

#ifdef _MSC_VER

template <>
int HexGrid<ModuleInfo*>::GetVersion() const
{
    return 1;
}

template <>
bool HexGrid<ModuleInfo*>::Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement )
{
    return WriteHexGridModuleInfo( this, xmlDoc, pRootElement );
}
template <>
bool HexGrid<ModuleInfo*>::Read( tinyxml2::XMLElement* pRootElement )
{
    return ReadHexGridModuleInfo( this, pRootElement );
}

#else

template <>
int HexGrid<ModuleInfo*>::GetVersion() const;
template <>
bool HexGrid<ModuleInfo*>::Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement );
template <>
bool HexGrid<ModuleInfo*>::Read( tinyxml2::XMLElement* pRootElement );

#endif // _MSC_VER

} // namespace Hexterminate
