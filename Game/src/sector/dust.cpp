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

#include <genesis.h>
#include <scene/scene.h>

#include "dust.h"
#include <configuration.h>
#include <rendersystem.h>
#include <resources/resourceimage.h>
#include <shader.h>
#include <shadercache.h>
#include <shaderuniform.h>
#include <vertexbuffer.h>

namespace Hexterminate
{

Dust::Dust()
{
    using namespace Genesis;

    m_pDust = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/backgrounds/dust.png" );

    m_pShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "textured" );
    ShaderUniform* pSampler = m_pShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    pSampler->Set( m_pDust, GL_TEXTURE0 );

    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV );

    DustParticle particle;
    for ( int i = 0; i < 256; ++i )
    {
        particle.x = (float)( rand() % 1000 ) / 1000.0f * 1200.0f - 600.0f;
        particle.y = (float)( rand() % 1000 ) / 1000.0f * 1200.0f - 600.0f;
        particle.z = (float)( rand() % 1000 ) / 1000.0f * 160.0f;
        m_dustParticles.push_back( particle );
    }
}

Dust::~Dust()
{
    delete m_pVertexBuffer;
}

void Dust::Update( float fDelta )
{
    glm::vec3 cameraPos = Genesis::FrameWork::GetScene()->GetCamera()->GetPosition();

    static const float sDistanceThreshold = 600.0f;

    for ( auto& particle : m_dustParticles )
    {
        if ( particle.x < cameraPos.x - sDistanceThreshold )
        {
            particle.x += sDistanceThreshold * 2.0f;
        }
        else if ( particle.x > cameraPos.x + sDistanceThreshold )
        {
            particle.x -= sDistanceThreshold * 2.0f;
        }
        else if ( particle.y < cameraPos.y - sDistanceThreshold )
        {
            particle.y += sDistanceThreshold * 2.0f;
        }
        else if ( particle.y > cameraPos.y + sDistanceThreshold )
        {
            particle.y -= sDistanceThreshold * 2.0f;
        }
    }
}

void Dust::Render()
{
    using namespace Genesis;

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Blend );

    const glm::vec2 size( 3.0f, 1.0f );
    size_t numVertices = m_dustParticles.size() * 6;
    PositionData posData;
    UVData uvData;
    posData.reserve( numVertices );
    uvData.reserve( numVertices );

    for ( auto& particle : m_dustParticles )
    {
        particle.z = 0.0f;

        posData.emplace_back( particle.x, particle.y, particle.z ); // 0
        posData.emplace_back( particle.x, particle.y + size.y, particle.z ); // 1
        posData.emplace_back( particle.x + size.x, particle.y + size.y, particle.z ); // 2
        posData.emplace_back( particle.x, particle.y, particle.z ); // 0
        posData.emplace_back( particle.x + size.x, particle.y + size.y, particle.z ); // 2
        posData.emplace_back( particle.x + size.x, particle.y, particle.z ); // 3

        uvData.emplace_back( 0.0f, 0.0f ); // 0
        uvData.emplace_back( 0.0f, 1.0f ); // 1
        uvData.emplace_back( 1.0f, 1.0f ); // 2
        uvData.emplace_back( 0.0f, 0.0f ); // 0
        uvData.emplace_back( 1.0f, 1.0f ); // 2
        uvData.emplace_back( 1.0f, 0.0f ); // 3
    }

    m_pVertexBuffer->CopyPositions( posData );
    m_pVertexBuffer->CopyUVs( uvData );
    m_pShader->Use();
    m_pVertexBuffer->Draw();

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Disabled );
}

} // namespace Hexterminate