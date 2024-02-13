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

namespace Genesis
{
class ResourceSound;
}

namespace Genesis::Gui
{
class Checkbox;
}

namespace Hexterminate::UI
{

using CheckboxPressedCallback = std::function<void( bool )>;

class Checkbox : public Element
{
public:
    Checkbox( const std::string& name, CheckboxPressedCallback pCallback = nullptr );

    virtual void Update() override;
    virtual void RenderProperties() override;
    virtual void SetSize( int width, int height ) override;

    bool IsChecked() const;
    void SetChecked( bool state );

protected:
    virtual void SaveProperties( json& properties ) override;
    virtual void LoadProperties( const json& properties ) override;

private:
    Genesis::Gui::Checkbox* m_pCheckbox;
    TextSharedPtr m_pText;
    Genesis::ResourceSound* m_pHoverSFX;
    Genesis::ResourceSound* m_pClickSFX;
    bool m_WasHovered;
};

} // namespace Hexterminate::UI
