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

#include <gui/gui.h>
#include <imgui/imgui.h>

#include "ui/itoggleable.h"
#include "ui/togglegroup.h"

namespace Hexterminate
{
namespace UI
{

    void ToggleGroup::Add( IToggleableSharedPtr pToggleable )
    {
        m_Group.push_back( pToggleable );
    }

    void ToggleGroup::Clear()
    {
        m_Group.clear();
    }

    void ToggleGroup::Select( IToggleable* pToggleable )
    {
        for ( auto& pGroupedToggleableWeakPtr : m_Group )
        {
            IToggleableSharedPtr pGroupedToggleable = pGroupedToggleableWeakPtr.lock();
            if ( pGroupedToggleable != nullptr )
            {
                pGroupedToggleable->Toggle( pGroupedToggleable.get() == pToggleable );
            }
        }
    }

} // namespace UI
} // namespace Hexterminate