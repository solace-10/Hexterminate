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

#include <cassert>

#include <math/constants.h>
#include <resources/resourcemodel.h>

#include "ammo/projectile.h"
#include "hexterminate.h"
#include "muzzleflash/muzzleflashmanager.h"
#include "sector/sector.h"

namespace Hexterminate
{

Projectile::Projectile()
    : m_pModel( nullptr )
{
}

void Projectile::Create( Weapon* pWeapon, float additionalRotation /* = 0.0f */ )
{
    Ammo::Create( pWeapon, additionalRotation );

    m_pModel = (Genesis::ResourceModel*)Genesis::FrameWork::GetResourceManager()->GetResource( "data/models/ammo/projectile.tmf" );
    m_pModel->SetFlipAxis( false );

    m_IsGlowSource = true;

    float muzzleflashScale = pWeapon->GetInfo()->GetMuzzleflashScale();
    g_pGame->GetCurrentSector()->GetMuzzleflashManager()->Add(
        MuzzleflashData( pWeapon, m_MuzzleOffset, 0, gRand( muzzleflashScale * 0.8f, muzzleflashScale * 1.2f ), gRand( 0.075f, 0.125f ) ) );
}

void Projectile::Update( float delta )
{
    if ( m_Range <= 0.0f )
    {
        Kill();
        m_pOwner->ResetSiegebreakerStacks();
    }
    else
    {
        m_Src += m_Dir * m_Speed * delta;
        m_Dst += m_Dir * m_Speed * delta;
        m_Range -= m_Speed * delta;
    }
}

void Projectile::Render()
{
    using namespace glm;
    using namespace Genesis;

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Add );

    const mat4 translation = translate( m_Src );
    const mat4 rotation = rotate( mat4( 1.0f ), -m_Angle + 180.0f * Genesis::kDegToRad, vec3( 0.0f, 0.0f, 1.0f ) );
    const mat4 scaling = scale( glm::vec3( 8.0f, m_RayLength, 1.0f ) );

    m_pModel->Render( translation * rotation * scaling );

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Disabled );
}

} // namespace Hexterminate