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

#ifndef GENESIS_GUI_ATLAS_H
#define GENESIS_GUI_ATLAS_H

#include <string>
#include <vector>

namespace Genesis
{

class ResourceImage;

namespace Gui
{

    /////////////////////////////////////////////////////////////////////
    // AtlasElement
    /////////////////////////////////////////////////////////////////////

    class AtlasElement
    {
    public:
        AtlasElement( float x1, float y1, float x2, float y2, int width, int height );

        float GetU1() const { return m_U1; }
        float GetV1() const { return m_V1; }
        float GetU2() const { return m_U2; }
        float GetV2() const { return m_V2; }
        float GetWidth() const { return m_Width; }
        float GetHeight() const { return m_Height; }

    private:
        float m_U1;
        float m_V1;
        float m_U2;
        float m_V2;
        float m_Width;
        float m_Height;
    };

    typedef std::vector<AtlasElement> AtlasElementVector;

    /////////////////////////////////////////////////////////////////////
    // Atlas
    /////////////////////////////////////////////////////////////////////

    class Atlas
    {
    public:
        Atlas();
        ~Atlas(){};

        Genesis::ResourceImage* GetSource() const;
        void SetSource( Genesis::ResourceImage* pImage );
        int GetElementCount() const;
        int AddElement( float x1, float y1, float x2, float y2 );
        const AtlasElement& GetElement( int index ) const;

    private:
        Genesis::ResourceImage* m_pSource;
        AtlasElementVector m_Elements;
    };

    inline int Atlas::GetElementCount() const
    {
        return static_cast<int>(m_Elements.size());
    }

    inline const AtlasElement& Atlas::GetElement( int index ) const
    {
#ifdef _DEBUG
        return m_Elements.at( index );
#else
        return m_Elements[ index ];
#endif
    }

    inline Genesis::ResourceImage* Atlas::GetSource() const
    {
        return m_pSource;
    }
}
}

#endif