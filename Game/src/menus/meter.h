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

#include <gui/gui.h>
#include <vector>

namespace Hexterminate
{

typedef std::vector<Genesis::Gui::Panel*> PanelVector;

class Meter : public Genesis::Gui::Panel
{
public:
    Meter();
    void Init( int initialValue, int maxValue );
    void SetValue( int value );
    void SetPipColour( const Genesis::Color& colour );
    void SetPipColour( float r, float g, float b, float a );

protected:
    PanelVector m_Panels;
    Genesis::Color m_PipColour;
};

} // namespace Hexterminate