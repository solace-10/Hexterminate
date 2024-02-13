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

#include "components/shieldinhibitorcomponent.h"

#include "hexterminate.h"
#include "sector/sector.h"
#include "ship/shield.h"

namespace Hexterminate
{

IMPLEMENT_COMPONENT( ShieldInhibitorComponent )

static const float sShieldInhibitorTimer = 15.0f;

bool ShieldInhibitorComponent::Initialise()
{
    m_Timer = sShieldInhibitorTimer;

    g_pGame->AddIntel(
        GameCharacter::FleetIntelligence,
        "Due to the proximity of a black hole, shields will not work properly in this sector." );

    return true;
}

void ShieldInhibitorComponent::Update( float delta )
{
    if ( m_Timer <= 0.0f )
    {
        const ShipList& ships = g_pGame->GetCurrentSector()->GetShipList();
        for ( auto& pShip : ships )
        {
            Shield* pShield = pShip->GetShield();
            if ( pShield != nullptr && pShield->GetQuantumState() == ShieldState::Activated )
            {
                pShield->Deactivate();
            }
        }

        m_Timer = sShieldInhibitorTimer;
    }
    else
    {
        m_Timer -= delta;
    }
}

} // namespace Hexterminate