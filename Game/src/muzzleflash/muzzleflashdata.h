// Copyright 2019 Pedro Nunes
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

#include <glm/vec3.hpp>

#include <math/misc.h>

namespace Hexterminate
{

class Weapon;

class MuzzleflashData
{
public:
    MuzzleflashData( Weapon* pWeapon, const glm::vec3& muzzleOffset, int muzzleIndex, float scale, float lifetime );
    Weapon* GetWeapon() const;
    const glm::vec3& GetMuzzleOffset() const;
    int GetMuzzleIndex() const;
    float GetScale() const;
    float GetLifetime() const;
    void SetLifetime( float value );
    float GetRotationMultiplier() const;

private:
    Weapon* m_pWeapon;
    glm::vec3 m_MuzzleOffset;
    int m_MuzzleIndex;
    float m_Scale;
    float m_Lifetime;
    float m_RotationMultiplier;
};

inline MuzzleflashData::MuzzleflashData( Weapon* pWeapon, const glm::vec3& muzzleOffset, int muzzleIndex, float scale, float lifetime )
    : m_pWeapon( pWeapon )
    , m_MuzzleOffset( muzzleOffset )
    , m_MuzzleIndex( muzzleIndex )
    , m_Scale( scale )
    , m_Lifetime( lifetime )
{
    m_RotationMultiplier = gRand( -50.0f, 50.0f );
}

inline Weapon* MuzzleflashData::GetWeapon() const
{
    return m_pWeapon;
}

inline const glm::vec3& MuzzleflashData::GetMuzzleOffset() const
{
    return m_MuzzleOffset;
}

inline int MuzzleflashData::GetMuzzleIndex() const
{
    return m_MuzzleIndex;
}

inline float MuzzleflashData::GetScale() const
{
    return m_Scale;
}

inline float MuzzleflashData::GetLifetime() const
{
    return m_Lifetime;
}

inline void MuzzleflashData::SetLifetime( float value )
{
    m_Lifetime = value;
}

inline float MuzzleflashData::GetRotationMultiplier() const
{
    return m_RotationMultiplier;
}

} // namespace Hexterminate
