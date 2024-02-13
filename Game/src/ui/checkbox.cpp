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

#include "menus/eva.h"
#include "ui/checkbox.h"
#include "ui/text.h"

namespace Hexterminate::UI
{

static const char* sCheckboxPropertyChecked = "checked";

Checkbox::Checkbox( const std::string& name, CheckboxPressedCallback pCallback /* = nullptr */ )
    : Element( name )
    , m_WasHovered( false )
{
    using namespace Genesis;

    m_pClickSFX = Genesis::Gui::LoadSFX( "data/sfx/beep.wav" );
    m_pHoverSFX = Genesis::Gui::LoadSFX( "data/sfx/hover.wav" );

    m_pCheckbox = new Gui::Checkbox( 0, 0, EVA_FONT, "", false,
        [ this, pCallback ]( bool checked ) {
            Genesis::Gui::PlaySFX( m_pClickSFX );

            if ( pCallback != nullptr )
            {
                pCallback( checked );
            }
        } );

    m_pCheckbox->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
    m_pCheckbox->SetColour( EVA_BUTTON_COLOUR_BACKGROUND );
    m_pCheckbox->SetBulletColour( EVA_CHECKBOX_BULLET_COLOUR );
    GetPanel()->AddElement( m_pCheckbox );

    m_pText = std::make_shared<Text>( "Text" );
    Add( m_pText );
}

void Checkbox::Update()
{
    Element::Update();
    const bool acceptingInput = IsAcceptingInput();
    m_pCheckbox->SetAcceptsInput( acceptingInput );

    if ( acceptingInput && m_pCheckbox->IsMouseInside() && !m_WasHovered )
    {
        Genesis::Gui::PlaySFX( m_pHoverSFX );
        m_WasHovered = true;
    }
    else if ( !acceptingInput || ( !m_pCheckbox->IsMouseInside() && m_WasHovered ) )
    {
        m_WasHovered = false;
    }
}

void Checkbox::RenderProperties()
{
    Element::RenderProperties();

    if ( ImGui::CollapsingHeader( "Checkbox", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        bool isChecked = m_pCheckbox->IsPressed();
        if ( ImGui::Checkbox( "Checked", &isChecked ) )
        {
            m_pCheckbox->SetPressed( isChecked );
        }
    }
}

void Checkbox::SetSize( int width, int height )
{
    Element::SetSize( width, height );
    m_pCheckbox->SetSize( 16, 16 );
    m_pCheckbox->SetPosition( 0, height / 2 - 8 );
}

bool Checkbox::IsChecked() const
{
    return m_pCheckbox->IsPressed();
}

void Checkbox::SetChecked( bool state )
{
    m_pCheckbox->SetPressed( state );
}

void Checkbox::SaveProperties( json& properties )
{
    Element::SaveProperties( properties );

    properties[ sCheckboxPropertyChecked ] = m_pCheckbox->IsPressed();
}

void Checkbox::LoadProperties( const json& properties )
{
    Element::LoadProperties( properties );

    if ( properties.contains( sCheckboxPropertyChecked ) )
    {
        m_pCheckbox->SetPressed( properties[ sCheckboxPropertyChecked ].get<bool>() );
    }
}

} // namespace Hexterminate::UI
