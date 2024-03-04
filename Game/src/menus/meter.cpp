// Copyright 2015 Pedro Nunes
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

#include "menus/meter.h"

#include "menus/eva.h"
#include <cassert>

namespace Hexterminate
{

Meter::Meter()
    : m_PipColor( 1.0f, 1.0f, 1.0f, 0.5f )
{
}

void Meter::Init( int initialValue, int maxValue )
{
    SDL_assert( initialValue >= 0 );
    SDL_assert( maxValue >= 1 );
    SDL_assert( initialValue <= maxValue );

    m_Panels.resize( maxValue );

    const int panelSize = 12;
    const int panelSpacing = 2;

    SetColor( EVA_COLOR_BACKGROUND );
    SetBorderColor( EVA_COLOR_BORDER );
    SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
    SetSize( ( panelSize + panelSpacing ) * maxValue + panelSpacing + 1, panelSize + panelSpacing * 2 );

    for ( int i = 0; i < (int)maxValue; ++i )
    {
        m_Panels[ i ] = new Genesis::Gui::Panel();
        m_Panels[ i ]->SetPosition( ( panelSize + panelSpacing ) * i + panelSpacing + 1, panelSpacing );
        m_Panels[ i ]->SetSize( panelSize, panelSize );
        m_Panels[ i ]->SetColor( m_PipColor );
        m_Panels[ i ]->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
        m_Panels[ i ]->Show( ( i + 1 ) <= initialValue );

        AddElement( m_Panels[ i ] );
    }
}

void Meter::SetValue( int value )
{
    for ( int i = 0, c = static_cast<int>( m_Panels.size() ); i < c; ++i )
    {
        m_Panels[ i ]->Show( ( i + 1 ) <= value );
    }
}

void Meter::SetPipColor( const Genesis::Color& color )
{
    m_PipColor = color;

    for ( auto& pPip : m_Panels )
    {
        pPip->SetColor( color );
    }
}

void Meter::SetPipColor( float r, float g, float b, float a )
{
    SetPipColor( Genesis::Color( r, g, b, a ) );
}

} // namespace Hexterminate