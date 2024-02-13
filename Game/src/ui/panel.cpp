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

#include "ui/panel.h"

namespace Hexterminate::UI
{

Panel::Panel( const std::string& name )
    : Element( name )
{
    m_BorderLeft = m_BorderRight = m_BorderBottom = m_BorderTop = false;

    GetPanel()->SetColour( 0.0f, 0.0f, 0.0f, 0.7f );
    GetPanel()->SetBorderColour( 0.22f, 0.22f, 0.22f, 1.0f );
}

void Panel::SaveProperties( json& properties )
{
    Element::SaveProperties( properties );

    properties[ "border" ] = {
        { "left", m_BorderLeft },
        { "right", m_BorderRight },
        { "top", m_BorderTop },
        { "bottom", m_BorderBottom }
    };
}

void Panel::LoadProperties( const json& properties )
{
    Element::LoadProperties( properties );

    if ( properties.contains( "border" ) )
    {
        const json& border = properties[ "border" ];
        m_BorderLeft = border[ "left" ].get<bool>();
        m_BorderRight = border[ "right" ].get<bool>();
        m_BorderTop = border[ "top" ].get<bool>();
        m_BorderBottom = border[ "bottom" ].get<bool>();
        ApplyBorder();
    }
}

void Panel::RenderProperties()
{
    Element::RenderProperties();

    if ( ImGui::CollapsingHeader( "Panel", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        ImGui::Text( "%s", "Border:" );
        ImGui::Checkbox( "Left", &m_BorderLeft );
        ImGui::SameLine();
        ImGui::Checkbox( "Right", &m_BorderRight );
        ImGui::SameLine();
        ImGui::Checkbox( "Top", &m_BorderTop );
        ImGui::SameLine();
        ImGui::Checkbox( "Bottom", &m_BorderBottom );
        ApplyBorder();
    }
}

void Panel::SetColour( float r, float g, float b, float a )
{
    GetPanel()->SetColour( r, g, b, a );
}

void Panel::SetBorders( bool left, bool right, bool top, bool bottom )
{
    m_BorderLeft = left;
    m_BorderRight = right;
    m_BorderTop = top;
    m_BorderBottom = bottom;
    ApplyBorder();
}

void Panel::SetBorderColour( float r, float g, float b, float a )
{
    GetPanel()->SetBorderColour( r, g, b, a );
}

void Panel::ApplyBorder()
{
    using namespace Genesis::Gui;
    char border = PANEL_BORDER_NONE;
    if ( m_BorderLeft )
    {
        border |= PANEL_BORDER_LEFT;
    }
    if ( m_BorderRight )
    {
        border |= PANEL_BORDER_RIGHT;
    }
    if ( m_BorderTop )
    {
        border |= PANEL_BORDER_TOP;
    }
    if ( m_BorderBottom )
    {
        border |= PANEL_BORDER_BOTTOM;
    }
    GetPanel()->SetBorderMode( border );
}

} // namespace Hexterminate::UI
