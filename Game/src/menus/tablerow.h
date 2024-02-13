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

#include <string>
#include <vector>

#include <color.h>

#include "menus/eva.h"

typedef std::vector<std::string> TableRowContents;

namespace Genesis
{
class ResourceFont;
}

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////
// TableRow
// A row of text to be used with a Table. A user data field can be set in
// order to be able to point back to non-text information.
///////////////////////////////////////////////////////////////////////////

class TableRow
{
public:
    TableRow()
        : m_pUserData( nullptr )
        , m_Colour( EVA_TEXT_COLOUR )
        , m_pFont( EVA_FONT ){};
    virtual ~TableRow(){};

    void Add( const std::string& text );
    void Set( unsigned int pos, const std::string& text );
    const std::string& Get( unsigned int pos ) const;
    void SetUserData( void* pUserData );
    void* GetUserData() const;
    void SetColour( const Genesis::Color& colour );
    const Genesis::Color& GetColour() const;
    void SetFont( Genesis::ResourceFont* pFont );
    Genesis::ResourceFont* GetFont() const;

    const TableRowContents& GetContents() const;

    virtual void OnPress(){};
    virtual void OnHover(){};

private:
    TableRowContents m_Contents;
    void* m_pUserData;
    Genesis::Color m_Colour;
    Genesis::ResourceFont* m_pFont;
};

inline void TableRow::Add( const std::string& text )
{
    m_Contents.push_back( text );
}

inline void TableRow::Set( unsigned int pos, const std::string& text )
{
    m_Contents.at( pos ) = text;
}

inline const std::string& TableRow::Get( unsigned int pos ) const
{
    return m_Contents.at( pos );
}

inline void TableRow::SetUserData( void* pUserData )
{
    m_pUserData = pUserData;
}

inline void* TableRow::GetUserData() const
{
    return m_pUserData;
}

inline const TableRowContents& TableRow::GetContents() const
{
    return m_Contents;
}

inline void TableRow::SetColour( const Genesis::Color& colour )
{
    m_Colour = colour;
}

inline const Genesis::Color& TableRow::GetColour() const
{
    return m_Colour;
}

inline void TableRow::SetFont( Genesis::ResourceFont* pFont )
{
    m_pFont = pFont;
}

inline Genesis::ResourceFont* TableRow::GetFont() const
{
    return m_pFont;
}

} // namespace Hexterminate
