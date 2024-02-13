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

#pragma once

#include <list>
#include <memory>
#include <optional>

#include "beginexternalheaders.h"
#include <glm/vec3.hpp>
#include "endexternalheaders.h"

#include "sound/soundbus.h"
#include "taskmanager.h"

namespace Genesis
{
    class ResourceSound;
}

namespace Genesis::Sound
{

class SoundInstance
{
public:
    friend class SoundManager;

    SoundInstance();
    ~SoundInstance();

    void Initialise( ResourceSound* pResourceSound, SoundBusSharedPtr& pSoundBus, void* pData, std::optional<glm::vec3> position = std::nullopt, float minDistance = 0.0f, float maxDistance = 10000.0f );
    bool IsPlaying() const;
    bool IsPaused() const;
    void Stop();
    bool IsValid() const;

    unsigned int GetLength() const; // Length in milliseconds
    unsigned int GetPosition() const; // Current position in milliseconds
    float GetAudability() const;

    ResourceSound* GetResource() const;

    void SetMinimumDistance( float value );
    void Set3DAttributes( const glm::vec3* pPosition = nullptr, const glm::vec3* pVelocity = nullptr );
    void Get3DAttributes( glm::vec3* pPosition = nullptr, glm::vec3* pVelocity = nullptr );
    void SetVolume( float value );
    float GetVolume() const;
    SoundBusSharedPtr& GetSoundBus();

private:
    Genesis::ResourceSound* m_pResourceSound;
    unsigned int m_Handle;
    glm::vec3 m_Position;
    glm::vec3 m_Velocity;
    float m_Volume;
    SoundBusSharedPtr m_pSoundBus;
    float m_MinDistance;
    float m_MaxDistance;
};

inline float SoundInstance::GetVolume() const
{
    return m_Volume;
}

inline void SoundInstance::SetVolume( float value )
{
    m_Volume = value;
}

inline SoundBusSharedPtr& SoundInstance::GetSoundBus()
{
    return m_pSoundBus;
}

} // namespace Genesis
