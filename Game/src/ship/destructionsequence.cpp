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

#include <algorithm>

#include <math/misc.h>
#include <physics/rigidbody.h>

#include "gameevents.h"
#include "hexterminate.h"
#include "misc/randomshuffle.h"
#include "player.h"
#include "ship/destructionsequence.h"
#include "ship/hexgrid.h"
#include "ship/module.h"
#include "ship/ship.h"

namespace Hexterminate
{

static const float sDestructionSequenceMinDelay = 0.10f;
static const float sDestructionSequenceMaxDelay = 0.25f;

DestructionSequence::DestructionSequence( Ship* pOwner )
    : m_pOwner( pOwner )
    , m_Timer( 0.0f )
    , m_Finished( false )
{
    int x1, y1, x2, y2;
    const ModuleHexGrid& hexGrid = m_pOwner->GetModuleHexGrid();
    hexGrid.GetBoundingBox( x1, y1, x2, y2 );
    for ( int y = y1; y <= y2; ++y )
    {
        for ( int x = x1; x <= x2; ++x )
        {
            Module* pModule = hexGrid.Get( x, y );

            // Ignore the tower, it still has health but is about to be destroyed anyhow
            if ( pModule == nullptr || pOwner->GetTowerModule() == pModule || pModule->IsDestroyed() )
                continue;

            m_Modules.push_back( pModule );
        }
    }

    if ( m_Modules.empty() )
    {
        m_Finished = true;
    }
    else
    {
        m_Timer = gRand( sDestructionSequenceMinDelay, sDestructionSequenceMaxDelay );
        RandomShuffle::Shuffle( m_Modules.begin(), m_Modules.end() );
    }
}

void DestructionSequence::Update( float delta )
{
    if ( m_Finished )
        return;

    m_Timer -= delta;

    if ( m_Timer <= 0.0f && m_Modules.empty() == false )
    {
        Module* pModule = m_Modules.back();
        pModule->Destroy();
        m_Modules.pop_back();

        if ( m_Modules.empty() == false )
        {
            m_Timer = gRand( sDestructionSequenceMinDelay, sDestructionSequenceMaxDelay );
        }
    }

    if ( m_Modules.empty() )
    {
        GrantPerkPointPart();
        m_pOwner->SpawnLoot();
        m_Finished = true;
    }
}

void DestructionSequence::GrantPerkPointPart()
{
    if ( Faction::sIsEnemyOf( m_pOwner->GetFaction(), g_pGame->GetPlayerFaction() ) )
    {
        Player* pPlayer = g_pGame->GetPlayer();
        int perkPointParts = pPlayer->GetPerkPointsParts() + 1;
        if ( perkPointParts >= 10 )
        {
            pPlayer->SetPerkPointsParts( 0 );
            pPlayer->SetPerkPoints( pPlayer->GetPerkPoints() + 1 );

            GameEventManager::Broadcast( new GameEvent( GameEventType::PerkPointAcquired ) );
        }
        else
        {
            pPlayer->SetPerkPointsParts( perkPointParts );

            GameEventManager::Broadcast( new GameEvent( GameEventType::PerkPointPartAcquired ) );
        }
    }
}

} // namespace Hexterminate