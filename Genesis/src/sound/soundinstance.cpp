// Copyright 2021 Pedro Nunes
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

#include "sound/soundinstance.h"

#include <list>
#include <memory>

// clang-format off
#include "beginexternalheaders.h"
#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>
#include "endexternalheaders.h"
// clang-format on

#include "resources/resourcesound.h"
#include "configuration.h"
#include "genesis.h"

namespace Genesis::Sound
{

extern std::unique_ptr<SoLoud::Soloud> g_pSoloud;

SoundInstance::SoundInstance()
    : m_pResourceSound( nullptr )
    , m_Handle( 0 )
    , m_Position( 0.0f )
    , m_Velocity( 0.0f )
    , m_Volume( 1.0f )
{
}

SoundInstance::~SoundInstance()
{
}

void SoundInstance::Initialise( ResourceSound* pResourceSound, SoundBusSharedPtr& pSoundBus, void* pData, std::optional<glm::vec3> position /* = std::nullopt */, float minDistance /* = 0.0f */, float maxDistance /* = 0.0f */ )
{
    m_pResourceSound = pResourceSound;
    m_pSoundBus = pSoundBus;
    m_MinDistance = minDistance;
    m_MaxDistance = maxDistance;
    SoLoud::AudioSource* pAudioSource = reinterpret_cast<::SoLoud::AudioSource*>( pData );

    // Set sound properties based on the type of sound we're playing.
    if ( pSoundBus->GetType() == SoundBus::Type::SFX )
    {
        // A sound effect doesn't need to keep ticking in the background, but will be killed if it ends up in the inaudible list.
        pAudioSource->setInaudibleBehavior( false, true );
    }
    else if ( pSoundBus->GetType() == SoundBus::Type::Music )
    {
        // Music must always tick in the background.
        pAudioSource->setInaudibleBehavior( true, false );
    }

    SoLoud::Bus* pSoLoudBus = pSoundBus->m_pBus.get();
    const float volume = m_Volume * pSoundBus->GetVolume() * ( static_cast<float>( Configuration::GetMasterVolume() ) / 100.0f );
    if ( pResourceSound->Is3D() )
    {
        SDL_assert( position.has_value() ); // Shouldn't have really got here, this should have been caught by the SoundManager!
        glm::vec3 pos = position.value_or( glm::vec3( 0.0f ) );
        pAudioSource->set3dMinMaxDistance( minDistance, maxDistance );
        m_Handle = pSoLoudBus->play3d( *pAudioSource, pos.x, pos.y, pos.z, 0.0f, 0.0f, 0.0f, volume );
    }
    else
    {
        m_Handle = pSoLoudBus->play( *pAudioSource, volume );
    }

    if ( pResourceSound->IsLooping() )
    {
        g_pSoloud->setLooping( m_Handle, true );
    }
}

bool SoundInstance::IsPlaying() const
{
    return g_pSoloud->isValidVoiceHandle( m_Handle ) && !g_pSoloud->getPause( m_Handle );
}

bool SoundInstance::IsPaused() const
{
    return g_pSoloud->getPause( m_Handle );
}

void SoundInstance::Stop()
{
    g_pSoloud->stop( m_Handle );
}

bool SoundInstance::IsValid() const
{
    return g_pSoloud->isValidVoiceHandle( m_Handle );
}

unsigned int SoundInstance::GetLength() const
{
    return 0u;
}

unsigned int SoundInstance::GetPosition() const
{
    return 0u;
}

float SoundInstance::GetAudability() const
{
    return 0.0f;
}

ResourceSound* SoundInstance::GetResource() const
{
    return m_pResourceSound;
}

void SoundInstance::SetMinimumDistance( float value )
{
    m_MinDistance = value;
    g_pSoloud->set3dSourceMinMaxDistance( m_Handle, m_MinDistance, m_MaxDistance );
}

void SoundInstance::Set3DAttributes( const glm::vec3* pPosition, const glm::vec3* pVelocity )
{
    SDL_assert( GetResource()->Is3D() );

    if ( pPosition != nullptr )
    {
        g_pSoloud->set3dSourcePosition( m_Handle, pPosition->x, pPosition->y, pPosition->z );
    }
    else
    {
        g_pSoloud->set3dSourcePosition( m_Handle, 0.0f, 0.0f, 0.0f );
    }

    if ( pVelocity != nullptr )
    {
        g_pSoloud->set3dSourceVelocity( m_Handle, pVelocity->x, pVelocity->y, pVelocity->z );
    }
    else
    {
        g_pSoloud->set3dSourceVelocity( m_Handle, 0.0f, 0.0f, 0.0f );
    }
}

void SoundInstance::Get3DAttributes( glm::vec3* pPosition, glm::vec3* pVelocity )
{
    if ( pPosition != nullptr )
    {
        *pPosition = m_Position;
    }

    if ( pVelocity != nullptr )
    {
        *pVelocity = m_Velocity;
    }
}

} // namespace Genesis::Sound
