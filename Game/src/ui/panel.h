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

class Panel : public Element
{
public:
    Panel( const std::string& name );

    virtual void RenderProperties() override;
    void SetColour( float r, float g, float b, float a );
    void SetBorders( bool left, bool right, bool top, bool bottom );
    void SetBorderColour( float r, float g, float b, float a );

protected:
    virtual void SaveProperties( json& properties ) override;
    virtual void LoadProperties( const json& properties ) override;

private:
    void ApplyBorder();

    bool m_BorderLeft;
    bool m_BorderRight;
    bool m_BorderTop;
    bool m_BorderBottom;
};

} // namespace Hexterminate::UI
