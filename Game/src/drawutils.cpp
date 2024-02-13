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

#include "drawutils.h"
#include <cassert>
#include <rendersystem.h>

namespace Hexterminate
{

/////////////////////////////////////////////////////////////////////
// DrawUtils
/////////////////////////////////////////////////////////////////////

void DrawUtils::Rect2D( Genesis::Shader* pShader, int x, int y, int width, int height )
{
    Rect2D( pShader, (float)x, (float)y, (float)width, (float)height );
}

void DrawUtils::Rect2D( Genesis::Shader* pShader, float x, float y, float width, float height )
{
    using namespace Genesis;

    SDL_assert( pShader != nullptr );

    const float x1 = x;
    const float x2 = x1 + width;
    const float y1 = y;
    const float y2 = y1 + height;

    pShader->Use();

    glBegin( GL_QUADS );

    glTexCoord2f( 0.0f, 0.0f );
    glVertex2f( x1, y1 );
    glTexCoord2f( 0.0f, 1.0f );
    glVertex2f( x1, y2 );
    glTexCoord2f( 1.0f, 1.0f );
    glVertex2f( x2, y2 );
    glTexCoord2f( 1.0f, 0.0f );
    glVertex2f( x2, y1 );

    glEnd();
}

void DrawUtils::Rect2DOriented( Genesis::Shader* pShader, int x, int y, int width, int height, float angle )
{
    SDL_assert( false ); // TODO: Implement
}

} // namespace Hexterminate
