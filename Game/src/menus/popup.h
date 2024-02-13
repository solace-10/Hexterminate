// Copyright 2014 Pedro Nunes
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

#include "menus/cursortype.h"
#include "menus/eva.h"
#include <gui/gui.h>

namespace Hexterminate
{

class ButtonPopup;

enum class PopupMode
{
    Undefined,
    YesNo,
    Ok,
    OkCancel
};

enum class PopupState
{
    Undefined,
    Displayed,
    Yes,
    No,
    Ok,
    Cancel
};

///////////////////////////////////////////////////////////////////////////////
// Popup
///////////////////////////////////////////////////////////////////////////////

class Popup
{
public:
    Popup();
    ~Popup();

    void Update( float delta );
    void Show( PopupMode mode, const std::string& text );
    void Close();

    void SetState( PopupState state );
    PopupState GetState() const;

    bool IsActive() const;

private:
    void AlignToCentre( Genesis::Gui::Text* pText );
    void RepositionButtons();

    EvaWindow* m_pWindow;
    Genesis::Gui::Text* m_pText;
    ButtonPopup* m_pButton[ 2 ];
    Genesis::Gui::Panel* m_pBackground;

    PopupState m_State;
    PopupMode m_Mode;

    float m_InputTimer;
    CursorType m_PreviousCursorType;
};

inline bool Popup::IsActive() const
{
    return ( m_State == PopupState::Displayed || m_InputTimer > 0.0f );
}

inline PopupState Popup::GetState() const
{
    return m_State;
}

///////////////////////////////////////////////////////////////////////////////
// ButtonPopup
///////////////////////////////////////////////////////////////////////////////

class ButtonPopup : public Genesis::Gui::Button
{
public:
    ButtonPopup( Popup* pOwner );
    virtual ~ButtonPopup() override{};
    virtual void OnPress() override;

    void SetCallbackState( PopupState state );

private:
    Popup* m_pOwner;
    PopupState m_CallbackState;
};

} // namespace Hexterminate
