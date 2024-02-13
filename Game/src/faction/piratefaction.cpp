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

#include "faction/piratefaction.h"
#include "blackboard.h"
#include "globals.h"
#include "hexterminate.h"
#include "requests/campaigntags.h"
#include "sector/galaxy.h"
#include "sector/sector.h"

namespace Hexterminate
{

PirateFaction::PirateFaction( const FactionInfo& info )
    : Faction( info, FactionId::Pirate )
{
}

void PirateFaction::ProcessTurn()
{
    Faction::ProcessTurn();

    if ( IsCollapsing() == false )
    {
        // Attempt to claim a random sector if we are under the NumPirateSectors threshold
        size_t numSectors = GetControlledSectors().size();
        if ( numSectors < NumPirateSectors )
        {
            SectorInfo* pSector = g_pGame->GetGalaxy()->GetSectorInfo( rand() % NumSectorsX, rand() % NumSectorsY );
            if ( pSector->GetFaction() == g_pGame->GetFaction( FactionId::Neutral ) )
            {
                pSector->SetFaction( this, false, false );
            }
        }
    }
}

} // namespace Hexterminate