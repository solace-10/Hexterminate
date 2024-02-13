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

#pragma once

#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "resourcemanager.h"
#include "rendersystem.fwd.h"

namespace Genesis
{
class ResourceImage;
class VertexBuffer;

class ResourceFont : public ResourceGeneric
{
public:
    ResourceFont( const Filename& filename );
    virtual ~ResourceFont();
    virtual ResourceType GetType() const override;
    virtual bool Load() override;

    unsigned int PopulateVertexBuffer( VertexBuffer& vertexBuffer, float x, float y, const std::string& text, float lineSpacing );
    ResourceImage* GetPage() const;
    float GetTextLength( const std::string& text ) const;
    float GetLineHeight() const;

private:
    bool LoadFontFile( const std::string& filename );
    void BuildLists();

    struct FontChar
    {
        float x;
        float y;
        float width;
        float height;
        float xoffset;
        float yoffset;
        float xadvance;
    };

    struct FontCharRenderData
    {
        glm::vec3 position[ 4 ];
        glm::vec2 uv[ 4 ];
    };

    typedef std::vector<FontChar*> FontCharList;
    typedef std::vector<FontCharRenderData> FontCharRenderDataArray;
    FontCharList mCharList;
    FontCharRenderDataArray mCharRenderDataArray;
    float mLineHeight;
    ResourceImage* mPage;
};

inline float ResourceFont::GetTextLength( const std::string& text ) const
{
    float len = 0.0f;
    size_t charListSize = mCharList.size();
    size_t s = text.length();
    for ( size_t i = 0; i < s; i++ )
    {
        int fontCharPos = text[ i ] - 32;
        if ( fontCharPos >= charListSize || fontCharPos < 0 )
            continue;
        len += mCharList[ fontCharPos ]->xadvance;
    }
    return len;
}

inline float ResourceFont::GetLineHeight() const
{
    return mLineHeight;
}

inline ResourceImage* ResourceFont::GetPage() const
{
    return mPage;
}

inline ResourceType ResourceFont::GetType() const
{
    return ResourceType::Font;
}
}