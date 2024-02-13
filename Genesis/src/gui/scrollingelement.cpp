// Copyright 2015 Pedro Nunes
//
// This file is part of Genesis.
//
// Genesis is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Genesis is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Genesis. If not, see <http://www.gnu.org/licenses/>.

#include <genesis.h>
#include <inputmanager.h>

#include "scrollingelement.h"

namespace Genesis
{
namespace Gui
{

    ScrollingElement::ScrollingElement()
        : m_pScrollingArea( nullptr )
        , m_pScrollbarBackground( nullptr )
        , m_pScrollbar( nullptr )
        , m_pScrollUp( nullptr )
        , m_pScrollDown( nullptr )
        , m_GrabAndHold( false )
        , m_CurrentOffset( 0.0f )
        , m_OffsetLimit( 0 )
    {
    }

    void ScrollingElement::Init()
    {
        const int scrollbarSize = GetScrollbarWidth();

        // Due to AddElement() being overriden, we need to call GuiElement::AddElement directly to avoid
        // having objects having themselves as parents.

        m_pScrollingArea = new GuiElement();
        m_pScrollingArea->SetSize( GetWidth() - scrollbarSize, GetHeight() );
        GuiElement::AddElement( m_pScrollingArea );

        m_pScrollbarBackground = new Panel();
        m_pScrollbarBackground->SetPosition( GetWidth() - scrollbarSize, scrollbarSize );
        m_pScrollbarBackground->SetSize( scrollbarSize, GetHeight() - scrollbarSize * 2 );
        m_pScrollbarBackground->SetColour( 0.0f, 0.0f, 0.0f, 0.3f );
        GuiElement::AddElement( m_pScrollbarBackground );

        m_pScrollbar = new Panel();
        m_pScrollbar->SetPosition( GetWidth() - scrollbarSize, scrollbarSize );
        m_pScrollbar->SetSize( scrollbarSize, 64 );
        m_pScrollbar->SetColour( 1.0f, 1.0f, 1.0f, 0.3f );
        GuiElement::AddElement( m_pScrollbar );

        ResourceImage* pUpImage = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/ui/icons/arrowup.png" );
        m_pScrollUp = new Image();
        m_pScrollUp->SetPosition( GetWidth() - scrollbarSize, 0 );
        m_pScrollUp->SetSize( scrollbarSize, scrollbarSize );
        m_pScrollUp->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
        m_pScrollUp->SetTexture( pUpImage );
        GuiElement::AddElement( m_pScrollUp );

        ResourceImage* pDownImage = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/ui/icons/arrowdown.png" );
        m_pScrollDown = new Image();
        m_pScrollDown->SetPosition( GetWidth() - scrollbarSize, GetHeight() - scrollbarSize );
        m_pScrollDown->SetSize( scrollbarSize, scrollbarSize );
        m_pScrollDown->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
        m_pScrollDown->SetTexture( pDownImage );
        GuiElement::AddElement( m_pScrollDown );
    }

    // Elements are added and removed directly into the scrolling area, so that the implementation details of
    // the scrolling element are invisible.
    void ScrollingElement::AddElement( GuiElement* pElement )
    {
        SDL_assert( m_pScrollingArea != nullptr );

        m_pScrollingArea->AddElement( pElement );
        UpdateScrollingAreaHeight();
    }

    void ScrollingElement::RemoveElement( GuiElement* pElement )
    {
        SDL_assert( m_pScrollingArea != nullptr );

        m_pScrollingArea->RemoveElement( pElement );
        UpdateScrollingAreaHeight();
    }

    void ScrollingElement::UpdateScrollingAreaHeight()
    {
        SDL_assert( m_pScrollingArea != nullptr );

        int totalHeight = 0;
        const GuiElementList& children = m_pScrollingArea->GetChildren();
        for ( auto& pElement : children )
        {
            if ( pElement->IsVisible() == false )
                continue;

            int height = static_cast<int>( ceilf( pElement->GetPosition().y ) ) + pElement->GetHeight();
            if ( totalHeight < height )
                totalHeight = height;
        }

        if ( totalHeight < GetHeight() )
            totalHeight = GetHeight();

        m_pScrollingArea->SetHeight( (float)totalHeight );

        m_OffsetLimit = std::max( 0, totalHeight - GetHeight() );

        m_pScrollbar->Show( IsScrollingEnabled() );
    }

    bool ScrollingElement::IsMouseInside( GuiElement* pElement ) const
    {
        const glm::vec2& mousePos = FrameWork::GetInputManager()->GetMousePosition();
        const glm::vec2& buttonAbsPos = pElement->GetPositionAbsolute();
        const glm::vec2& buttonSize = pElement->GetSize();
        return ( mousePos.x > buttonAbsPos.x && mousePos.x < buttonAbsPos.x + buttonSize.x && mousePos.y > buttonAbsPos.y && mousePos.y < buttonAbsPos.y + buttonSize.y );
    }

    void ScrollingElement::Update( float delta )
    {
        GuiElement::Update( delta );

        const float scrollingSpeed = 160.0f;

        const bool insideScrollbar = IsMouseInside( m_pScrollbar );
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
                const float min = m_pScrollbarBackground->GetPositionAbsolute().y + (float)m_pScrollbar->GetHeight() * 0.5f;
                const float max = m_pScrollbarBackground->GetPositionAbsolute().y + (float)m_pScrollbarBackground->GetHeight() - m_pScrollbar->GetHeight() * 0.5f;
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
                if ( IsMouseInside( m_pScrollUp ) )
                {
                    offset += scrollingSpeed * delta;
                }
                else if ( IsMouseInside( m_pScrollDown ) )
                {
                    offset -= scrollingSpeed * delta;
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
            m_pScrollingArea->SetPosition( m_pScrollingArea->GetPosition().x, ceilf( m_CurrentOffset ) );
        }

        UpdateScrollbarPosition();
    }

    void ScrollingElement::UpdateScrollbarPosition()
    {
        if ( m_OffsetLimit <= 0 )
            return;

        const float ratio = -m_CurrentOffset / static_cast<float>( m_OffsetLimit );
        const float localPosition = ratio * ( m_pScrollbarBackground->GetHeight() - m_pScrollbar->GetHeight() );
        m_pScrollbar->SetPosition( m_pScrollbar->GetPosition().x, m_pScrollbarBackground->GetPosition().y + localPosition );
    }

    void ScrollingElement::Reset()
    {
        m_CurrentOffset = 0.0f;
        UpdateScrollbarPosition();
        m_pScrollingArea->SetPosition( m_pScrollingArea->GetPosition().x, 0.0f );
    }
}
}
