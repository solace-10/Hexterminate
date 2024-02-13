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

#include "ui/types.fwd.h"
#include "ui/window.h"

namespace Hexterminate
{

class CreditsWindow : public UI::Window
{
public:
    CreditsWindow();

    virtual void Update() override;
    virtual void Reset() override;

private:
    UI::TextSharedPtr m_pText;
    size_t m_Index;
    float m_Timer;
};

} // namespace Hexterminate
