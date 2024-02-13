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

#include "ship/ship.h"
#include "ship/weapon.h"
#include <genesis.h>
#include <resourcemanager.h>

namespace Genesis
{
class ResourceModel;
}

namespace Hexterminate
{

class Weapon;

///////////////////////////////////////////////////////////////////////////////
// Ammo
///////////////////////////////////////////////////////////////////////////////

class Ammo
{
public:
    Ammo();
    virtual ~Ammo(){};

    virtual void Create( Weapon* pWeapon, float additionalRotation = 0.0f );
    virtual void Update( float delta ) = 0;
    virtual void Render() = 0;
    bool IsAlive() const;
    virtual void Kill();
    virtual bool CanBeIntercepted() const;
    virtual bool CanBypassShields() const;
    void Intercept();
    bool WasIntercepted() const;

    const glm::vec3& GetSource() const;
    const glm::vec3& GetDestination() const;

    uint32_t GetCollisionFilter() const;
    Weapon* GetOwner() const;

    bool IsGlowSource() const;
    bool GetDiesOnHit() const;

    void SetHitFraction( float value );
    float GetHitFraction() const;

protected:
    Weapon* m_pOwner;
    bool m_IsAlive;
    bool m_IsGlowSource;
    bool m_DiesOnHit;
    bool m_Intercepted;
    float m_Angle;
    float m_AdditionalRotation;

    glm::vec3 m_Src;
    glm::vec3 m_Dst;
    glm::vec3 m_Dir;
    float m_Speed;
    float m_RayLength;
    float m_Range;
    float m_HitFraction;

    uint32_t m_CollisionFilter;

    glm::vec3 m_MuzzleOffset;
};

inline const glm::vec3& Ammo::GetSource() const
{
    return m_Src;
}

inline const glm::vec3& Ammo::GetDestination() const
{
    return m_Dst;
}

inline uint32_t Ammo::GetCollisionFilter() const
{
    return m_CollisionFilter;
}

inline Weapon* Ammo::GetOwner() const
{
    return m_pOwner;
}

inline bool Ammo::IsAlive() const
{
    return m_IsAlive;
}

inline bool Ammo::IsGlowSource() const
{
    return m_IsGlowSource;
}

inline void Ammo::SetHitFraction( float value )
{
    m_HitFraction = value;
}

inline float Ammo::GetHitFraction() const
{
    return m_HitFraction;
}

inline bool Ammo::GetDiesOnHit() const
{
    return m_DiesOnHit;
}

inline bool Ammo::CanBypassShields() const
{
    return false;
}

} // namespace Hexterminate
