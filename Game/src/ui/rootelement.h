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

namespace Hexterminate::UI
{

//-----------------------------------------------------------------------------
// This is the root UI element in the game. All other Elements should be added
// to this one or as a child of an Element which has been added to this.
// The RootElement automatically resizes itself to fully cover the window.
//-----------------------------------------------------------------------------
class RootElement : public Element
{
public:
    RootElement();
    virtual ~RootElement() override;

    void Save();
    void Load();

    Design* GetDesign() const;

private:
    DesignUniquePtr m_pDesign;
};

inline Design* RootElement::GetDesign() const
{
    return m_pDesign.get();
}

} // namespace Hexterminate::UI
