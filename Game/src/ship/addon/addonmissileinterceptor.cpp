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

#include "ship/addon/addonmissileinterceptor.h"
#include "ammo/ammo.h"
#include "ammo/ammomanager.h"
#include "faction/faction.h"
#include "hexterminate.h"
#include "laser/laser.h"
#include "laser/lasermanager.h"
#include "sector/sector.h"

#include <genesis.h>
#include <resources/resourcesound.h>
#include <sound/soundinstance.h>
#include <sound/soundmanager.h>

#include <algorithm>

namespace Hexterminate
{

AddonMissileInterceptor::AddonMissileInterceptor( AddonModule* pModule, Ship* pOwner )
    : Addon( pModule, pOwner )
    , m_ReloadDuration( 1.0f )
    , m_ReloadTimer( 0.0f )
    , m_LaserTimer( 0.0f )
    , m_pSFX( nullptr )
{
    AddonInfo* pAddonInfo = static_cast<AddonInfo*>( pModule->GetModuleInfo() );
    m_ReloadDuration = static_cast<float>( atof( pAddonInfo->GetParameter().c_str() ) );

    LoadSFX();
}

void AddonMissileInterceptor::LoadSFX()
{
    using namespace Genesis;
    using namespace std::literals;

    m_pSFX = (ResourceSound*)FrameWork::GetResourceManager()->GetResource( "data/sfx/missile_interceptor.wav" );
    if ( m_pSFX != nullptr )
    {
        m_pSFX->Initialise( SOUND_FLAG_3D | SOUND_FLAG_FX );
        m_pSFX->SetInstancingLimit( 100ms );
    }
}

void AddonMissileInterceptor::Update( float delta )
{
    Addon::Update( delta );

    m_ReloadTimer = std::max( m_ReloadTimer - delta, 0.0f );

    if ( CanIntercept() )
    {
        Ammo* pClosestMissile = FindClosestMissile();
        if ( pClosestMissile != nullptr )
        {
            InterceptMissile( pClosestMissile );
        }
    }

    if ( m_LaserTimer > 0.0f )
    {
        LaserManager* pLaserManager = g_pGame->GetCurrentSector()->GetLaserManager();
        pLaserManager->AddLaser( m_Laser );
        m_LaserTimer -= delta;
    }
}

bool AddonMissileInterceptor::CanIntercept() const
{
    return IsActive() && ( m_ReloadTimer <= 0.0f );
}

Ammo* AddonMissileInterceptor::FindClosestMissile() const
{
    AmmoVector vec;
    vec.reserve( 128 );

    Sector* pCurrentSector = g_pGame->GetCurrentSector();
    if ( pCurrentSector == nullptr )
        return nullptr;

    AmmoManager* pAmmoManager = pCurrentSector->GetAmmoManager();
    pAmmoManager->GetInterceptables( vec );

    Ammo* pClosestMissile = nullptr;
    float closestDistance = FLT_MAX;

    glm::vec3 moduleWorldPos = m_pModule->GetWorldPosition();
    Faction* pShipFaction = m_pModule->GetOwner()->GetFaction();
    Faction* pMissileFaction = nullptr;
    float distance = 0.0f;

    for ( auto& pAmmo : vec )
    {
        pMissileFaction = pAmmo->GetOwner()->GetOwner()->GetFaction();
        if ( Faction::sIsEnemyOf( pShipFaction, pMissileFaction ) )
        {
            distance = glm::distance( pAmmo->GetSource(), moduleWorldPos );
            if ( distance < closestDistance )
            {
                pClosestMissile = pAmmo;
                closestDistance = distance;
            }
        }
    }

    const float maxDistance = 400.0f;
    if ( closestDistance > maxDistance )
        return nullptr;
    else
        return pClosestMissile;
}

void AddonMissileInterceptor::InterceptMissile( Ammo* pMissile )
{
    pMissile->Intercept();

    m_Laser = Laser(
        m_pModule->GetWorldPosition(),
        pMissile->GetSource(),
        Genesis::Color( 1.0f, 0.0f, 0.0f ),
        3.0f );

    m_ReloadTimer = m_ReloadDuration;
    m_LaserTimer = std::min( 0.15f, m_ReloadTimer );

    PlaySFX();
}

void AddonMissileInterceptor::PlaySFX()
{
    if ( m_pSFX )
    {
        using namespace Genesis;

        Sound::SoundInstanceSharedPtr pSoundInstance = FrameWork::GetSoundManager()->CreateSoundInstance( m_pSFX, Genesis::Sound::SoundBus::Type::SFX, m_pModule->GetWorldPosition(), 300.0f );
    }
}

} // namespace Hexterminate