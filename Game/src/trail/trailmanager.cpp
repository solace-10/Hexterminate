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

#include <memory.h>

#include "hexterminate.h"
#include "trail/trail.h"
#include "trail/trailmanager.h"

namespace Hexterminate
{

TrailManager::TrailManager()
{
}

TrailManager::~TrailManager()
{
    for ( auto& pTrail : m_Trails )
    {
        delete pTrail;
    }
}

void TrailManager::Update( float delta )
{
    if ( g_pGame->IsPaused() == false )
    {
        for ( auto& pTrail : m_Trails )
        {
            pTrail->Update( delta );
        }
    }

    ProcessOrphanedTrails();
}

void TrailManager::Add( Trail* pTrail )
{
    for ( auto& pCurrentTrail : m_Trails )
    {
        if ( pCurrentTrail == pTrail )
        {
            return;
        }
    }

    m_Trails.push_back( pTrail );
}

void TrailManager::Remove( Trail* pTrail )
{
    m_Trails.remove( pTrail );
    if ( pTrail->IsOrphan() )
    {
        delete pTrail;
    }
}

void TrailManager::ProcessOrphanedTrails()
{
    for ( TrailList::iterator it = m_Trails.begin(); it != m_Trails.end(); )
    {
        if ( ( *it )->IsOrphan() && ( *it )->GetActivePoints() == 0 )
        {
            delete *it;
            it = m_Trails.erase( it );
        }
        else
        {
            it++;
        }
    }
}

} // namespace Hexterminate