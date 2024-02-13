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

#pragma once

#include "faction/faction.h"
#include "gameevents.h"
#include <string>
#include <vector>

namespace Hexterminate
{

class SectorInfo;
class Ship;
class SectorEvent;

typedef std::vector<SectorEvent*> SectorEventVector;

class SectorEvent : public GameEventHandler
{
public:
    SectorEvent();
    virtual ~SectorEvent() {}

    virtual void HandleGameEvent( GameEvent* pEvent ) override;

    const std::string& GetName() const;
    bool IsRepeatable() const;
    virtual bool IsAvailableAt( const SectorInfo* pSectorInfo ) const;

    virtual void OnPlayerEnterSector() = 0;
    virtual void OnShipDestroyed( Ship* pShip ) {}

protected:
    bool IsAllowedForFaction( FactionId factionId ) const;
    void AllowForFaction( FactionId factionId );
    void DisallowForFaction( FactionId factionId );

    void SetMinimumPlayTime( unsigned int minutes ); // This event can only spawn after the player has played for this many minutes
    void SetName( const std::string& name );

private:
    std::string m_Name;
    int m_FactionMask;
    bool m_IsRepeatable;
    unsigned int m_MinimumPlayTime;
};

inline const std::string& SectorEvent::GetName() const
{
    return m_Name;
}

inline void SectorEvent::SetName( const std::string& name )
{
    m_Name = name;
}

inline void SectorEvent::SetMinimumPlayTime( unsigned int minutes )
{
    m_MinimumPlayTime = minutes;
}

inline bool SectorEvent::IsRepeatable() const
{
    return m_IsRepeatable;
}

} // namespace Hexterminate
