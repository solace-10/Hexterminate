// Copyright 2019 Pedro Nunes
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

#include <glm/gtc/matrix_access.hpp>

#include "hexterminate.h"
#include "muzzleflash/muzzleflashdata.h"
#include "muzzleflash/muzzleflashmanager.h"
#include "muzzleflash/muzzleflashmanagerrep.h"
#include "sector/sector.h"
#include <genesis.h>
#include <render/debugrender.h>
#include <rendersystem.h>
#include <shader.h>
#include <shadercache.h>
#include <shaderuniform.h>

namespace Hexterminate
{

static const Uint32 sMuzzleflashNumBaseIndices = 6;
static const Uint32 sMuzzleflashBaseIndices[ sMuzzleflashNumBaseIndices ] = { 0, 1, 2, 0, 2, 3 };

MuzzleflashManagerRep::MuzzleflashManagerRep( MuzzleflashManager* pManager )
    : m_pManager( pManager )
    , m_pShader( nullptr )
    , m_pVertexBuffer( nullptr )
    , m_NumVertices( 0 )
{
    using namespace Genesis;
    ResourceImage* pTexture = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/images/muzzleflash.png" );

    RenderSystem* pRenderSystem = FrameWork::GetRenderSystem();
    m_pShader = pRenderSystem->GetShaderCache()->Load( "muzzleflash" );
    ShaderUniform* pDiffuseSamplerUniform = m_pShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    pDiffuseSamplerUniform->Set( pTexture, GL_TEXTURE0 );

    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );
}

MuzzleflashManagerRep::~MuzzleflashManagerRep()
{
    delete m_pVertexBuffer;
}

void MuzzleflashManagerRep::Update( float delta )
{
    using namespace Genesis;

    SceneObject::Update( delta );

    if ( m_pManager == nullptr )
    {
        return;
    }

    PositionData posData;
    UVData uvData;
    ColourData colourData;
    posData.reserve( 512 );
    uvData.reserve( 512 );
    colourData.reserve( 512 );

    const MuzzleflashDataVector& muzzleflashes = m_pManager->GetMuzzleflashes();
    glm::vec3 p1, p2, d;
    glm::vec3 v[ 4 ];
    Genesis::VboFloat3 vf;
    for ( auto& muzzleflash : muzzleflashes )
    {
        glm::mat4x4 weaponTransform = muzzleflash.GetWeapon()->GetWorldTransform();
        glm::mat4x4 muzzleOffsetTransform = glm::translate( muzzleflash.GetMuzzleOffset() );

        weaponTransform = weaponTransform * muzzleOffsetTransform;

        glm::vec3 weaponPosition( glm::column( weaponTransform, 3 ) );
        glm::vec3 weaponForward( glm::column( weaponTransform, 1 ) );

        float mflAdjustment = -2.0f;
        float mfl = 32.0f * muzzleflash.GetScale();
        float mfw = 16.0f * muzzleflash.GetScale();

        p1 = weaponPosition + weaponForward * mflAdjustment;
        p2 = weaponPosition + weaponForward * ( mfl + mflAdjustment );
        d = p2 - p1;
        const float l = glm::length( d );

        float dx = muzzleflash.GetLifetime() * muzzleflash.GetRotationMultiplier();
        for ( int i = 0; i < 2; ++i )
        {
            float cdx = cosf( dx );
            float sdx = sinf( dx );
            d = glm::vec3( -d.y / l, ( i == 0 ) ? cdx : sdx, ( i == 0 ) ? sdx : cdx );
            d *= mfw * 0.5f;

            v[ 0 ] = p1 + d;
            v[ 1 ] = p2 + d;
            v[ 2 ] = p2 - d;
            v[ 3 ] = p1 - d;

            posData.push_back( v[ 0 ] );
            posData.push_back( v[ 1 ] );
            posData.push_back( v[ 2 ] );
            posData.push_back( v[ 0 ] );
            posData.push_back( v[ 2 ] );
            posData.push_back( v[ 3 ] );

            PushBackUVs( uvData );
            PushBackColours( colourData, muzzleflash.GetWeapon()->GetInfo()->GetMuzzleflashColour() );
        }
    }

    m_NumVertices = posData.size();
    if ( posData.empty() == false )
    {
        m_pVertexBuffer->CopyPositions( posData );
        m_pVertexBuffer->CopyUVs( uvData );
        m_pVertexBuffer->CopyColours( colourData );
    }
}

void MuzzleflashManagerRep::PushBackUVs( Genesis::UVData& uvData )
{
    using namespace Genesis;
    static const glm::vec2 uvs[ 4 ] = {
        glm::vec2( 0.0f, 0.0f ),
        glm::vec2( 1.0f, 0.0f ),
        glm::vec2( 1.0f, 1.0f ),
        glm::vec2( 0.0f, 1.0f )
    };

    for ( Uint32 i = 0; i < sMuzzleflashNumBaseIndices; ++i )
    {
        uvData.push_back( uvs[ sMuzzleflashBaseIndices[ i ] ] );
    }
}

void MuzzleflashManagerRep::PushBackColours( Genesis::ColourData& colourData, const Genesis::Color& colour )
{
    using namespace Genesis;
    const float r = colour.r;
    const float g = colour.g;
    const float b = colour.b;
    const float a = 1.0f;
    const glm::vec4 colours[ 4 ] = {
        glm::vec4( r, g, b, a ),
        glm::vec4( r, g, b, a ),
        glm::vec4( r, g, b, a ),
        glm::vec4( r, g, b, a )
    };

    for ( Uint32 i = 0; i < sMuzzleflashNumBaseIndices; ++i )
    {
        colourData.push_back( colours[ sMuzzleflashBaseIndices[ i ] ] );
    }
}

void MuzzleflashManagerRep::Render()
{
    using namespace Genesis;

    SceneObject::Render();

    if ( m_NumVertices > 0 )
    {
        RenderSystem* pRenderSystem = FrameWork::GetRenderSystem();
        pRenderSystem->SetBlendMode( BlendMode::Add );

        m_pShader->Use();

        pRenderSystem->SetRenderTarget( RenderTargetId::Glow );
        m_pVertexBuffer->Draw( static_cast<uint32_t>( m_NumVertices ) );
        pRenderSystem->SetRenderTarget( RenderTargetId::Default );
        m_pVertexBuffer->Draw( static_cast<uint32_t>( m_NumVertices ) );

        pRenderSystem->SetBlendMode( BlendMode::Disabled );
    }
}

void MuzzleflashManagerRep::SetManager( MuzzleflashManager* pManager )
{
    m_pManager = pManager;
}

} // namespace Hexterminate