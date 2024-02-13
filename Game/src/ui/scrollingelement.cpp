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

#include <numeric>

#include <gui/gui.h>
#include <imgui/imgui.h>
#include <math/misc.h>

#include "ui/image.h"
#include "ui/panel.h"
#include "ui/scrollingelement.h"

namespace Hexterminate::UI
{

ScrollingElement::ScrollingElement( const std::string& name )
    : Element( name )
    , m_GrabAndHold( false )
    , m_CurrentOffset( 0.0f )
    , m_OffsetLimit( 0 )
{
    using namespace Genesis;

    m_MouseWheelEventToken = FrameWork::GetInputManager()->AddMouseWheelCallback( std::bind( &ScrollingElement::OnMouseWheelCallback, this, std::placeholders::_1, std::placeholders::_2 ) );

    m_pScrollingArea = std::make_shared<UI::Element>( "Scrolling area" );
    m_pScrollingArea->SetEditable( false );
    Add( m_pScrollingArea );

    m_pScrollbarBackground = std::make_shared<UI::Panel>( "Scrollbar background" );
    m_pScrollbarBackground->SetEditable( false );
    m_pScrollbarBackground->SetColour( 0.0f, 0.0f, 0.0f, 0.3f );
    Add( m_pScrollbarBackground );

    m_pScrollbar = std::make_shared<UI::Panel>( "Scrollbar" );
    m_pScrollbar->SetEditable( false );
    m_pScrollbar->SetSize( GetScrollbarWidth(), GetScrollbarHeight() );
    m_pScrollbar->SetColour( 1.0f, 1.0f, 1.0f, 0.3f );
    m_pScrollbarBackground->Add( m_pScrollbar );

    m_pScrollUp = std::make_shared<UI::Image>( "Scroll up image", "data/ui/icons/arrowup.png" );
    m_pScrollUp->SetEditable( false );
    m_pScrollbarBackground->Add( m_pScrollUp );

    m_pScrollDown = std::make_shared<UI::Image>( "Scroll down image", "data/ui/icons/arrowdown.png" );
    m_pScrollDown->SetEditable( false );
    m_pScrollbarBackground->Add( m_pScrollDown );

    SetSize( 128, 128 );
}

ScrollingElement::~ScrollingElement()
{
    Genesis::FrameWork::GetInputManager()->RemoveMouseWheelCallback( m_MouseWheelEventToken );
}

void ScrollingElement::Update()
{
    Element::Update();

    if ( GetPanel()->IsVisible() == false )
    {
        return;
    }

    using namespace Genesis;

    UpdateScrollingAreaHeight();

    const float scrollingSpeed = 160.0f;

    const bool insideScrollbar = m_pScrollbar->IsHovered();
    m_pScrollbar->SetColour( 1.0f, 1.0f, 1.0f, ( m_GrabAndHold || insideScrollbar ) ? 0.6f : 0.3f );

    float offset = 0.0f;
    InputManager* pInputManager = FrameWork::GetInputManager();
    if ( pInputManager->IsMouseButtonPressed( MouseButton::Left ) )
    {
        if ( insideScrollbar && IsScrollingEnabled() )
        {
            m_GrabAndHold = true;
        }

        if ( m_GrabAndHold && IsScrollingEnabled() )
        {
            const float scrollbarBackgroundAbsY = m_pScrollbarBackground->GetPanel()->GetPositionAbsolute().y;
            const float min = scrollbarBackgroundAbsY + (float)m_pScrollbar->GetHeight() * 0.5f;
            const float max = scrollbarBackgroundAbsY + (float)m_pScrollbarBackground->GetHeight() - m_pScrollbar->GetHeight() * 0.5f;
            float mousePos = pInputManager->GetMousePosition().y;
            if ( mousePos < min )
                mousePos = min;
            else if ( mousePos > max )
                mousePos = max;
            const float ratio = ( mousePos - min ) / ( max - min );
            m_CurrentOffset = (float)-m_OffsetLimit * ratio;
        }
        else
        {
            if ( m_pScrollUp->IsHovered() )
            {
                offset += scrollingSpeed * 0.033f; /*delta*/
                ;
            }
            else if ( m_pScrollDown->IsHovered() )
            {
                offset -= scrollingSpeed * 0.033f; /*delta*/
                ;
            }
        }
    }
    else if ( m_GrabAndHold == true )
    {
        m_GrabAndHold = false;
    }

    if ( m_GrabAndHold == true || fabsf( offset ) > 0.0f )
    {
        m_CurrentOffset += offset;
        if ( m_CurrentOffset > 0.0f )
            m_CurrentOffset = 0.0f;
        else if ( m_CurrentOffset < (float)-m_OffsetLimit )
            m_CurrentOffset = (float)-m_OffsetLimit;

        // Offset is rounded up to prevent shimmering of child elements
        m_pScrollingArea->GetPanel()->SetPosition( m_pScrollingArea->GetPanel()->GetPosition().x, ceilf( m_CurrentOffset ) );
        Genesis::FrameWork::GetLogger()->LogInfo( "grab and hold offset: %f, limit %d", m_CurrentOffset, m_OffsetLimit );
    }

    UpdateScrollbarPosition();
}

void ScrollingElement::UpdateScrollingAreaHeight()
{
    int totalHeight = 0;
    const Genesis::Gui::GuiElementList& children = m_pScrollingArea->GetPanel()->GetChildren();
    for ( auto& pElement : children )
    {
        if ( pElement->IsVisible() )
        {
            int height = static_cast<int>( ceilf( pElement->GetPosition().y ) ) + pElement->GetHeight();
            if ( totalHeight < height )
            {
                totalHeight = height;
            }
        }
    }

    if ( totalHeight < GetHeight() )
    {
        totalHeight = GetHeight();
    }

    m_pScrollingArea->SetSize( m_pScrollingArea->GetWidth(), totalHeight );
    m_OffsetLimit = std::max( 0, totalHeight - GetHeight() );
    m_pScrollbar->Show( IsScrollingEnabled() );
}

void ScrollingElement::SetSize( int width, int height )
{
    Element::SetSize( width, height );

    m_pScrollingArea->SetSize( width - GetScrollbarWidth(), height );
    m_pScrollbarBackground->SetPosition( width - GetScrollbarWidth(), 0 );
    m_pScrollbarBackground->SetSize( GetScrollbarWidth(), height );
    m_pScrollbar->SetPosition( 0, m_pScrollUp->GetHeight() );
    m_pScrollDown->SetPosition( 0, height - m_pScrollDown->GetHeight() );
}

void ScrollingElement::UpdateScrollbarPosition()
{
    if ( m_OffsetLimit <= 0 )
    {
        return;
    }

    const float ratio = -m_CurrentOffset / static_cast<float>( m_OffsetLimit );
    const int localPosition = static_cast<int>( ratio * ( m_pScrollbarBackground->GetHeight() - m_pScrollbar->GetHeight() - m_pScrollUp->GetHeight() - m_pScrollDown->GetHeight() ) );

    int x, y;
    m_pScrollbar->GetPosition( x, y );
    m_pScrollbar->SetPosition( x, m_pScrollUp->GetHeight() + localPosition );
}

void ScrollingElement::OnMouseWheelCallback( float x, float y )
{
    // Don't do anything if we aren't moving the mouse wheel vertically.
    if ( fabsf( y ) < std::numeric_limits<float>::epsilon() )
    {
        return;
    }
    else if ( m_GrabAndHold || IsVisible() == false )
    {
        return;
    }
    else if ( m_pScrollingArea->IsHovered() == false && m_pScrollbarBackground->IsHovered() == false )
    {
        return;
    }

    m_CurrentOffset = gClamp( m_CurrentOffset + 32.0f * y, static_cast<float>( -m_OffsetLimit ), 0.0f );
    m_pScrollingArea->GetPanel()->SetPosition( m_pScrollingArea->GetPanel()->GetPosition().x, ceilf( m_CurrentOffset ) );
}

} // namespace Hexterminate::UI
