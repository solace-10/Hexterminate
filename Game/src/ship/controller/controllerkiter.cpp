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

#include "ship/controller/controllerkiter.h"
#include "hexterminate.h"
#include "menus/shiptweaks.h"
#include "sector/sector.h"
#include "ship/ship.h"

#include <render/debugrender.h>

namespace Hexterminate
{

ControllerKiter::ControllerKiter( Ship* pShip )
    : ControllerAI( pShip )
{
}

void ControllerKiter::HandleOrders( float delta )
{
    ControllerAI::HandleOrders( delta );

    if ( GetShip()->GetFleetCommandOrder() == FleetCommandOrder::Engage && GetTargetShip() )
    {
        OrbitTargetShip();
    }
}

bool ControllerKiter::OrbitTargetShip()
{
    if ( GetTargetShip() == nullptr || HasWeapons() == false )
    {
        return false;
    }

    // Check the tower modules are valid, although this should always be the case as GetTargetShip() should have returned null otherwise.
    if ( GetTargetShip()->GetTowerModule() == nullptr || GetShip()->GetTowerModule() == nullptr )
    {
        return false;
    }

    const glm::vec2 shipPosition( GetShip()->GetTowerPosition() );
    const glm::vec2 targetShipPosition( GetTargetShip()->GetTowerPosition() );
    const glm::vec2 positionDelta = targetShipPosition - shipPosition;
    const float weaponRange = GetMinimumWeaponRange();
    const float orbitTighteningMult = 0.9f; // Tightening the orbit helps keep the ships in range as they drift due to inertia.
    const glm::vec2 orbitPosition = targetShipPosition + glm::normalize( glm::vec2( -positionDelta.y, positionDelta.x ) ) * weaponRange * orbitTighteningMult;

    if ( g_pGame->GetCurrentSector()->GetShipTweaks()->GetDrawNavigation() )
    {
        Genesis::FrameWork::GetDebugRender()->DrawCircle( targetShipPosition, weaponRange, glm::vec3( 0.0f, 1.0f, 1.0f ) );
        Genesis::FrameWork::GetDebugRender()->DrawCircle( orbitPosition, 5.0f, glm::vec3( 0.0f, 1.0f, 0.0f ) );
        Genesis::FrameWork::GetDebugRender()->DrawLine( shipPosition, targetShipPosition, glm::vec3( 1.0f, 0.0f, 0.0f ) );
        Genesis::FrameWork::GetDebugRender()->DrawLine( shipPosition, orbitPosition, glm::vec3( 0.0f, 1.0f, 0.0f ) );
    }

    MoveToPosition( orbitPosition, 0.0f );

    return true;
}

} // namespace Hexterminate