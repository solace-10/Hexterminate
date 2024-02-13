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

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <gui/gui.h>

#include "ui/inputarea.h"

namespace Hexterminate::UI
{

static const char* sInputAreaPropertyDefaultText = "default_text";

InputArea::InputArea( const std::string& name )
    : Panel( name )
{
    using namespace Genesis;

    GetPanel()->SetBorderMode( Gui::PANEL_BORDER_ALL );

    m_pInputArea = new Gui::InputArea();
    m_pInputArea->SetFont( (ResourceFont*)FrameWork::GetResourceManager()->GetResource( "data/fonts/kimberley18light.fnt" ) );
    m_pInputArea->SetFilter( Gui::InputAreaFilter::ACCEPT_LETTERS | Gui::InputAreaFilter::ACCEPT_SPACES );
    m_pInputArea->SetColour( 0.0f, 0.0f, 0.0f, 0.5f );
    m_pInputArea->SetHoverColour( 1.0f, 1.0f, 1.0f, 0.15f );
    m_pInputArea->SetBorderMode( Gui::PANEL_BORDER_NONE );
    GetPanel()->AddElement( m_pInputArea );
}

void InputArea::Update()
{
    Panel::Update();
    m_pInputArea->SetAcceptsInput( IsAcceptingInput() );
}

void InputArea::SetSize( int width, int height )
{
    Panel::SetSize( width, height );
    m_pInputArea->SetSize( width, height );
}

const std::string& InputArea::GetText() const
{
    return m_pInputArea->GetText();
}

void InputArea::SetText( const std::string& text )
{
    m_pInputArea->SetText( text );
}

void InputArea::SaveProperties( json& properties )
{
    Element::SaveProperties( properties );

    properties[ sInputAreaPropertyDefaultText ] = m_DefaultText;
}

void InputArea::LoadProperties( const json& properties )
{
    Element::LoadProperties( properties );

    if ( properties.contains( sInputAreaPropertyDefaultText ) )
    {
        m_DefaultText = properties[ sInputAreaPropertyDefaultText ].get<std::string>();
        m_pInputArea->SetText( m_DefaultText );
    }
}

void InputArea::RenderProperties()
{
    Element::RenderProperties();

    if ( ImGui::CollapsingHeader( "Input Area", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        ImGui::InputText( "Default text", &m_DefaultText );
    }
}

} // namespace Hexterminate::UI
