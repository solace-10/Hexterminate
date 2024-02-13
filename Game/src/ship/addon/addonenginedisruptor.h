// Copyright 2016 Pedro Nunes
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

#include "laser/laser.h"
#include "ship/addon/addon.h"

#include <array>

namespace Genesis
{
class ResourceSound;
}

namespace Hexterminate
{

static const unsigned int sEngineDisruptorShockwaveCount = 3;
static const float sEngineDisruptorShockwaveDuration = 1.25f; // in seconds
static const float sEngineDisruptorLaserDuration = 1.0f; // in seconds

class AddonEngineDisruptor : public Addon
{
public:
    AddonEngineDisruptor( AddonModule* pModule, Ship* pOwner );
    virtual ~AddonEngineDisruptor(){};

    virtual void Activate() override;
    virtual void Update( float delta ) override;

private:
    void LoadSFX();
    void PlaySFX();
    void UpdateFiring( float delta );
    void UpdateLaser( float delta );
    void UpdateShockwave( float delta );

    struct DisruptorLaser
    {
        float m_Timer;
        Laser m_Laser;
        glm::vec3 m_Target;
    };

    std::array<DisruptorLaser, sEngineDisruptorShockwaveCount> m_DisruptorLasers;
    std::array<float, sEngineDisruptorShockwaveCount> m_ShockwaveTimers;

    Genesis::ResourceSound* m_pSFX;
    float m_ShockwaveMaximumRadius;
    float m_ShockwaveParticleScale;
    glm::vec3 m_DisruptorAnchor;
    int m_TimesToFire;
    float m_TimeToNextShot;
};

} // namespace Hexterminate
