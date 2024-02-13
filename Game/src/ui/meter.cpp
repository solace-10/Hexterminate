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

#include <gui/gui.h>
#include <imgui/imgui.h>

#include "ui/meter.h"

namespace Hexterminate::UI
{

static const char* sMeterPropertyPips = "pips";

Meter::Meter( const std::string& name, int maxValue )
    : Panel( name )
    , m_Pips( 0 )
{
    using namespace Genesis;

    SDL_assert( maxValue >= 1 );

    m_Panels.resize( maxValue );

    const int panelSize = 12;
    const int panelSpacing = 2;

    GetPanel()->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
    SetSize( ( panelSize + panelSpacing ) * maxValue + panelSpacing + 1, panelSize + panelSpacing * 2 );

    for ( int i = 0; i < (int)maxValue; ++i )
    {
        m_Panels[ i ] = new Genesis::Gui::Panel();
        m_Panels[ i ]->SetPosition( ( panelSize + panelSpacing ) * i + panelSpacing + 1, panelSpacing );
        m_Panels[ i ]->SetSize( panelSize, panelSize );
        m_Panels[ i ]->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
        m_Panels[ i ]->Show( false );

        GetPanel()->AddElement( m_Panels[ i ] );
    }
}

void Meter::SaveProperties( json& properties )
{
    Element::SaveProperties( properties );

    properties[ sMeterPropertyPips ] = m_Pips;
}

void Meter::LoadProperties( const json& properties )
{
    Element::LoadProperties( properties );

    if ( properties.contains( sMeterPropertyPips ) )
    {
        m_Pips = properties[ sMeterPropertyPips ].get<int>();
        UpdatePips();
    }
}

void Meter::RenderProperties()
{
    Element::RenderProperties();

    if ( ImGui::CollapsingHeader( "Meter", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        if ( ImGui::SliderInt( "Pips", &m_Pips, 0, static_cast<int>( m_Panels.size() ) ) )
        {
            UpdatePips();
        }
    }
}

void Meter::UpdatePips()
{
    const int numPanels = static_cast<int>( m_Panels.size() );
    for ( int i = 0; i < numPanels; ++i )
    {
        if ( i < m_Pips - 1 )
        {
            m_Panels[ i ]->SetColour( 0.4f, 0.4f, 0.4f, 1.0f );
        }
        else
        {
            m_Panels[ i ]->SetColour( 0.309f, 0.639f, 0.690f, 1.0f );
        }

        m_Panels[ i ]->Show( i < m_Pips );
    }
}

} // namespace Hexterminate::UI
