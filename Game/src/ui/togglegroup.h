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

#include "ui/itoggleable.h"
#include "ui/types.fwd.h"

namespace Hexterminate::UI
{

class ToggleGroup;
using ToggleGroupSharedPtr = std::shared_ptr<ToggleGroup>;
using ToggleGroupWeakPtr = std::weak_ptr<ToggleGroup>;

//-----------------------------------------------------------------------------
// ToggleGroup
// Keeps a collection of weak pointers to IToggleable elements.
// When one is selected, all others are deselected.
//-----------------------------------------------------------------------------
class ToggleGroup
{
public:
    ToggleGroup() {}
    ~ToggleGroup() {}

    void Add( IToggleableSharedPtr pToggleable );
    void Clear();

    void Select( IToggleable* pToggleable );

private:
    std::vector<IToggleableWeakPtr> m_Group;
};

} // namespace Hexterminate::UI
