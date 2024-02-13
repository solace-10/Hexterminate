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

#include <functional>

#include <inputmanager.h>

#include "ui/element.h"
#include "ui/types.fwd.h"

namespace Hexterminate::UI
{

class Slider : public Element
{
public:
    using OnValueChanged = std::function<void( float )>;

    Slider( const std::string& name, OnValueChanged onValueChanged = nullptr );
    virtual ~Slider() override;
    virtual void Update() override;
    virtual void SetSize( int width, int height ) override;

    void SetValue( float value );

private:
    void UpdateHandlePosition();
    void OnLeftClickPressed();
    void OnLeftClickReleased();

    PanelSharedPtr m_pBar;
    PanelSharedPtr m_pHandle;
    bool m_Grabbed;
    Genesis::InputCallbackToken m_LeftClickPressedToken;
    Genesis::InputCallbackToken m_LeftClickReleasedToken;
    OnValueChanged m_OnValueChanged;
};

} // namespace Hexterminate::UI
