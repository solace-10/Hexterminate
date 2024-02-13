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

#pragma once

#include "gui.h"

namespace Genesis
{
namespace Gui
{

    class ScrollingElement : public GuiElement
    {
    public:
        ScrollingElement();

        void Init();

        virtual void AddElement( GuiElement* pElement ) override;
        virtual void RemoveElement( GuiElement* pElement ) override;

        virtual void Update( float delta ) override;
        void UpdateScrollingAreaHeight();

        int GetScrollbarWidth() const;
        void Reset();

    private:
        void UpdateScrollbarPosition();
        bool IsMouseInside( GuiElement* pElement ) const;
        bool IsScrollingEnabled() const;

        GuiElement* m_pScrollingArea;
        Panel* m_pScrollbarBackground;
        Panel* m_pScrollbar;
        Image* m_pScrollUp;
        Image* m_pScrollDown;
        bool m_GrabAndHold;
        float m_CurrentOffset;
        int m_OffsetLimit;
    };

    inline int ScrollingElement::GetScrollbarWidth() const
    {
        return 16;
    }

    inline bool ScrollingElement::IsScrollingEnabled() const
    {
        return m_OffsetLimit != 0;
    }
}
}
