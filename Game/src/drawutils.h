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

namespace Genesis
{
class Shader;
}

namespace Hexterminate
{

/////////////////////////////////////////////////////////////////////
// DrawUtils
// Collection of functions to encapsulate simple drawing.
/////////////////////////////////////////////////////////////////////

class DrawUtils
{
public:
    // Draws a 2D rectangle with a given shader. X and Y are the top left coordinates.
    static void Rect2D( Genesis::Shader* pShader, int x, int y, int width, int height );
    static void Rect2D( Genesis::Shader* pShader, float x, float y, float width, float height );

    // Draws an oriented 2D rectangle with a given shader. X and Y are the center of rectangle and the rotation
    // happens around this point.
    static void Rect2DOriented( Genesis::Shader* pShader, int x, int y, int width, int height, float angle );
};

} // namespace Hexterminate