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

typedef std::vector<Ammo*> AmmoVector;

///////////////////////////////////////////////////////////////////////////////
// AmmoManager
// Circular buffer containing all bullets, missiles, beams, etc flying around
// in space.
///////////////////////////////////////////////////////////////////////////////

static const unsigned int AmmoManagerCapacity = 1024;

class AmmoManager : public Genesis::SceneObject
{
public:
    AmmoManager();
    virtual ~AmmoManager() override;

    Ammo* Create( Weapon* pWeapon, float additionalRotation = 0.0f );
    virtual void Update( float delta ) override;
    virtual void Render() override;

    void GetInterceptables( AmmoVector& vec ) const;

private:
    Ammo** GetFreeAmmo();
    void CreateHitEffect( const glm::vec3& position, const glm::vec3& hitNormal, Weapon* pWeapon );
    void PlayHitSFX( const glm::vec3& position, Weapon* pWeapon );

    int m_Idx;
    Ammo* m_pAmmo[ AmmoManagerCapacity ];
    Genesis::Physics::RayTestResultVector m_RayTestResults;
};

} // namespace Hexterminate