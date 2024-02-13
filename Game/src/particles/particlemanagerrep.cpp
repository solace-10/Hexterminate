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

#include <algorithm>

#include <genesis.h>
#include <math/misc.h>
#include <rendersystem.h>
#include <resources/resourceimage.h>
#include <shaderuniform.h>

#include "particles/particleemitter.h"
#include "particles/particlemanager.h"
#include "particles/particlemanagerrep.h"
#include "particles/particlepass.h"

namespace Hexterminate
{

bool ParticleSort( const Particle* pA, const Particle* pB )
{
    return pA->GetPosition().z < pB->GetPosition().z;
}

ParticleManagerRep::ParticleManagerRep( ParticleManager* pParticleManager )
    : m_pParticleManager( pParticleManager )
{
    m_pPass[ 0 ] = new ParticlePass( Genesis::BlendMode::Add, "textured_vertex_coloured", true );
    m_pPass[ 1 ] = new ParticlePass( Genesis::BlendMode::Blend, "textured_vertex_coloured", false );
}

ParticleManagerRep::~ParticleManagerRep()
{
    for ( int i = 0; i < sNumParticlePasses; ++i )
    {
        delete m_pPass[ i ];
    }
}

void ParticleManagerRep::Update( float delta )
{
    Genesis::SceneObject::Update( delta );

    if ( m_pParticleManager == nullptr )
    {
        return;
    }

    for ( int i = 0; i < sNumParticlePasses; ++i )
    {
        ParticlePass* pPass = m_pPass[ i ];

        pPass->m_Data.clear();
        const ParticleEmitterVector& emitters = m_pParticleManager->GetEmitters();
        for ( auto& emitter : emitters )
        {
            if ( emitter.GetBlendMode() != pPass->m_BlendMode )
                continue;

            if ( emitter.IsActive() == false )
                continue;

            Genesis::ResourceImage* pTexture = emitter.GetTexture();
            if ( pTexture == nullptr )
                continue;

            int index = FindIndexForTexture( pPass, pTexture->GetTexture() );
            pPass->m_Data[ index ].pAtlas = emitter.GetAtlas(); // TODO: this is wrong, should be passed as a parameter to FindIndexForTexture

            const ParticleVector& particles = emitter.GetParticles();
            for ( auto& particle : particles )
            {
                if ( particle.IsAlive() )
                {
                    pPass->m_Data[ index ].particles.push_back( &particle );
                }
            }
        }

        pPass->m_PositionData.clear();
        pPass->m_UVData.clear();
        pPass->m_ColourData.clear();

        for ( auto& particleRenderData : pPass->m_Data )
        {
            std::sort( particleRenderData.particles.begin(), particleRenderData.particles.end(), ParticleSort );

            for ( auto& particle : particleRenderData.particles )
            {
                AddQuad( particleRenderData.pAtlas, particle, pPass->m_PositionData, pPass->m_UVData, pPass->m_ColourData );
            }
        }

        if ( pPass->m_PositionData.empty() == false )
        {
            pPass->m_pVertexBuffer->CopyPositions( pPass->m_PositionData );
            pPass->m_pVertexBuffer->CopyUVs( pPass->m_UVData );
            pPass->m_pVertexBuffer->CopyColours( pPass->m_ColourData );
        }
    }
}

int ParticleManagerRep::FindIndexForTexture( ParticlePass* pPass, int id )
{
    int i = 0;
    for ( ; i < (int)pPass->m_Data.size(); ++i )
    {
        if ( pPass->m_Data[ i ].textureId == id )
        {
            return i;
        }
    }

    ParticleRenderData prd;
    prd.textureId = id;
    prd.pAtlas = nullptr;
    pPass->m_Data.push_back( prd );
    return i;
}

void ParticleManagerRep::AddQuad( const Genesis::Gui::Atlas* pAtlas, const Particle* pParticle, Genesis::PositionData& vertices, Genesis::UVData& uvs, Genesis::ColourData& colours )
{
    float u1, u2, v1, v2;
    if ( pAtlas->GetElementCount() > 0 )
    {
        float fraction = 1.0f - pParticle->GetCurrentLifetime() / pParticle->GetInitialLifetime();
        int atlasIndex = gClamp<int>( (int)( fraction * pAtlas->GetElementCount() ), 0, pAtlas->GetElementCount() - 1 );

        const Genesis::Gui::AtlasElement& atlasElement = pAtlas->GetElement( atlasIndex );
        u1 = atlasElement.GetU1();
        v1 = atlasElement.GetV1();
        u2 = atlasElement.GetU2();
        v2 = atlasElement.GetV2();
    }
    else
    {
        u1 = 0.0f;
        v1 = 0.0f;
        u2 = 1.0f;
        v2 = 1.0f;
    }

    const glm::vec3& position = pParticle->GetPosition();
    const float x = position.x;
    const float y = position.y;
    const float halfSize = 60.0f * pParticle->GetScale();

    vertices.push_back( glm::vec3( x - halfSize, y - halfSize, 0.0f ) );
    vertices.push_back( glm::vec3( x + halfSize, y - halfSize, 0.0f ) );
    vertices.push_back( glm::vec3( x - halfSize, y + halfSize, 0.0f ) );
    vertices.push_back( glm::vec3( x + halfSize, y - halfSize, 0.0f ) );
    vertices.push_back( glm::vec3( x + halfSize, y + halfSize, 0.0f ) );
    vertices.push_back( glm::vec3( x - halfSize, y + halfSize, 0.0f ) );

    uvs.push_back( glm::vec2( u1, v1 ) );
    uvs.push_back( glm::vec2( u2, v1 ) );
    uvs.push_back( glm::vec2( u1, v2 ) );
    uvs.push_back( glm::vec2( u2, v1 ) );
    uvs.push_back( glm::vec2( u2, v2 ) );
    uvs.push_back( glm::vec2( u1, v2 ) );

    const float alpha = pParticle->GetAlpha();
    glm::vec4 colour( 1.0f, 1.0f, 1.0f, alpha );
    for ( int i = 0; i < 6; ++i )
    {
        colours.push_back( colour );
    }
}

void ParticleManagerRep::Render()
{
    Genesis::SceneObject::Render();

    for ( int i = 0; i < sNumParticlePasses; ++i )
    {
        uint32_t startIdx = 0;
        uint32_t endIdx = 0;

        ParticlePass* pPass = m_pPass[ i ];
        for ( const ParticleRenderData& particleRenderData : pPass->m_Data )
        {
            const uint32_t numParticles = static_cast<uint32_t>( particleRenderData.particles.size() );
            if ( numParticles > 0 )
            {
                Genesis::FrameWork::GetRenderSystem()->SetBlendMode( pPass->m_BlendMode );

                endIdx += numParticles * 6;

                if ( pPass->m_GlowEnabled )
                {
                    Genesis::FrameWork::GetRenderSystem()->SetRenderTarget( Genesis::RenderTargetId::Glow );
                    RenderGeometry( pPass, particleRenderData, startIdx, endIdx );
                }

                Genesis::FrameWork::GetRenderSystem()->SetRenderTarget( Genesis::RenderTargetId::Default );
                RenderGeometry( pPass, particleRenderData, startIdx, endIdx );

                startIdx += numParticles * 6;
            }
        }
    }
}

void ParticleManagerRep::RenderGeometry( ParticlePass* pPass, const ParticleRenderData& particleRenderData, unsigned int startIdx, unsigned int endIdx )
{
    SDL_assert( endIdx > startIdx );
    const unsigned int numVertices = endIdx - startIdx;

    pPass->m_pSamplerUniform->Set( particleRenderData.pAtlas->GetSource(), GL_TEXTURE0 );
    pPass->m_pShader->Use();
    pPass->m_pVertexBuffer->Draw( startIdx, numVertices );
}

} // namespace Hexterminate