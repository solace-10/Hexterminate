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

#pragma once

#include <vector>

#include <gui/atlas.h>
#include <rendersystem.h>

#include "particles/particle.h"

namespace Genesis
{
class ResourceImage;
}

namespace Hexterminate
{

class Particle;
typedef std::vector<Particle> ParticleVector;
typedef std::vector<const Particle*> ParticlePointerVector;

static const int sInfiniteParticles = -1;

///////////////////////////////////////////////////////////////////////////////
// ParticleEmitter
///////////////////////////////////////////////////////////////////////////////

class ParticleEmitter
{
public:
    ParticleEmitter();
    ~ParticleEmitter();

    void Start();
    void Stop();
    void Reset();

    void SetTexture( const std::string& filename );
    void SetTextureAtlas( const std::string& filename, int elementWidth, int elementHeight, int elementCount );
    void SetBlendMode( Genesis::BlendMode blendMode );
    void SetParticleCount( int value );
    void SetEmissionDelay( float value );
    void SetPosition( const glm::vec3& position );
    void SetGlowEnabled( bool state );
    void SetScale( float minScale, float maxScale );
    void SetLifetime( float minLifetime, float maxLifetime );
    void SetVelocity( const glm::vec3& velocity );

    Genesis::BlendMode GetBlendMode() const;
    bool GetGlowEnabled() const;
    Genesis::ResourceImage* GetTexture() const;
    const ParticleVector& GetParticles() const;

    void Update( float delta );
    bool IsActive() const;

    const Genesis::Gui::Atlas* GetAtlas() const;

    static const std::string& GetRandomExplosion();

private:
    void CreateParticle();
    Particle* GetAvailableParticle();

    ParticleVector m_Particles;
    size_t m_Idx;
    bool m_Active;
    bool m_UsingAtlas;
    bool m_GlowEnabled;
    int m_ParticlesToSpawn;
    float m_EmissionDelay;
    float m_EmissionTimer;
    glm::vec3 m_Position;
    float m_MinScale;
    float m_MaxScale;
    float m_MinLifetime;
    float m_MaxLifetime;
    glm::vec3 m_Velocity;

    Genesis::Gui::Atlas m_Atlas;
    Genesis::BlendMode m_BlendMode;
    Genesis::ResourceImage* m_pTexture;
};

inline void ParticleEmitter::Start()
{
    m_Active = true;
}

inline bool ParticleEmitter::IsActive() const
{
    return m_Active;
}

inline void ParticleEmitter::SetEmissionDelay( float value )
{
    m_EmissionDelay = value;
}

inline Genesis::BlendMode ParticleEmitter::GetBlendMode() const
{
    return m_BlendMode;
}

inline void ParticleEmitter::SetPosition( const glm::vec3& position )
{
    m_Position = position;
}

inline void ParticleEmitter::SetBlendMode( Genesis::BlendMode mode )
{
    m_BlendMode = mode;
}

inline Genesis::ResourceImage* ParticleEmitter::GetTexture() const
{
    return m_pTexture;
}

inline const ParticleVector& ParticleEmitter::GetParticles() const
{
    return m_Particles;
}

inline const Genesis::Gui::Atlas* ParticleEmitter::GetAtlas() const
{
    return &m_Atlas;
}

inline void ParticleEmitter::SetScale( float minScale, float maxScale )
{
    m_MinScale = minScale;
    m_MaxScale = maxScale;
}

inline void ParticleEmitter::SetLifetime( float minLifetime, float maxLifetime )
{
    m_MinLifetime = minLifetime;
    m_MaxLifetime = maxLifetime;
}

inline void ParticleEmitter::SetVelocity( const glm::vec3& velocity )
{
    m_Velocity = velocity;
}

} // namespace Hexterminate
