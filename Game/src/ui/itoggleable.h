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

#include <memory>
#include <vector>

#include "ui/types.fwd.h"

namespace Hexterminate::UI
{

//-----------------------------------------------------------------------------
// IToggleable
// This element can be toggle on and off. See ToggleGroup.
//-----------------------------------------------------------------------------
class IToggleable
{
public:
    IToggleable() {}
    virtual ~IToggleable() {}
    virtual void Toggle( bool state ) = 0;
};

} // namespace Hexterminate::UI
