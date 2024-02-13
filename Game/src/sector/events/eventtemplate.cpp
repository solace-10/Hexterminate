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

#include "sector/events/eventtemplate.h"
#include "hexterminate.h"
#include "sector/sector.h"

namespace Hexterminate
{

SectorEventTemplate::SectorEventTemplate()
{
    SetName( "EventTemplate" );

    AllowForFaction( FactionId::Empire );
}

bool SectorEventTemplate::IsAvailableAt( const SectorInfo* pSectorInfo ) const
{
    return SectorEvent::IsAvailableAt( pSectorInfo );
}

void SectorEventTemplate::OnPlayerEnterSector()
{
}

void SectorEventTemplate::OnShipDestroyed( Ship* pShip )
{
}

} // namespace Hexterminate