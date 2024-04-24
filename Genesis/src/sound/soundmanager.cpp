// Copyright 2015 Pedro Nunes
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

#include "sound/soundmanager.h"

// clang-format off
#include "beginexternalheaders.h"
#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>
#include "endexternalheaders.h"
// clang-format on

#include "resources/resourceplaylist.h"
#include "resources/resourcesound.h"
#include "sound/soundbus.h"
#include "sound/soundinstance.h"
#include "sound/window.h"
#include "configuration.h"
#include "genesis.h"

namespace Genesis::Sound
{

std::unique_ptr<SoLoud::Soloud> g_pSoloud = nullptr;

SoundManager::SoundManager()
: m_Initialized( false )
, m_ListenerPosition( 0.0f )
, m_pPlaylist( nullptr )
, m_PlaylistShuffle( false )
{
    g_pSoloud = std::make_unique<SoLoud::Soloud>();
    int result = g_pSoloud->init();
    if ( result == ::SoLoud::SO_NO_ERROR )
    {
        m_Initialized = true;
        g_pSoloud->setMaxActiveVoiceCount( 64u );

        for ( size_t i = 0; i < static_cast<size_t>( SoundBus::Type::Count ); ++i )
        {
            m_Buses[ i ] = std::make_shared<SoundBus>( static_cast<SoundBus::Type>( i ) );
        }
    }
    else
    {
        Genesis::FrameWork::GetLogger()->LogWarning( "Failed to initialize SoLoud audio library: %s [%s]", g_pSoloud->getErrorString( result ), SDL_GetError() );
    }

    m_pDebugWindow = std::make_unique<Window>(this);
}

SoundManager::~SoundManager()
{
    if ( m_Initialized && g_pSoloud )
    {
        g_pSoloud->stopAll();
        g_pSoloud->deinit();
    }
}

TaskStatus SoundManager::Update( float delta )
{

    if ( m_Initialized )
    {
        g_pSoloud->update3dAudio();
        m_SoundInstances.remove_if( []( const SoundInstanceSharedPtr& pInstance ) { return !pInstance->IsValid(); } );
        UpdatePlaylist();
        UpdateVolumes();
    }

    m_pDebugWindow->Update(delta);
    return TaskStatus::Continue;
}

SoundInstanceSharedPtr SoundManager::CreateSoundInstance( ResourceSound* pResourceSound, SoundBus::Type bus, std::optional<glm::vec3> position /* = std::nullopt */, float minDistance /* = 0.0f */, float maxDistance /* = 10000.0f */ )
{
    if ( m_Initialized == false )
    {
        return nullptr;
    }
    else if ( pResourceSound->Is3D() && position.has_value() == false )
    {
        FrameWork::GetLogger()->LogWarning( "Attempting to play ResourceSound '%s' has a 3D sound with no position.", pResourceSound->GetFilename().GetFullPath().c_str() );
        return nullptr;
    }
    else if ( pResourceSound->CanInstance() == false )
    {
        return nullptr;
    }
    else
    {
        ::SoLoud::AudioSource* pAudioSourceRaw = nullptr;
        auto audioSourceIt = m_AudioSources.find( pResourceSound->GetFilename().GetFullPath() );
        if ( audioSourceIt == m_AudioSources.end() )
        {
            int result = SoLoud::UNKNOWN_ERROR;
            std::shared_ptr<SoLoud::AudioSource> pAudioSource;
            if ( pResourceSound->IsStreamed() )
            {
                pAudioSource = std::make_shared<::SoLoud::WavStream>();
                pAudioSourceRaw = pAudioSource.get();
                result = reinterpret_cast<::SoLoud::WavStream*>( pAudioSourceRaw )->load( pResourceSound->GetFilename().GetFullPath().c_str() );
            }
            else
            {
                pAudioSource = std::make_shared<::SoLoud::Wav>();
                pAudioSourceRaw = pAudioSource.get();
                result = reinterpret_cast<::SoLoud::Wav*>( pAudioSourceRaw )->load( pResourceSound->GetFilename().GetFullPath().c_str() );
            }

            if ( result == SoLoud::SO_NO_ERROR )
            {
                // The attenuation model has to be explicitly set, as the default is not to attenuate over distance.
                if ( pResourceSound->Is3D() )
                {
                    pAudioSource->set3dAttenuation( SoLoud::AudioSource::INVERSE_DISTANCE, 1.0f );
                }

                m_AudioSources[ pResourceSound->GetFilename().GetFullPath().c_str() ] = pAudioSource;
            }
            else
            {
                Genesis::FrameWork::GetLogger()->LogError( "SoundManager::CreateSoundInstance ('%s'): %s", pResourceSound->GetFilename().GetFullPath().c_str(), g_pSoloud->getErrorString( result ) );
                return nullptr;
            }
        }
        else
        {
            pAudioSourceRaw = audioSourceIt->second.get();
        }

        SoundInstanceSharedPtr pInstance = std::make_shared<SoundInstance>();
        SoundBusSharedPtr pSoundBus = m_Buses[ static_cast<size_t>( bus ) ];
        pInstance->Initialise( pResourceSound, pSoundBus, pAudioSourceRaw, position, minDistance, maxDistance );
        m_SoundInstances.push_back( pInstance );
        pResourceSound->SetInstancingTimePoint();
        return pInstance;
    }
}

void SoundManager::SetPlaylist( ResourcePlaylist* pResourcePlaylist, bool shuffle )
{
    if ( pResourcePlaylist == m_pPlaylist )
    {
        return;
    }

    // If we're changing playlists, stop the previous one.
    // There's no need to start playing the first track here, that will be handled in the next Update().
    m_pPlaylist = pResourcePlaylist;
    m_PlaylistShuffle = shuffle;

    if ( m_pCurrentTrack != nullptr && m_pCurrentTrack->IsValid() )
    {
        m_pCurrentTrack->Stop();
        m_pCurrentTrack = nullptr;
    }
}

ResourcePlaylist* SoundManager::GetPlaylist() const
{
    return m_pPlaylist;
}

SoundInstanceSharedPtr SoundManager::GetCurrentTrack() const
{
    return m_pCurrentTrack;
}

const SoundInstanceList& SoundManager::GetSoundInstances() const
{
    return m_SoundInstances;
}

void SoundManager::SetListener( const glm::vec3& position, const glm::vec3& velocity, const glm::vec3& forward, const glm::vec3& up )
{
    if ( m_Initialized == false )
    {
        return;
    }

    g_pSoloud->set3dListenerPosition( position.x, position.y, position.z );
    g_pSoloud->set3dListenerVelocity( velocity.x, velocity.y, velocity.z );
    g_pSoloud->set3dListenerAt( forward.x, forward.y, forward.z );
    g_pSoloud->set3dListenerUp( up.x, up.y, up.z );
    m_ListenerPosition = position;
}

glm::vec3 SoundManager::GetListenerPosition() const
{
    return m_ListenerPosition;
}

unsigned int SoundManager::GetActiveSoundCount() const
{
    return g_pSoloud->getActiveVoiceCount();
}

unsigned int SoundManager::GetMaximumSoundCount() const
{
    return g_pSoloud->getMaxActiveVoiceCount();
}

unsigned int SoundManager::GetVirtualSoundCount() const
{
    return g_pSoloud->getVoiceCount();
}

void SoundManager::UpdatePlaylist()
{
    ResourcePlaylist* pPlaylist = GetPlaylist();
    if ( pPlaylist )
    {
        if ( m_pCurrentTrack == nullptr || m_pCurrentTrack->IsValid() == false )
        {
            ResourceSound* pNextTrackResource = pPlaylist->GetNextTrack( m_PlaylistShuffle );
            if ( pNextTrackResource )
            {
                m_pCurrentTrack = FrameWork::GetSoundManager()->CreateSoundInstance( pNextTrackResource, SoundBus::Type::Music );
            }
        }
    }
}

void SoundManager::UpdateVolumes()
{
    if ( m_Initialized == false )
    {
        return;
    }

    const float sfxVolume = static_cast<float>( Configuration::GetSFXVolume() / 100.0f );
    SDL_assert( sfxVolume >= 0.0f && sfxVolume <= 1.0f );
    m_Buses[ static_cast<size_t>( SoundBus::Type::SFX ) ]->SetVolume( sfxVolume );

    const float musicVolume = static_cast<float>( Configuration::GetMusicVolume() / 100.0f );
    SDL_assert( musicVolume >= 0.0f && musicVolume <= 1.0f );
    m_Buses[ static_cast<size_t>( SoundBus::Type::Music ) ]->SetVolume( musicVolume );

    const float masterVolume = static_cast<float>( Configuration::GetMasterVolume() / 100.0f );

    for ( auto& pSoundInstance : m_SoundInstances )
    {
        float volume = pSoundInstance->GetVolume() * pSoundInstance->GetSoundBus()->GetVolume() * masterVolume;
        g_pSoloud->setVolume( pSoundInstance->m_Handle, volume );
    }
}

} // namespace Genesis::Sound
