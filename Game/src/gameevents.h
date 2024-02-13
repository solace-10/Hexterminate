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

#pragma once

#include <list>

namespace Hexterminate
{

class ModuleInfo;
class Ship;

/////////////////////////////////////////////////////////////////////
// GameEvent
/////////////////////////////////////////////////////////////////////

enum class GameEventType
{
    PerkAcquired,
    PerkPointAcquired,
    PerkPointPartAcquired,
    ModuleAcquired,
    ShipDestroyed
};

class GameEvent
{
public:
    GameEvent( GameEventType gameEventType )
        : m_GameEventType( gameEventType ){};

    inline GameEventType GetType() const { return m_GameEventType; }

private:
    GameEventType m_GameEventType;
};

// Message for when a new module is acquired by the player.
// If triggersNotification is true, then the loot window will be displayed.
class GameEventModuleAcquired : public GameEvent
{
public:
    GameEventModuleAcquired( ModuleInfo* pModuleInfo, unsigned int quantity, bool triggersNotification )
        : GameEvent( GameEventType::ModuleAcquired )
        , m_pModuleInfo( pModuleInfo )
        , m_Quantity( quantity )
        , m_TriggersNotification( triggersNotification )
    {
    }

    inline ModuleInfo* GetModuleInfo() const { return m_pModuleInfo; }
    inline unsigned int GetQuantity() const { return m_Quantity; }
    inline bool GetTriggersNotification() const { return m_TriggersNotification; }

private:
    ModuleInfo* m_pModuleInfo;
    unsigned int m_Quantity;
    bool m_TriggersNotification;
};

// Message for when a ship has been destroyed
class GameEventShipDestroyed : public GameEvent
{
public:
    GameEventShipDestroyed( Ship* pShip )
        : GameEvent( GameEventType::ShipDestroyed )
        , m_pShip( pShip )
    {
    }

    inline Ship* GetShip() const { return m_pShip; }

private:
    Ship* m_pShip;
};

/////////////////////////////////////////////////////////////////////
// GameEventHandler
/////////////////////////////////////////////////////////////////////

class GameEventHandler
{
public:
    GameEventHandler();
    virtual ~GameEventHandler();

    virtual void HandleGameEvent( GameEvent* pEvent ) = 0;
};

typedef std::list<GameEventHandler*> GameEventHandlerList;

/////////////////////////////////////////////////////////////////////
// GameEventManager
/////////////////////////////////////////////////////////////////////

class GameEventManager
{
public:
    static void RegisterHandler( GameEventHandler* pListener );
    static void RemoveHandler( GameEventHandler* pListener );
    static void Broadcast( GameEvent* pEvent );

    static GameEventHandlerList m_Handlers;
};

}; // namespace Hexterminate