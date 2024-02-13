#include "resourcefont.h"
#include "../genesis.h"
#include "../logger.h"
#include "../rendersystem.h"
#include "../vertexbuffer.h"
#include "resourceimage.h"
#include "rendersystem.fwd.h"

#include "beginexternalheaders.h"
#include "tinyxml2.h"
#include "endexternalheaders.h"

namespace Genesis
{

ResourceFont::ResourceFont( const Filename& filename )
    : ResourceGeneric( filename )
    , mLineHeight( 0.0f )
    , mPage( 0 )
{
}

ResourceFont::~ResourceFont()
{
    if ( !mCharList.empty() )
    {
        for ( auto& fontChar : mCharList )
        {
            delete fontChar;
        }
    }
}

bool ResourceFont::Load()
{
    if ( LoadFontFile( GetFilename().GetFullPath() ) )
    {
        BuildLists();
        m_State = ResourceState::Loaded;
        return true;
    }
    else
    {
        return false;
    }
}

bool ResourceFont::LoadFontFile( const std::string& filename )
{
    tinyxml2::XMLDocument doc;

    if ( doc.LoadFile( filename.c_str() ) == tinyxml2::XML_SUCCESS )
    {
        tinyxml2::XMLElement* elemConfiguration = doc.FirstChildElement();
        for ( tinyxml2::XMLElement* elemEntry = elemConfiguration->FirstChildElement(); elemEntry; elemEntry = elemEntry->NextSiblingElement() )
        {
            if ( strcmp( elemEntry->Value(), "common" ) == 0 )
            {
                mLineHeight = (float)atof( elemEntry->Attribute( "lineHeight" ) );
            }
            else if ( strcmp( elemEntry->Value(), "pages" ) == 0 )
            {
                // Only 1 page supported, file must be inside a folder
                tinyxml2::XMLElement* elemPage = elemEntry->FirstChildElement();
                std::string pageFile = filename.substr( 0, filename.find_last_of( "\\/" ) + 1 ) + elemPage->Attribute( "file" );
                mPage = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( pageFile.c_str() );
                if ( !mPage )
                    return false;
            }
            else if ( strcmp( elemEntry->Value(), "chars" ) == 0 )
            {
                int count = atoi( elemEntry->Attribute( "count" ) );
                mCharList.reserve( count );

                for ( tinyxml2::XMLElement* elemChar = elemEntry->FirstChildElement(); elemChar; elemChar = elemChar->NextSiblingElement() )
                {
                    FontChar* fontChar = new FontChar();
                    fontChar->x = (float)atof( elemChar->Attribute( "x" ) );
                    fontChar->y = (float)atof( elemChar->Attribute( "y" ) );
                    fontChar->width = (float)atof( elemChar->Attribute( "width" ) );
                    fontChar->height = (float)atof( elemChar->Attribute( "height" ) );
                    fontChar->xoffset = (float)atof( elemChar->Attribute( "xoffset" ) );
                    fontChar->yoffset = (float)atof( elemChar->Attribute( "yoffset" ) );
                    fontChar->xadvance = (float)atof( elemChar->Attribute( "xadvance" ) );
                    mCharList.push_back( fontChar );
                }
            }
        }
    }
    return true;
}

void ResourceFont::BuildLists()
{
    const float textureWidth = (float)mPage->GetWidth();
    const float textureHeight = (float)mPage->GetHeight();

    mCharRenderDataArray.reserve( mCharList.size() );

    FontCharList::const_iterator itEnd = mCharList.end();
    for ( FontCharList::const_iterator it = mCharList.begin(); it != itEnd; it++ )
    {
        FontChar* fontChar = ( *it );

        const float u1 = fontChar->x / textureWidth;
        const float v1 = fontChar->y / textureHeight;
        const float u2 = ( fontChar->x + fontChar->width ) / textureWidth;
        const float v2 = ( fontChar->y + fontChar->height ) / textureHeight;

        const float x1 = fontChar->xoffset;
        const float y1 = fontChar->yoffset;
        const float x2 = fontChar->width + fontChar->xoffset;
        const float y2 = fontChar->height + fontChar->yoffset;

        FontCharRenderData renderData;
        renderData.position[ 0 ] = glm::vec3( x1, y1, 0.0f );
        renderData.position[ 1 ] = glm::vec3( x1, y2, 0.0f );
        renderData.position[ 2 ] = glm::vec3( x2, y2, 0.0f );
        renderData.position[ 3 ] = glm::vec3( x2, y1, 0.0f );
        renderData.uv[ 0 ] = glm::vec2( u1, v1 );
        renderData.uv[ 1 ] = glm::vec2( u1, v2 );
        renderData.uv[ 2 ] = glm::vec2( u2, v2 );
        renderData.uv[ 3 ] = glm::vec2( u2, v1 );
        mCharRenderDataArray.push_back( renderData );
    }
}

unsigned int ResourceFont::PopulateVertexBuffer( VertexBuffer& vertexBuffer, float x, float y, const std::string& text, float lineSpacing )
{
    if ( text.empty() )
    {
        return 0;
    }

    const int textLength = static_cast<int>(text.length());
    PositionData positionData;
    UVData uvData;
    positionData.reserve( textLength * 6 );
    uvData.reserve( textLength * 6 );

    float xtranslate = x;
    float ytranslate = y;

    unsigned int vertexCount = 0;
    for ( int i = 0; i < textLength; i++ )
    {
        if ( text[ i ] == '\n' )
        {
            xtranslate = x;
            ytranslate += floorf( mLineHeight * lineSpacing );
            continue;
        }

        int fontCharPos = text[ i ] - 32;
        if ( fontCharPos >= (int)mCharList.size() || fontCharPos < 0 )
            continue;

        const FontCharRenderData& renderData = mCharRenderDataArray[ fontCharPos ];

        const glm::vec3 vtranslate( xtranslate, ytranslate, 0.0f );
        positionData.push_back( renderData.position[ 0 ] + vtranslate );
        positionData.push_back( renderData.position[ 1 ] + vtranslate );
        positionData.push_back( renderData.position[ 2 ] + vtranslate );
        positionData.push_back( renderData.position[ 0 ] + vtranslate );
        positionData.push_back( renderData.position[ 2 ] + vtranslate );
        positionData.push_back( renderData.position[ 3 ] + vtranslate );
           
        uvData.push_back( renderData.uv[ 0 ] );
        uvData.push_back( renderData.uv[ 1 ] );
        uvData.push_back( renderData.uv[ 2 ] );
        uvData.push_back( renderData.uv[ 0 ] );
        uvData.push_back( renderData.uv[ 2 ] );
        uvData.push_back( renderData.uv[ 3 ] );

        vertexCount += 6;

        xtranslate += mCharList[ fontCharPos ]->xadvance;
    }

    vertexBuffer.CopyPositions( positionData );
    vertexBuffer.CopyUVs( uvData );
    return vertexCount;
}
}