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

#include "moduleinfo.h"
#include <resources/resourcemodel.h>
#include <sound/soundmanager.h>
#include <vector>

namespace Hexterminate
{

class Weapon;
class Ship;
class Ammo;
class Module;

enum class WeaponFiringMode
{
    SingleShot,
    BurstFire,
    RandomBurstFire,
    Swarm,
    Continuous
};

struct WeaponHardpoint
{
    glm::vec3 offset;
    bool isTurret;
};

typedef std::vector<glm::vec3> MuzzleOffsets;

class Weapon
{
public:
    Weapon( Ship* pOwner, Module* pModule, WeaponInfo* pInfo, const WeaponHardpoint& hardpoint );
    virtual ~Weapon();
    virtual void Render( const glm::mat4& modelTransform );
    virtual void Update( float delta );

    bool IsTurnedTowards( const glm::vec3& position, float threshold = 10.0f ) const;
    virtual void TurnTowards( const glm::vec3& position );
    virtual void Fire();
    virtual void StopFiring();
    virtual bool IsFiring() const;
    virtual bool CanFire() const;

    Ship* GetOwner() const;
    float GetAngle() const;
    const WeaponHardpoint& GetHardpoint() const;
    const glm::vec3& GetTargetPosition() const;

    WeaponInfo* GetInfo() const;

    const glm::mat4x4& GetWorldTransform() const;
    const glm::vec3& GetMuzzleOffset( int index = -1 ) const;
    void MarkMuzzleAsUsed();

    Genesis::ResourceModel* GetModel() const;

    void AddSiegebreakerStack();
    void ResetSiegebreakerStacks();

protected:
    void FireSingle( float delta );
    void FireBurst( float delta );
    void FireRandomBurst( float delta );
    void FireSwarm( float delta );
    void FireContinuous( float delta );

    Genesis::Sound::SoundInstanceSharedPtr PlayFireSFX();
    void SetupMuzzles();
    void AddMuzzle( const glm::vec3& position );

    Ship* m_pOwner;
    Module* m_pModule;
    WeaponInfo* m_pInfo;
    WeaponHardpoint m_Hardpoint;
    Genesis::ResourceModel* m_pWeaponModel;

    float m_Angle; // Local angle to the ship
    float m_AngleWorld; // World angle of this weapon
    glm::vec3 m_TargetPosition; // Target position in world space
    float m_ReloadTimer; // Time until this weapon can fire again

    glm::mat4x4 m_WorldTransform;
    MuzzleOffsets m_MuzzleOffsets;
    unsigned int m_CurrentMuzzle;

    int m_BulletsToFire; // To fire multiple bullets with a single activation
    float m_BurstTimer;
    WeaponFiringMode m_FiringMode;

    bool m_IsFiring;
    Ammo* m_pCurrentAmmo;
    float m_RateOfFire;
    float m_ContinuousFireTimer;
    Genesis::Sound::SoundInstanceSharedPtr m_pContinuousSFX;
    unsigned int m_SiegebreakerStacks;
};

inline Ship* Weapon::GetOwner() const
{
    return m_pOwner;
}

inline const WeaponHardpoint& Weapon::GetHardpoint() const
{
    return m_Hardpoint;
}

inline bool Weapon::IsFiring() const
{
    return m_IsFiring;
}

inline bool Weapon::CanFire() const
{
    return ( m_ReloadTimer <= 0.0f );
}

inline WeaponInfo* Weapon::GetInfo() const
{
    return m_pInfo;
}

inline const glm::mat4x4& Weapon::GetWorldTransform() const
{
    return m_WorldTransform;
}

inline Genesis::ResourceModel* Weapon::GetModel() const
{
    return m_pWeaponModel;
}

inline const glm::vec3& Weapon::GetTargetPosition() const
{
    return m_TargetPosition;
}

inline const glm::vec3& Weapon::GetMuzzleOffset( int index /* = -1 */ ) const
{
    return m_MuzzleOffsets[ index == -1 ? m_CurrentMuzzle : index ];
}

inline void Weapon::AddSiegebreakerStack()
{
    m_SiegebreakerStacks++;
}

inline void Weapon::ResetSiegebreakerStacks()
{
    m_SiegebreakerStacks = 0;
}

typedef std::vector<Weapon*> WeaponList;

} // namespace Hexterminate
