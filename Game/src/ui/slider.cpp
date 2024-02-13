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

#include <gui/gui.h>
#include <math/misc.h>

#include "ui/panel.h"
#include "ui/slider.h"

namespace Hexterminate::UI
{

static const int sSliderHandleWidth = 8;
static const int sSliderBarHeight = 4;

Slider::Slider( const std::string& name, OnValueChanged onValueChanged /* = nullptr */ )
    : Element( name )
    , m_Grabbed( false )
    , m_OnValueChanged( onValueChanged )
{
    using namespace Genesis;

    m_pBar = std::make_shared<Panel>( "Bar" );
    m_pBar->SetEditable( false );
    m_pBar->SetColour( 0.5f, 0.5f, 0.5f, 1.0f );
    Add( m_pBar );

    m_pHandle = std::make_shared<Panel>( "Handle" );
    m_pHandle->SetEditable( false );
    m_pHandle->SetColour( 0.309f, 0.639f, 0.690f, 1.0f );
    Add( m_pHandle );

    SetSize( 256, 16 );

    m_LeftClickPressedToken = FrameWork::GetInputManager()->AddMouseCallback( [ this ]() { OnLeftClickPressed(); }, MouseButton::Left, ButtonState::Pressed );
    m_LeftClickReleasedToken = FrameWork::GetInputManager()->AddMouseCallback( [ this ]() { OnLeftClickReleased(); }, MouseButton::Left, ButtonState::Released );
}

Slider::~Slider()
{
    Genesis::InputManager* pInputManager = Genesis::FrameWork::GetInputManager();
    pInputManager->RemoveMouseCallback( m_LeftClickPressedToken );
    pInputManager->RemoveMouseCallback( m_LeftClickReleasedToken );
}

void Slider::Update()
{
    Element::Update();

    if ( m_pHandle->IsHovered() || m_Grabbed )
    {
        m_pHandle->SetColour( 0.0f, 1.0f, 1.0f, 1.0f );
    }
    else
    {
        m_pHandle->SetColour( 0.309f, 0.639f, 0.690f, 1.0f );
    }

    if ( m_Grabbed )
    {
        UpdateHandlePosition();
    }
}

void Slider::SetSize( int width, int height )
{
    Element::SetSize( width, height );

    m_pBar->SetSize( width, sSliderBarHeight );
    m_pBar->SetPosition( 0, height / 2 - sSliderBarHeight / 2 );

    m_pHandle->SetSize( sSliderHandleWidth, height );
    m_pHandle->SetPosition( sSliderHandleWidth / 2, 0 );
}

// Expects a value between 0 and 1.
void Slider::SetValue( float value )
{
    value = gClamp( value, 0.0f, 1.0f );

    int xBar, yBar;
    m_pBar->GetPosition( xBar, yBar );

    int xHandle = static_cast<int>( xBar + 4 + value * ( m_pBar->GetWidth() - 8 - sSliderHandleWidth ) );
    m_pHandle->SetPosition( xHandle, 0 );

    if ( m_OnValueChanged != nullptr )
    {
        m_OnValueChanged( value );
    }
}

void Slider::UpdateHandlePosition()
{
    const float xMin = m_pBar->GetPanel()->GetPositionAbsolute().x + 4;
    const float xMax = xMin + m_pBar->GetWidth() - 8 - sSliderHandleWidth;
    const float xMouse = Genesis::FrameWork::GetInputManager()->GetMousePosition().x;
    const float value = gClamp( ( xMouse - xMin ) / ( xMax - xMin ), 0.0f, 1.0f );
    SetValue( value );
}

void Slider::OnLeftClickPressed()
{
    if ( m_pHandle->IsHovered() && IsAcceptingInput() )
    {
        m_Grabbed = true;
    }
}

void Slider::OnLeftClickReleased()
{
    m_Grabbed = false;
}

} // namespace Hexterminate::UI
