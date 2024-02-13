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

#include "sector/events/sectorevent.h"
#include "hexterminate.h"
#include "sector/sector.h"
#include "sector/sectorinfo.h"

namespace Hexterminate
{

SectorEvent::SectorEvent()
    : m_Name( "" )
    , m_FactionMask( 0 )
    , m_IsRepeatable( true )
    , m_MinimumPlayTime( 0 )
{
}

bool SectorEvent::IsAllowedForFaction( FactionId factionId ) const
{
    return ( m_FactionMask & ( 1 << (int)factionId ) );
}

void SectorEvent::AllowForFaction( FactionId factionId )
{
    m_FactionMask |= ( 1 << (int)factionId );
}

void SectorEvent::DisallowForFaction( FactionId factionId )
{
    m_FactionMask &= ~( 1 << (int)factionId );
}

bool SectorEvent::IsAvailableAt( const SectorInfo* pSectorInfo ) const
{
    if ( g_pGame->GetPlayedTime() < (float)m_MinimumPlayTime * 60.0f )
    {
        return false;
    }
    else
    {
        return IsAllowedForFaction( pSectorInfo->GetFaction()->GetFactionId() );
    }
}

void SectorEvent::HandleGameEvent( GameEvent* pEvent )
{
    if ( pEvent != nullptr && pEvent->GetType() == GameEventType::ShipDestroyed )
    {
        GameEventShipDestroyed* pGameEventShipDestroyed = static_cast<GameEventShipDestroyed*>( pEvent );
        OnShipDestroyed( pGameEventShipDestroyed->GetShip() );
    }
}

} // namespace Hexterminate