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

#pragma once

#include <algorithm>
#include <list>
#include <memory>
#include <string>

#include <glm/vec2.hpp>

#include "fleet/fleetbehaviour.h"
#include "globals.h"
#include "serialisable.h"
#include "ship/ship.fwd.h"

#include "fleet/fleet.fwd.h"

namespace Hexterminate
{

class Fleet;
class FleetRep;
class FleetBehaviour;
class Faction;
class SectorInfo;
class ShipInfo;

typedef std::list<const ShipInfo*> ShipInfoList;

enum class FleetState
{
    Idle,
    Engaged,
    Moving,
    Arrived
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Fleet
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Fleet : public Serialisable, public std::enable_shared_from_this<Fleet>
{
public:
    Fleet();
    virtual ~Fleet();

    void Initialise( Faction* pFaction, const SectorInfo* pInitialSector );

    void Update( float delta );
    void ProcessTurn();

    inline FleetRep* GetRepresentation() const { return m_pRep; }

    inline Faction* GetFaction() const { return m_pFaction; }
    inline const glm::vec2& GetPosition() const { return m_Position; }
    inline void SetPosition( float x, float y ) { m_Position = glm::vec2( x, y ); }
    void SetDestination( float x, float y );
    inline const glm::vec2& GetDestination() const { return m_Destination; }
    void SetDestinationSector( const SectorInfo* pSectorInfo );
    inline const SectorInfo* GetDestinationSector() const { return m_pDestinationSector; }
    inline FleetState GetState() const { return m_State; }
    inline void SetState( FleetState state ) { m_State = state; }
    inline bool HasArrived() const { return ( m_State == FleetState::Arrived ); }
    inline bool IsIdle() const { return ( m_State == FleetState::Idle ); }
    inline bool IsEngaged() const { return ( m_State == FleetState::Engaged ); }
    void SetEngaged( bool state );
    SectorInfo* GetCurrentSector() const;
    int GetPoints() const;
    inline int GetAutoResolvePoints() const { return m_AutoResolvePoints; }
    inline void SetAutoResolvePoints( int points ) { m_AutoResolvePoints = std::max( 0, points ); }
    void NotifyBattleWon();
    inline void SetTerminating() { m_Terminating = true; }
    inline bool IsTerminating() const { return m_Terminating; }
    inline void SetImmunity( bool state ) { m_ImmunityTimer = state ? 10.0f : 0.0f; }
    inline bool IsImmune() const { return m_ImmunityTimer > 0.0f; }
    inline bool IsRegionalFleet() const { return m_RegionalFleet; }
    inline bool HasFlagship() const { return m_HasFlagship; }
    inline void SetFaction( Faction* pFaction ) { m_pFaction = pFaction; }

    bool IsInRangeOf( FleetWeakPtr pOtherFleetWeakPtr ) const;

    void GenerateProceduralFleet( int points );
    void GenerateFlagshipFleet();
    void AddShip( const std::string& shipName );
    void AddShip( const ShipInfo* pShipInfo );
    bool RemoveShip( const ShipInfo* pShipInfo );
    inline const ShipInfoList& GetShips() const { return m_Ships; }

    // Serialisable
    virtual bool Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement ) override;
    virtual bool Read( tinyxml2::XMLElement* pRootElement ) override;
    virtual int GetVersion() const override { return 3; }
    virtual void UpgradeFromVersion( int version ) override;

private:
    void SetBehaviourFromFaction();
    int FindCheapestShipCost() const;
    void UpdateFogOfWar();

    Faction* m_pFaction;
    const SectorInfo* m_pInitialSector;
    FleetState m_State;
    glm::vec2 m_Position;
    glm::vec2 m_Destination;
    const SectorInfo* m_pDestinationSector;

    FleetRep* m_pRep;
    FleetBehaviour* m_pBehaviour;

    int m_Points;
    int m_AutoResolvePoints;

    bool m_Terminating;
    bool m_RegionalFleet;
    bool m_HasFlagship;

    float m_ImmunityTimer;

    ShipInfoList m_Ships;
};

} // namespace Hexterminate