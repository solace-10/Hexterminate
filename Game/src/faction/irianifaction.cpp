// Copyright 2017 Pedro Nunes
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

#include "faction/irianifaction.h"

#include "blackboard.h"
#include "hexterminate.h"
#include "requests/campaigntags.h"

namespace Hexterminate
{

IrianiFaction::IrianiFaction( const FactionInfo& info )
    : Faction( info, FactionId::Iriani )
{
}

void IrianiFaction::AddControlledSector( SectorInfo* pSector, bool immediate, bool takenByPlayer )
{
    Faction::AddControlledSector( pSector, immediate, takenByPlayer );

    if ( g_pGame->GetGameMode() == GameMode::Campaign && g_pGame->GetBlackboard()->Exists( sPlayerHasOrionsSword ) == false )
    {
        pSector->SetHyperspaceInhibitor( true );
    }
}

} // namespace Hexterminate