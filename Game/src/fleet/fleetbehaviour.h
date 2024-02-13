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

#include <vector>

namespace Hexterminate
{

class Faction;
class Fleet;
class SectorInfo;
typedef std::vector<SectorInfo*> SectorInfoVector;

///////////////////////////////////////////////////////////////////////////////
// FleetBehaviourType
///////////////////////////////////////////////////////////////////////////////

enum class FleetBehaviourType
{
    None = -1, // Probably a player fleet, no indepedent behaviour
    Roaming = 0,
    Raiding,
    Expansionist,
    Defensive
};

///////////////////////////////////////////////////////////////////////////////
// FleetBehaviour
// Cannot be instantiated directly. Use one of the subclasses instead.
///////////////////////////////////////////////////////////////////////////////

class FleetBehaviour
{
protected:
    FleetBehaviour( Fleet* pFleet );

public:
    virtual ~FleetBehaviour(){};
    virtual bool ProcessTurn(); // Returns whether to continue processing the turn or not.
    virtual void NotifyBattleWon();

protected:
    SectorInfo* GetSectorToAssist() const;
    void NotifyAssist( SectorInfo* pSectorToAssist );
    bool CanAttackSector( SectorInfo* pSectorInfo ) const;
    int GetAssistingFleetsCount( SectorInfo* pSectorToAssist ) const;

    Fleet* m_pFleet;
    bool m_ClaimsSectors; // Does this fleet claim the sectors it fights in?
    bool m_AssistsFriendlies; // If a friendly fleet is fighting in this fleet's range, does it move in to assist?
    bool m_JoinsTheFray; // If there is an on-going battle not involving friendlies, does this fleet jump in?
};

///////////////////////////////////////////////////////////////////////////////
// FleetBehaviourRoaming
// A roaming fleet will head out from its base shipyard and wander randomly,
// up to a maximum distance from its base.
///////////////////////////////////////////////////////////////////////////////

class FleetBehaviourRoaming : public FleetBehaviour
{
public:
    FleetBehaviourRoaming( Fleet* pFleet, const SectorInfo* pBaseSector );
    virtual ~FleetBehaviourRoaming(){};
    virtual bool ProcessTurn();

protected:
    bool HasValidBaseSector() const;
    void RelocateBaseSector();
    bool TryAttackSector( SectorInfo* pSector );

    const SectorInfo* m_pBaseSector;
};

///////////////////////////////////////////////////////////////////////////////
// FleetBehaviourRaiding
///////////////////////////////////////////////////////////////////////////////

class FleetBehaviourRaiding : public FleetBehaviour
{
public:
    FleetBehaviourRaiding( Fleet* pFleet );
    virtual ~FleetBehaviourRaiding(){};
    virtual bool ProcessTurn();
};

/////////////////////////////////////s//////////////////////////////////////////
// FleetBehaviourExpansionist
// Attempts to expand along the borders of its faction.
///////////////////////////////////////////////////////////////////////////////

class FleetBehaviourExpansionist : public FleetBehaviour
{
public:
    FleetBehaviourExpansionist( Fleet* pFleet );
    virtual ~FleetBehaviourExpansionist(){};
    virtual bool ProcessTurn();

    static void sGetHostileBorderingSectors( Faction* pFaction, const SectorInfo* pAroundSector, SectorInfoVector& hostileSectors );
};

} // namespace Hexterminate
