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
#include "ui/types.fwd.h"

namespace Hexterminate::UI
{

class Window : public Element
{
public:
    Window( const std::string& name, bool canBeClosed = false );

    virtual void SetSize( int width, int height ) override;
    virtual void Reset() {}
    virtual void Show( bool state ) override;
    virtual void RenderProperties() override;

    void SetTitle( const std::string& text );

protected:
    virtual void SaveProperties( json& properties ) override;
    virtual void LoadProperties( const json& properties ) override;

    Panel* GetContentPanel();

private:
    void CenterWindowHorizontally();
    void CenterWindowVertically();

    PanelSharedPtr m_pTitlePanel;
    TextSharedPtr m_pTitleText;
    PanelSharedPtr m_pContentPanel;
    ButtonSharedPtr m_pCloseButton;

    bool m_CenterHorizontally;
    bool m_CenterVertically;
    bool m_CanBeClosed;
};

inline Panel* Window::GetContentPanel()
{
    return m_pContentPanel.get();
}

} // namespace Hexterminate::UI
