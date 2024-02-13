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
#include "ui/radiobutton.h"
#include "ui/text.h"

namespace Hexterminate::UI
{

static const char* sRadioButtonPropertyChecked = "checked";
static const char* sRadioButtonPropertyEnabled = "enabled";

RadioButton::RadioButton( const std::string& name, const std::string& groupName, RadioButtonPressedCallback pCallback )
    : Element( name )
    , m_WasHovered( false )
{
    using namespace Genesis;

    m_pClickSFX = Genesis::Gui::LoadSFX( "data/sfx/beep.wav" );
    m_pHoverSFX = Genesis::Gui::LoadSFX( "data/sfx/hover.wav" );

    m_pRadioButton = new Gui::RadioButton( 0, 0, EVA_FONT, "", groupName, false,
        [ this, pCallback ]() {
            Genesis::Gui::PlaySFX( m_pClickSFX );

            if ( pCallback != nullptr )
            {
                pCallback();
            }
        } );
    m_pRadioButton->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
    m_pRadioButton->SetColour( EVA_BUTTON_COLOUR_BACKGROUND );
    m_pRadioButton->SetBulletColour( EVA_CHECKBOX_BULLET_COLOUR );

    GetPanel()->AddElement( m_pRadioButton );

    m_pText = std::make_shared<Text>( "Text" );
    Add( m_pText );
}

void RadioButton::Update()
{
    Element::Update();
    const bool acceptingInput = IsAcceptingInput();
    m_pRadioButton->SetAcceptsInput( acceptingInput );

    if ( acceptingInput && m_pRadioButton->IsMouseInside() && !m_WasHovered )
    {
        Genesis::Gui::PlaySFX( m_pHoverSFX );
        m_WasHovered = true;
    }
    else if ( !acceptingInput || ( !m_pRadioButton->IsMouseInside() && m_WasHovered ) )
    {
        m_WasHovered = false;
    }
}

void RadioButton::SaveProperties( json& properties )
{
    Element::SaveProperties( properties );

    properties[ sRadioButtonPropertyChecked ] = m_pRadioButton->IsChecked();
    properties[ sRadioButtonPropertyEnabled ] = m_pRadioButton->IsEnabled();
}

void RadioButton::LoadProperties( const json& properties )
{
    Element::LoadProperties( properties );

    if ( properties.contains( sRadioButtonPropertyChecked ) )
    {
        m_pRadioButton->SetChecked( properties[ sRadioButtonPropertyChecked ].get<bool>() );
    }

    if ( properties.contains( sRadioButtonPropertyEnabled ) )
    {
        m_pRadioButton->Enable( properties[ sRadioButtonPropertyEnabled ].get<bool>() );
    }
}

void RadioButton::RenderProperties()
{
    Element::RenderProperties();

    if ( ImGui::CollapsingHeader( "Radio Button", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        bool isChecked = m_pRadioButton->IsChecked();
        if ( ImGui::Checkbox( "Checked", &isChecked ) )
        {
            m_pRadioButton->SetChecked( isChecked );
        }

        bool isEnabled = m_pRadioButton->IsEnabled();
        if ( ImGui::Checkbox( "Enabled", &isEnabled ) )
        {
            m_pRadioButton->Enable( isEnabled );
        }
    }
}

} // namespace Hexterminate::UI
