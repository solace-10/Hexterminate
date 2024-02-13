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

#include <shadercache.h>
#include <vertexbuffer.h>

#include "laser/lasermanager.h"

#include "hexterminate.h"

namespace Hexterminate
{

LaserManager::LaserManager()
    : m_pTexture( nullptr )
    , m_pShader( nullptr )
    , m_pVertexBuffer( nullptr )
{
    using namespace Genesis;

    m_Lasers.reserve( sLaserManagerCapacity );

    m_pTexture = (Genesis::ResourceImage*)Genesis::FrameWork::GetResourceManager()->GetResource( "data/images/laser.png" );

    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );
    m_pShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "laser" );
    ShaderUniform* pSampler = m_pShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    pSampler->Set( m_pTexture, GL_TEXTURE0 );
};

LaserManager::~LaserManager()
{
    delete m_pVertexBuffer;
}

void LaserManager::Update( float delta )
{
    // VB-TODO

    using namespace Genesis;

    const size_t lasersCount = m_Lasers.size();
    if ( lasersCount == 0 )
    {
        return;
    }

    PositionData posData;
    UVData uvData;
    ColourData colourData;

    posData.reserve( lasersCount * 6 );
    uvData.reserve( lasersCount * 6 );
    colourData.reserve( lasersCount * 6 );

    for ( auto& laser : m_Lasers )
    {
        const float halfWidth = laser.GetWidth() / 2.0f;
        const glm::vec3& src = laser.GetSource();
        const glm::vec3& dst = laser.GetDestination();
        glm::vec3 dir = glm::normalize( dst - src );
        glm::vec3 perp( -dir.y * halfWidth, dir.x * halfWidth, 0.0f );

        posData.emplace_back( src.x + perp.x, src.y + perp.y, src.z ); // 0
        posData.emplace_back( src.x - perp.x, src.y - perp.y, src.z ); // 1
        posData.emplace_back( dst.x - perp.x, dst.y - perp.y, dst.z ); // 2
        posData.emplace_back( src.x + perp.x, src.y + perp.y, src.z ); // 0
        posData.emplace_back( dst.x - perp.x, dst.y - perp.y, dst.z ); // 2
        posData.emplace_back( dst.x + perp.x, dst.y + perp.y, dst.z ); // 3

        uvData.emplace_back( 1.0f, 0.0f ); // 0
        uvData.emplace_back( 0.0f, 0.0f ); // 1
        uvData.emplace_back( 0.0f, 1.0f ); // 2
        uvData.emplace_back( 1.0f, 0.0f ); // 0
        uvData.emplace_back( 0.0f, 1.0f ); // 2
        uvData.emplace_back( 1.0f, 1.0f ); // 3

        const glm::vec4 colour = laser.GetColour().glm();
        for ( int i = 0; i < 6; ++i )
        {
            colourData.push_back( colour );
        }
    }

    m_pVertexBuffer->CopyPositions( posData );
    m_pVertexBuffer->CopyUVs( uvData );
    m_pVertexBuffer->CopyColours( colourData );
}

void LaserManager::Render()
{
    using namespace Genesis;

    const size_t lasersCount = m_Lasers.size();
    if ( lasersCount == 0 )
    {
        return;
    }

    const size_t verticesCount = lasersCount * 6;

    RenderSystem* pRenderSystem = FrameWork::GetRenderSystem();
    pRenderSystem->SetBlendMode( BlendMode::Add );

    m_pShader->Use();

    pRenderSystem->SetRenderTarget( RenderTargetId::Glow );
    m_pVertexBuffer->Draw( static_cast<uint32_t>( verticesCount ) );

    pRenderSystem->SetRenderTarget( RenderTargetId::Default );
    m_pVertexBuffer->Draw( static_cast<uint32_t>( verticesCount ) );

    pRenderSystem->SetBlendMode( BlendMode::Disabled );

    m_Lasers.clear();
}

void LaserManager::AddLaser( const Laser& laser )
{
    const size_t numLasers = m_Lasers.size();
    if ( numLasers == sLaserManagerCapacity )
    {
        Genesis::FrameWork::GetLogger()->LogWarning( "Unable to add laser, manager at capacity (%d)", sLaserManagerCapacity );
    }
    else
    {
        m_Lasers.push_back( laser );
    }
}

} // namespace Hexterminate