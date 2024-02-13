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

#include <any>
#include <array>
#include <filesystem>
#include <functional>
#include <string>

#include <inputmanager.h>

#include "ui/element.h"
#include "ui/itoggleable.h"

namespace Genesis
{
class ResourceImage;
class ResourceSound;

namespace Gui
{
    class Image;
    class Text;
} // namespace Gui
} // namespace Genesis.

namespace Hexterminate::UI
{

using ButtonPressedCallback = std::function<void( const std::any& userData )>;

class Button : public Element, public IToggleable
{
public:
    Button( const std::string& name, ButtonPressedCallback onPressed, std::any userData = {}, ToggleGroupWeakPtr pToggleGroup = {} );
    virtual ~Button() override;

    virtual void Update() override;
    virtual void RenderProperties() override;
    virtual bool IsResizeable() const override;
    virtual void Toggle( bool state ) override; // From IToggleable

    void Enable( bool state );
    void SetUserData( const std::any& userdata );
    void SetDefaultImage( const std::string& path );
    void SetHoveredImage( const std::string& path );
    void SetPressedImage( const std::string& path );
    void SetText( const std::string& text, bool centerHorizontally = true, bool centerVertically = true );
    void SetFont( const std::string& fontName );
    void SetOnPressed( ButtonPressedCallback onPressed );

protected:
    enum class State
    {
        Unselected,
        Hovered,
        Selected,
        Disabled
    };

    virtual void SaveProperties( json& properties ) override;
    virtual void LoadProperties( const json& properties ) override;
    void LoadResources();
    void CenterText( bool horizontally = true, bool vertically = true );
    void OnLeftClickPressed();

    Genesis::Gui::Image* m_pImage;
    Genesis::ResourceImage* m_pDefault;
    Genesis::ResourceImage* m_pHovered;
    Genesis::ResourceImage* m_pPressed;

    std::string m_DefaultPath;
    std::string m_HoveredPath;
    std::string m_PressedPath;

    Genesis::Gui::Text* m_pText;
    std::string m_Label;

    ButtonPressedCallback m_OnPressed;
    Genesis::InputCallbackToken m_LeftClickPressedToken;
    std::any m_UserData;
    ToggleGroupWeakPtr m_pToggleGroup;
    State m_State;

    Genesis::ResourceSound* m_pHoverSFX;
    Genesis::ResourceSound* m_pClickSFX;
};

inline bool Button::IsResizeable() const
{
    return false;
}

inline void Button::SetOnPressed( ButtonPressedCallback onPressed )
{
    m_OnPressed = onPressed;
}

} // namespace Hexterminate::UI
