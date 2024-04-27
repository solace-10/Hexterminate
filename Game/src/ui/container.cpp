// Copyright 2024 Pedro Nunes
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
#include <magic_enum.hpp>

#include "ui/container.h"

namespace Hexterminate::UI
{

Container::Container( const std::string& name )
    : Element( name )
    , m_Direction( Direction::Row )
    , m_Dirty( false )
{
    SetFlags( ElementFlags_DynamicSize );
}

void Container::SaveProperties( json& properties )
{
    Element::SaveProperties( properties );

    properties[ "direction" ] = magic_enum::enum_name( m_Direction );
}

void Container::LoadProperties( const json& properties )
{
    Element::LoadProperties( properties );

    if ( properties.contains( "direction" ) )
    {
        auto direction = magic_enum::enum_cast<Direction>( properties[ "direction" ].get<std::string>() );
        if ( direction.has_value() )
        {
            m_Direction = direction.value();
        }
    }
}

void Container::Update()
{
    Element::Update();

    if ( !m_Dirty && GetParent() )
    {
        if ( m_CachedParentSize != GetParent()->GetSize() )
        {
            m_CachedParentSize = GetParent()->GetSize();
            m_Dirty = true;
        }
    }

    if ( m_Dirty )
    {
        RebuildContents();
    }
}

void Container::RenderProperties()
{
    Element::RenderProperties();

    if ( ImGui::CollapsingHeader( "Container", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        int direction = static_cast<int>( m_Direction );
        if ( ImGui::Combo( "Direction", &direction, "Row\0Row reverse\0Column\0Column reverse\0\0" ) )
        {
            m_Direction = static_cast<Direction>( direction );
            m_Dirty = true;
        }
    }
}

void Container::Add( ElementSharedPtr pElement )
{
    Element::Add( pElement );
    pElement->SetFlags( ElementFlags_DynamicPosition );
    m_Dirty = true;
}

void Container::Remove( ElementSharedPtr pElement )
{
    Element::Remove( pElement );
    m_Dirty = true;
}

void Container::RebuildContents()
{
    SDL_assert( GetParent() );

    SetSize( GetParent()->GetSize() );

    static const glm::ivec2 initialOffsetMultiplier[ 4 ] = {
        { 0, 0 },
        { 1, 0 },
        { 0, 0 },
        { 0, 1 }
    };

    static const glm::ivec2 directionMultiplier[ 4 ] = {
        { 1, 0 },
        { -1, 0 },
        { 0, 1 },
        { 0, -1 }
    };

    const int directionIndex = static_cast<int>( m_Direction );
    glm::ivec2 offset( GetSize() );
    offset *= initialOffsetMultiplier[ directionIndex ];

    for ( auto& pChildElement : GetChildren() )
    {
        offset += pChildElement->GetSize() * initialOffsetMultiplier[ directionIndex ] * directionMultiplier[ directionIndex ];
        pChildElement->SetPosition( offset );
    }

    m_Dirty = false;
}

} // namespace Hexterminate::UI
