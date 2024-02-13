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

#pragma once

#include "ui/element.h"

namespace Hexterminate::UI
{

class ScrollingElement : public Element
{
public:
    ScrollingElement( const std::string& name );
    virtual ~ScrollingElement() override;

    virtual void Update() override;
    virtual void SetSize( int width, int height ) override;

    UI::Element* GetScrollingArea();

private:
    int GetScrollbarHeight() const;
    int GetScrollbarWidth() const;

    void UpdateScrollingAreaHeight();
    void UpdateScrollbarPosition();
    bool IsScrollingEnabled() const;
    void OnMouseWheelCallback( float x, float y );

    UI::ElementSharedPtr m_pScrollingArea;
    UI::PanelSharedPtr m_pScrollbarBackground;
    UI::PanelSharedPtr m_pScrollbar;
    UI::ImageSharedPtr m_pScrollUp;
    UI::ImageSharedPtr m_pScrollDown;
    bool m_GrabAndHold;
    float m_CurrentOffset;
    int m_OffsetLimit;

    Genesis::InputCallbackToken m_MouseWheelEventToken;
};

inline UI::Element* ScrollingElement::GetScrollingArea()
{
    return m_pScrollingArea.get();
}

inline int ScrollingElement::GetScrollbarHeight() const
{
    return 64;
}

inline int ScrollingElement::GetScrollbarWidth() const
{
    return 16;
}

inline bool ScrollingElement::IsScrollingEnabled() const
{
    return m_OffsetLimit != 0;
}

} // namespace Hexterminate::UI
