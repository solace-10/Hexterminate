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

#pragma once

#include "laser/laser.h"
#include "ship/addon/addon.h"

namespace Genesis
{
class ResourceSound;
}

namespace Hexterminate
{

class Ammo;

class AddonMissileInterceptor : public Addon
{
public:
    AddonMissileInterceptor( AddonModule* pModule, Ship* pOwner );
    virtual ~AddonMissileInterceptor(){};

    virtual void Update( float delta ) override;

private:
    bool CanIntercept() const;
    Ammo* FindClosestMissile() const;
    void InterceptMissile( Ammo* pMissile );
    void LoadSFX();
    void PlaySFX();

    float m_ReloadDuration;
    float m_ReloadTimer;
    float m_LaserTimer;
    Laser m_Laser;

    Genesis::ResourceSound* m_pSFX;
};

} // namespace Hexterminate
