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

#include "trail/trailmanagerrep.h"
#include "hexterminate.h"
#include "menus/shiptweaks.h"
#include "sector/sector.h"
#include "trail/trail.h"
#include "trail/trailmanager.h"
#include "trail/trailpointdata.h"
#include <genesis.h>
#include <render/debugrender.h>
#include <rendersystem.h>
#include <shader.h>
#include <shadercache.h>
#include <shaderuniform.h>

namespace Hexterminate
{

static const Uint32 sNumBaseIndices = 6;
static const Uint32 sBaseIndices[ sNumBaseIndices ] = { 0, 1, 2, 0, 2, 3 };

TrailManagerRep::TrailManagerRep( TrailManager* pTrailManager )
    : m_pTrailManager( pTrailManager )
    , m_pShader( nullptr )
    , m_pVertexBuffer( nullptr )
    , m_NumVertices( 0 )
{
    using namespace Genesis;
    ResourceImage* pTexture = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/models/misc/trail/trail.png" );

    RenderSystem* pRenderSystem = FrameWork::GetRenderSystem();
    m_pShader = pRenderSystem->GetShaderCache()->Load( "trail" );
    ShaderUniform* pDiffuseSamplerUniform = m_pShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    pDiffuseSamplerUniform->Set( pTexture, GL_TEXTURE0 );

    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );
}

TrailManagerRep::~TrailManagerRep()
{
    delete m_pVertexBuffer;
}

void TrailManagerRep::Update( float delta )
{
    using namespace Genesis;

    SceneObject::Update( delta );

    PositionData posData;
    UVData uvData;
    ColourData colourData;
    posData.reserve( 512 );
    uvData.reserve( 512 );
    colourData.reserve( 512 );

    const TrailList& trails = m_pTrailManager->GetTrails();
    glm::vec3 p1, p2, d;
    glm::vec3 v[ 4 ];
    Genesis::VboFloat3 vf;
    for ( auto& pTrail : trails )
    {
        const TrailPointDataList& data = pTrail->GetData();
        if ( data.size() < 2 )
        {
            continue;
        }

        bool useLast = true;
        for ( TrailPointDataList::const_iterator it = data.begin(), nextIt = data.begin(), endIt = data.end(); ++nextIt != endIt; ++it )
        {
            p1 = it->GetPosition();
            p2 = nextIt->GetPosition();
            d = p2 - p1;
            const float l = glm::length( d );
            if ( l <= std::numeric_limits<float>::epsilon() )
            {
                useLast = false;
                continue;
            }

            d = glm::vec3( -d.y / l, d.x / l, 0.0f );
            d *= it->GetWidth() * 0.5f;

            if ( it == data.begin() || !useLast )
            {
                v[ 0 ] = p1 + d;
                v[ 3 ] = p1 - d;
            }
            else
            {
                v[ 0 ] = v[ 1 ];
                v[ 3 ] = v[ 2 ];
            }

            v[ 1 ] = p2 + d;
            v[ 2 ] = p2 - d;

            posData.push_back( v[ 0 ] );
            posData.push_back( v[ 1 ] );
            posData.push_back( v[ 2 ] );
            posData.push_back( v[ 0 ] );
            posData.push_back( v[ 2 ] );
            posData.push_back( v[ 3 ] );

            PushBackUVs( uvData );
            PushBackColours( colourData, pTrail->GetColour(), pTrail->GetInitialWidth(), it->GetWidth(), nextIt->GetWidth() );

            useLast = true;
        }
    }

    if ( g_pGame->GetCurrentSector()->GetShipTweaks()->GetDrawTrails() )
    {
        size_t l = posData.size();
        for ( size_t i = 0; i < l - 1; ++i )
        {
            if ( l > 2 )
            {
                Genesis::FrameWork::GetDebugRender()->DrawLine(
                    posData[ i ],
                    posData[ i + 1 ],
                    glm::vec3( 1.0f, 0.0f, 0.0f ) );
            }
        }
    }

    if ( posData.empty() == false )
    {
        m_pVertexBuffer->CopyPositions( posData );
        m_pVertexBuffer->CopyUVs( uvData );
        m_pVertexBuffer->CopyColours( colourData );

        m_NumVertices = posData.size();
    }
}

void TrailManagerRep::PushBackUVs( Genesis::UVData& uvData )
{
    using namespace Genesis;
    static const glm::vec2 uvs[ 4 ] = {
        glm::vec2( 0.0f, 0.0f ),
        glm::vec2( 1.0f, 0.0f ),
        glm::vec2( 1.0f, 1.0f ),
        glm::vec2( 0.0f, 1.0f )
    };

    for ( Uint32 i = 0; i < sNumBaseIndices; ++i )
    {
        uvData.push_back( uvs[ sBaseIndices[ i ] ] );
    }
}

void TrailManagerRep::PushBackColours( Genesis::ColourData& colourData, const Genesis::Color& colour, float initialWidth, float currentWidth, float nextWidth )
{
    using namespace Genesis;
    const float r = colour.r;
    const float g = colour.g;
    const float b = colour.b;
    const float a1 = currentWidth / initialWidth;
    const float a2 = nextWidth / initialWidth;
    const glm::vec4 colours[ 4 ] = {
        glm::vec4( r, g, b, a1 ),
        glm::vec4( r, g, b, a2 ),
        glm::vec4( r, g, b, a2 ),
        glm::vec4( r, g, b, a1 )
    };

    for ( Uint32 i = 0; i < sNumBaseIndices; ++i )
        colourData.push_back( colours[ sBaseIndices[ i ] ] );
}

void TrailManagerRep::Render()
{
    using namespace Genesis;

    SceneObject::Render();

    if ( m_NumVertices > 0 )
    {
        RenderSystem* pRenderSystem = FrameWork::GetRenderSystem();
        pRenderSystem->SetBlendMode( BlendMode::Blend );

        m_pShader->Use();

        pRenderSystem->SetRenderTarget( RenderTargetId::Glow );
        m_pVertexBuffer->Draw( static_cast<uint32_t>( m_NumVertices ) );
        pRenderSystem->SetRenderTarget( RenderTargetId::Default );
        m_pVertexBuffer->Draw( static_cast<uint32_t>( m_NumVertices ) );

        pRenderSystem->SetBlendMode( BlendMode::Disabled );
    }
}

} // namespace Hexterminate