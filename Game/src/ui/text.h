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

#include <array>
#include <string>

#include "ui/element.h"

namespace Genesis::Gui
{
class Text;
}

namespace Hexterminate::UI
{

class Text : public Element
{
public:
    Text( const std::string& name );
    virtual ~Text() override;

    virtual void RenderProperties() override;
    virtual void SetSize( int width, int height ) override;

    void SetColour( float r, float g, float b, float a );
    void SetFont( const std::string& font );
    void SetText( const std::string& text );

protected:
    virtual void SaveProperties( json& properties ) override;
    virtual void LoadProperties( const json& properties ) override;

    bool m_Multiline;
    Genesis::Gui::Text* m_pText;
    std::string m_Label;
    std::array<float, 4> m_Colour;
};

} // namespace Hexterminate::UI
