// Copyright 2015 Pedro Nunes
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

#include "menus/eva.h"
#include <gui/gui.h>

namespace Hexterminate
{

class ButtonHyperspaceJump;
class ButtonHyperspaceCancel;

///////////////////////////////////////////////////////////////////////////////
// HyperspaceMenu
///////////////////////////////////////////////////////////////////////////////

class HyperspaceMenu
{
public:
    HyperspaceMenu();
    ~HyperspaceMenu();

    void Update( float delta );
    void Show( bool state );
    void ShowProgress();

private:
    void AlignToCentre( Genesis::Gui::Text* pText );

    EvaWindow* m_pWindow;
    Genesis::Gui::Text* m_pText;
    Genesis::Gui::Text* m_pText2;
    ButtonHyperspaceJump* m_pButtonJump;
    ButtonHyperspaceCancel* m_pButtonCancel;
    bool m_ShowProgress;
};

///////////////////////////////////////////////////////////////////////////////
// ButtonHyperspaceJump
///////////////////////////////////////////////////////////////////////////////

class ButtonHyperspaceJump : public Genesis::Gui::Button
{
public:
    ButtonHyperspaceJump( HyperspaceMenu* pOwner );
    virtual ~ButtonHyperspaceJump() override{};
    virtual void OnPress() override;

private:
    HyperspaceMenu* m_pOwner;
};

///////////////////////////////////////////////////////////////////////////////
// ButtonHyperspaceCancel
///////////////////////////////////////////////////////////////////////////////

class ButtonHyperspaceCancel : public Genesis::Gui::Button
{
public:
    ButtonHyperspaceCancel( HyperspaceMenu* pOwner );
    virtual ~ButtonHyperspaceCancel() override{};
    virtual void OnPress() override;

private:
    HyperspaceMenu* m_pOwner;
};

} // namespace Hexterminate