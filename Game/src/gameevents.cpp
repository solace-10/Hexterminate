// Copyright 2016 Pedro Nunes
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

#include "gameevents.h"

namespace Hexterminate
{

GameEventHandlerList GameEventManager::m_Handlers;

/////////////////////////////////////////////////////////////////////
// GameEventManager
/////////////////////////////////////////////////////////////////////

void GameEventManager::RegisterHandler( GameEventHandler* pListener )
{
    m_Handlers.push_back( pListener );
}

void GameEventManager::RemoveHandler( GameEventHandler* pHandler )
{
    m_Handlers.remove( pHandler );
}

void GameEventManager::Broadcast( GameEvent* pEvent )
{
    for ( auto& pHandler : m_Handlers )
    {
        pHandler->HandleGameEvent( pEvent );
    }

    delete pEvent;
}

/////////////////////////////////////////////////////////////////////
// GameEventHandler
/////////////////////////////////////////////////////////////////////

GameEventHandler::GameEventHandler()
{
    GameEventManager::RegisterHandler( this );
}

GameEventHandler::~GameEventHandler()
{
    GameEventManager::RemoveHandler( this );
}

} // namespace Hexterminate
