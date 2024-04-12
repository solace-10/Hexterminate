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

#include <genesis.h>
#include <physics/raytestresult.h>
#include <physics/simulation.h>
#include <resourcemanager.h>
#include <scene/sceneobject.h>
#include <vector>

namespace Genesis
{
class ResourceModel;
}

namespace Hexterminate
{

class Ammo;
class Weapon;

using AmmoVector = std::vector<Ammo*>;
using AmmoHandle = int32_t;
static const AmmoHandle InvalidAmmoHandle = -1;

///////////////////////////////////////////////////////////////////////////////
// AmmoManager
// Circular buffer containing all bullets, missiles, beams, etc flying around
// in space.
///////////////////////////////////////////////////////////////////////////////

class AmmoManager : public Genesis::SceneObject
{
public:
    AmmoManager();
    virtual ~AmmoManager() override;

    AmmoHandle Create( Weapon* pWeapon, float additionalRotation = 0.0f );
    virtual void Update( float delta ) override;
    virtual void Render() override;

    void GetInterceptables( AmmoVector& vec ) const;

    // Do not hold on to this pointer as it is owned by the AmmoManager and can be invalidated.
    Ammo* Get( AmmoHandle handle ) const;

private:
    using AmmoSizeType = AmmoHandle;

    AmmoHandle GetFreeAmmo();
    void CreateHitEffect( const glm::vec3& position, const glm::vec3& hitNormal, Weapon* pWeapon );
    void PlayHitSFX( const glm::vec3& position, Weapon* pWeapon );

    AmmoSizeType m_Idx;
    AmmoVector m_Ammo;
    Genesis::Physics::RayTestResultVector m_RayTestResults;
};

inline Ammo* AmmoManager::Get( AmmoHandle handle ) const
{
    SDL_assert( handle != InvalidAmmoHandle );
    SDL_assert( handle >= 0 );
    SDL_assert( handle < m_Ammo.size() );
    return m_Ammo[ handle ];
}

} // namespace Hexterminate