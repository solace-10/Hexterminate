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

#include <glm/gtc/matrix_access.hpp>

#include <math/constants.h>
#include <render/debugrender.h>
#include <resources/resourcemodel.h>

#include "ammo/ammo.h"
#include "hexterminate.h"
#include "misc/mathaux.h"
#include "sector/sector.h"
#include "ship/collisionmasks.h"

namespace Hexterminate
{

Ammo::Ammo()
    : m_pOwner( nullptr )
    , m_IsAlive( false )
    , m_IsGlowSource( false )
    , m_DiesOnHit( true )
    , m_Intercepted( false )
    , m_Speed( 0.0f )
    , m_RayLength( 0.0f )
    , m_CollisionFilter( 0 )
    , m_HitFraction( 1.0f )
    , m_AdditionalRotation( 0.0f )
{
    m_Src = glm::vec3( 0.0f );
    m_Dst = glm::vec3( 0.0f );
}

void Ammo::Create( Weapon* pWeapon, float additionalRotation /* = 0.0f */ )
{
    m_pOwner = pWeapon;
    m_IsAlive = true;
    m_AdditionalRotation = additionalRotation;
    m_Angle = pWeapon->GetAngle() + m_AdditionalRotation;

    glm::mat4x4 weaponTransform = pWeapon->GetWorldTransform();

    m_MuzzleOffset = pWeapon->GetMuzzleOffset();
    pWeapon->MarkMuzzleAsUsed();
    glm::mat4x4 muzzleOffsetTransform = glm::translate( m_MuzzleOffset );

    weaponTransform = weaponTransform * muzzleOffsetTransform;

    glm::vec3 weaponPosition( glm::column( weaponTransform, 3 ) );
    glm::vec3 weaponForward( glm::column( weaponTransform, 1 ) );
    Math::RotateVector( weaponForward, Genesis::kDegToRad * m_AdditionalRotation );

    m_RayLength = pWeapon->GetInfo()->GetRayLength();
    m_Speed = pWeapon->GetInfo()->GetSpeed();
    m_Src = weaponPosition;
    m_Dst = m_Src + weaponForward * m_RayLength;
    m_Dir = weaponForward;
    m_Range = pWeapon->GetInfo()->GetRange( pWeapon->GetOwner() );
}

void Ammo::Kill()
{
    if ( !m_IsAlive )
    {
        return;
    }

    m_IsAlive = false;
}

bool Ammo::CanBeIntercepted() const
{
    return false;
}

void Ammo::Intercept()
{
    if ( CanBeIntercepted() )
    {
        m_Intercepted = true;
    }
}

bool Ammo::WasIntercepted() const
{
    return m_Intercepted;
}

} // namespace Hexterminate