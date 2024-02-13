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

#include <cassert>

#include <genesis.h>
#include <math/misc.h>
#include <rendersystem.h>
#include <resources/resourceimage.h>

#include "particles/particle.h"
#include "particles/particleemitter.h"

namespace Hexterminate
{

ParticleEmitter::ParticleEmitter()
{
}

ParticleEmitter::~ParticleEmitter()
{
}

void ParticleEmitter::Reset()
{
    m_Active = false;
    m_UsingAtlas = false;
    m_GlowEnabled = false;
    m_ParticlesToSpawn = 0;
    m_EmissionDelay = 0.0f;
    m_EmissionTimer = 0.0f;
    m_BlendMode = Genesis::BlendMode::Disabled;
    m_pTexture = nullptr;
    m_Position = glm::vec3( 0.0f );
    m_Idx = 0;
    m_MinScale = 1.0f;
    m_MaxScale = 1.0f;
    m_MinLifetime = 1.0f;
    m_MaxLifetime = 1.0f;
    m_Velocity = glm::vec3( 0.0f );
}

void ParticleEmitter::Stop()
{
    m_ParticlesToSpawn = 0;
}

void ParticleEmitter::Update( float delta )
{
    m_EmissionTimer += delta;
    while ( ( m_ParticlesToSpawn > 0 || m_ParticlesToSpawn == sInfiniteParticles ) && m_EmissionTimer > m_EmissionDelay )
    {
        m_EmissionTimer -= m_EmissionDelay;
        CreateParticle();

        if ( m_ParticlesToSpawn != sInfiniteParticles )
        {
            m_ParticlesToSpawn--;
        }
    }

    int aliveParticles = 0;
    for ( auto& particle : m_Particles )
    {
        if ( particle.IsAlive() )
        {
            particle.Update( delta );
            aliveParticles++;
        }
    }
    m_Active = ( aliveParticles != 0 || m_ParticlesToSpawn > 0 || m_ParticlesToSpawn == sInfiniteParticles );
}

void ParticleEmitter::CreateParticle()
{
    Particle* pParticle = GetAvailableParticle();
    pParticle->SetLifetime( gRand( m_MinLifetime, m_MaxLifetime ) );
    pParticle->SetPosition( m_Position );
    pParticle->SetScale( gRand( m_MinScale, m_MaxScale ) );
    pParticle->SetVelocity( m_Velocity );
}

Particle* ParticleEmitter::GetAvailableParticle()
{
    size_t numEmitters = m_Particles.size();
    for ( size_t i = m_Idx; i < numEmitters; ++i )
    {
        if ( m_Particles[ i ].IsAlive() == false )
        {
            m_Idx = i;
            m_Particles[ i ].Reset();
            return &m_Particles[ i ];
        }
    }

    for ( size_t i = 0; i < m_Idx; ++i )
    {
        if ( m_Particles[ i ].IsAlive() == false )
        {
            m_Idx = i;
            m_Particles[ i ].Reset();
            return &m_Particles[ i ];
        }
    }

    // If all particles have been used, override the first one
    m_Particles[ 0 ].Reset();
    return &m_Particles[ 0 ];
}

void ParticleEmitter::SetTexture( const std::string& filename )
{
    m_pTexture = (Genesis::ResourceImage*)Genesis::FrameWork::GetResourceManager()->GetResource( filename );
    m_UsingAtlas = false;
}

void ParticleEmitter::SetTextureAtlas( const std::string& filename, int elementWidth, int elementHeight, int elementCount )
{
    if ( m_pTexture != NULL )
    {
        return;
    }

    Genesis::ResourceImage* pTexture = (Genesis::ResourceImage*)Genesis::FrameWork::GetResourceManager()->GetResource( filename );
    m_pTexture = pTexture;
    if ( pTexture != m_Atlas.GetSource() )
    {
        m_Atlas.SetSource( m_pTexture );

        float x = 0.0f;
        float y = 0.0f;
        for ( int i = 0; i < elementCount; ++i )
        {
            m_Atlas.AddElement( x, y, x + (float)elementWidth, y + (float)elementHeight );

            x += (float)elementWidth;

            if ( (uint32_t)( x + elementWidth ) > m_pTexture->GetWidth() )
            {
                x = 0.0f;
                y += (float)elementHeight;
            }
        }
    }

    m_UsingAtlas = true;
}

void ParticleEmitter::SetParticleCount( int value )
{
    m_ParticlesToSpawn = value;

    int bufferSize = ( value == -1 ) ? 128 : value;
    m_Particles.resize( bufferSize );
    for ( int i = 0; i < bufferSize; ++i )
    {
        m_Particles[ i ].Reset();
    }
    m_Idx = 0;
}

const std::string& ParticleEmitter::GetRandomExplosion()
{
    static const int sFxVariants = 3;
    static const std::string atlas[ sFxVariants ] = {
        "data/particles/Oil_Rig_Sprites_v1.png",
        "data/particles/Oil_Rig_Sprites_v2.png",
        "data/particles/Oil_Rig_Sprites_v3.png"
    };
    return atlas[ rand() % sFxVariants ];
}

} // namespace Hexterminate