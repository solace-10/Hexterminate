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

#include "ship/controller/controllerassault.h"
#include "hexterminate.h"
#include "ship/ship.h"

#include <render/debugrender.h>

namespace Hexterminate
{

ControllerAssault::ControllerAssault( Ship* pShip )
    : ControllerAI( pShip )
    , m_HasRammingProws( false )
{
    // Only ships equipped with ramming prows should attempt to use the Ramming Speed ability.
    for ( auto& pModule : pShip->GetModules() )
    {
        if ( pModule->GetModuleInfo()->GetType() == ModuleType::Armour )
        {
            ArmourInfo* pArmourInfo = reinterpret_cast<ArmourInfo*>( pModule->GetModuleInfo() );
            if ( pArmourInfo->IsRammingProw() )
            {
                m_HasRammingProws = true;
                break;
            }
        }
    }
}

void ControllerAssault::HandleOrders( float delta )
{
    ControllerAI::HandleOrders( delta );

    if ( GetShip()->GetFleetCommandOrder() == FleetCommandOrder::Engage && GetTargetShip() )
    {
        if ( ApproachTargetShip() ) // Have we managed to get in range of the ship?
        {
            FactionId factionId = GetShip()->GetFaction()->GetFactionId();

            if ( m_HasRammingProws && ( factionId == FactionId::Marauders || factionId == FactionId::Hegemon ) && GetShip()->GetRammingSpeedCooldown() <= 0.0f && GetShip()->AreEnginesDisrupted() == false )
            {
                GetShip()->RammingSpeed();
            }
        }
    }
}

bool ControllerAssault::ApproachTargetShip()
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

    // Move closer to the target ship in a straight line and stop when we are within our optimal range
    // This allows the ship to get close to its target, reducing its chance to miss.
    glm::vec3 targetPosition = GetTargetShip()->GetTowerPosition();
    const float weaponRange = GetMinimumWeaponRange() * 0.85f;
    return MoveToPosition( glm::vec2( targetPosition ), weaponRange );
}

} // namespace Hexterminate