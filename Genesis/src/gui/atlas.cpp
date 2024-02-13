// Copyright 2014 Pedro Nunes
//
// This file is part of Genesis.
//
// Genesis is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Genesis is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Genesis. If not, see <http://www.gnu.org/licenses/>.

#include "atlas.h"
#include "../resources/resourceimage.h"

namespace Genesis
{

namespace Gui
{

    /////////////////////////////////////////////////////////////////////
    // AtlasElement
    /////////////////////////////////////////////////////////////////////

    AtlasElement::AtlasElement( float x1, float y1, float x2, float y2, int width, int height )
    {
        SDL_assert( width > 0 && height > 0 );
        static const float sBias = 0.5f;
        m_U1 = ( x1 + sBias ) / width;
        m_V1 = ( y1 + sBias ) / height;
        m_U2 = ( x2 - sBias ) / width;
        m_V2 = ( y2 - sBias ) / height;
        m_Width = fabs( x2 - x1 );
        m_Height = fabs( y2 - y1 );
    }

    /////////////////////////////////////////////////////////////////////
    // Atlas
    /////////////////////////////////////////////////////////////////////

    Atlas::Atlas()
        : m_pSource( nullptr )
    {
    }

    void Atlas::SetSource( ResourceImage* pImage )
    {
        if ( pImage != m_pSource && m_Elements.empty() == false )
        {
            m_Elements.clear();
        }

        m_pSource = pImage;
    }

    int Atlas::AddElement( float x1, float y1, float x2, float y2 )
    {
        SDL_assert( m_pSource != nullptr );
        m_Elements.push_back( AtlasElement( x1, y1, x2, y2, m_pSource->GetWidth(), m_pSource->GetHeight() ) );
        return static_cast<int>(m_Elements.size() - 1);
    }
}
}