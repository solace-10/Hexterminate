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

#include "faction/empirefaction.h"

#include "hexterminate.h"
#include "player.h"
#include "requests/requestmanager.h"
#include "sector/fogofwar.h"
#include "sector/galaxy.h"

namespace Hexterminate
{

// How long to go between giving the player a passive allocation of influence.
// Currently the same as TurnDuration but kept separate in case it needs to be adjusted without side effects.
static const float sPassiveInfluenceAllocationTimer = 30.0f;

// The player receives sInfluenceAllocationPerSector times how many sectors the Empire controls every time the
// allocation timer expires.
static const int sInfluenceAllocationPerSector = 20;

EmpireFaction::EmpireFaction( const FactionInfo& info )
    : Faction( info, FactionId::Empire )
    , m_InfluenceTimer( sPassiveInfluenceAllocationTimer )
    , m_pRequestManager( nullptr )
{
    m_pRequestManager = new RequestManager();
}

EmpireFaction::~EmpireFaction()
{
    delete m_pRequestManager;
}

void EmpireFaction::Update( float delta )
{
    Faction::Update( delta );

    if ( g_pGame->GetPlayer()->GetPerks()->IsEnabled( Perk::UnityIsStrength ) && g_pGame->GetGalaxy() != nullptr )
    {
        m_InfluenceTimer -= delta * g_pGame->GetGalaxy()->GetCompression();
        if ( m_InfluenceTimer <= 0.0f )
        {
            m_InfluenceTimer += sPassiveInfluenceAllocationTimer;

            Player* pPlayer = g_pGame->GetPlayer();
            if ( pPlayer != nullptr )
            {
                const int numControlledSectors = static_cast<int>( GetControlledSectors().size() );
                const int influence = numControlledSectors * sInfluenceAllocationPerSector;
                pPlayer->SetInfluence( pPlayer->GetInfluence() + influence );
            }
        }
    }

    FogOfWar* pFogOfWar = g_pGame->GetGalaxy()->GetFogOfWar();
    if ( pFogOfWar != nullptr )
    {
        const SectorInfoVector& controlledSectors = GetControlledSectors();
        for ( const SectorInfo* pSectorInfo : controlledSectors )
        {
            pFogOfWar->MarkAsVisible( pSectorInfo, 1 );
        }
    }

    m_pRequestManager->Update( delta );
}

void EmpireFaction::AddControlledSector( SectorInfo* pSector, bool immediate, bool takenByPlayer )
{
    Faction* pOriginalFaction = pSector->GetFaction();

    Faction::AddControlledSector( pSector, immediate, takenByPlayer );

    // If the Empire has gained a new sector from another faction, then we need to take into account the
    // perks "Reclaimed sectors" (increased strength of the regional fleet) and Shared glory (additional
    // influence if taken by an Imperial AI fleet).
    if ( pOriginalFaction != nullptr )
    {
        Player* pPlayer = g_pGame->GetPlayer();
        const Perks* pPerks = pPlayer->GetPerks();

        if ( takenByPlayer == false && pPerks->IsEnabled( Perk::SharedGlory ) )
        {
            // Shared glory gives the player half of the normal reward for a sector when an AI fleet conquers it
            const int conquestReward = pOriginalFaction->GetConquestReward( pSector ) / 2;
            const int influence = pPlayer->GetInfluence() + conquestReward;
            pPlayer->SetInfluence( influence );
        }

        if ( pPerks->IsEnabled( Perk::ReclaimedSectors ) )
        {
            pSector->RestoreRegionalFleet();
        }
    }
}

} // namespace Hexterminate