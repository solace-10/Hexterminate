// Copyright 2024 Pedro Nunes
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

#include <glm/vec2.hpp>


#include "ui/element.h"
#include "ui/types.fwd.h"

namespace Hexterminate::UI
{

class Container : public Element
{
public:
    Container( const std::string& name );

    virtual void Update() override;
    virtual void RenderProperties() override;

    virtual void Add( ElementSharedPtr pElement ) override;
    virtual void Remove( ElementSharedPtr pElement ) override;

protected:
    virtual void SaveProperties( json& properties ) override;
    virtual void LoadProperties( const json& properties ) override;

private:
    void RebuildContents();

    enum class Direction
    {
        Row,
        RowReverse,
        Column,
        ColumnReverse
    };
    Direction m_Direction;

    glm::ivec2 m_CachedParentSize;
    bool m_Dirty;
};

} // namespace Hexterminate::UI
