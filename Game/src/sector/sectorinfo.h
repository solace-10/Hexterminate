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

#include <list>
#include <unordered_set>
#include <vector>

#include <beginexternalheaders.h>
#include <endexternalheaders.h>
#include <tinyxml2.h>

#include "misc/mathaux.h"
#include "sector/background.h"
#include "serialisable.h"

#include "fleet/fleet.fwd.h"

namespace Hexterminate
{

class BackgroundInfo;
class ShipInfo;
class Faction;
class Fleet;

typedef std::unordered_set<std::string> StringSet;

enum class ThreatRating
{
    None,
    Trivial,
    Easy,
    Fair,
    Challenging,
    Overpowering,

    Count
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SectorInfo
// Contains all the information that is needed to instantiate a Sector. Gets updated throughout the game as its
// contents change due to being contested, having a shipyard built etc.
// The information contained by a SectorInfo is modified as the game changes - the files in the the disk are only
// read when a SectorInfo gets initially created for a new game.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SectorInfo;
typedef std::vector<SectorInfo*> SectorInfoVector;

class SectorInfo : public Serialisable
{
public:
    SectorInfo();
    SectorInfo( int x, int y );
    virtual ~SectorInfo(){};

    void SetupRegionalFleet( bool isEasySector );

    const std::string& GetName() const { return m_Name; }
    void SetName( const std::string& name ) { m_Name = name; }
    Faction* GetFaction() const { return m_pFaction; }
    void SetFaction( Faction* pFaction, bool immediate, bool byPlayer );
    bool HasShipyard() const { return m_HasShipyard; }
    void SetShipyard( bool state ) { m_HasShipyard = state; }
    bool HasProbe() const { return m_HasProbe; }
    void SetProbe( bool state ) { m_HasProbe = state; }
    bool HasStarfort() const { return m_HasStarfort; }
    void SetStarfort( bool state );
    int GetStarfortHealth() const { return m_StarfortHealth; }
    void SetStarfortHealth( int value ) { m_StarfortHealth = value; }
    void GetCoordinates( int& x, int& y ) const
    {
        x = m_Coordinates.x;
        y = m_Coordinates.y;
    }
    const glm::ivec2& GetCoordinates() const { return m_Coordinates; }
    int GetRegionalFleetPoints() const;
    void RestoreRegionalFleet();
    bool IsContested() const { return m_Contested; }
    void SetAutoResolve( bool state ) { m_AutoResolve = state; }
    FleetWeakPtrList GetContestedFleets() const { return m_ContestedFleets; }
    void ForceResolve( Faction* pVictoriousFaction );
    const BackgroundInfo* GetBackground() const { return m_pBackgroundInfo; }
    bool IsPersonal() const { return m_IsPersonal; }
    void SetPersonal( bool state ) { m_IsPersonal = state; }
    int GetConquestReward() const;
    void Contest();
    void ProcessTurn();
    void FleetDisengaged( FleetWeakPtr pFleet );
    void GetBorderingSectors( SectorInfoVector& sectors, bool allowDiagonals = true ) const;
    ThreatRating GetThreatRating() const;
    void SetHyperspaceInhibitor( bool state ) { m_HasHyperspaceInhibitor = state; }
    bool HasHyperspaceInhibitor() const { return m_HasHyperspaceInhibitor; }
    bool HasStar() const { return m_HasStar; }
    void SetProceduralSpawning( bool state ) { m_HasProceduralSpawning = state; }
    bool HasProceduralSpawning() const { return m_HasProceduralSpawning; }
    const StringSet& GetComponentNames() const { return m_ComponentNames; }
    StringSet& GetComponentNames() { return m_ComponentNames; }
    void AddComponentName( const std::string& componentName );
    bool HasComponentName( const std::string& componentName ) const;
    void SetHomeworld( bool state );
    bool IsHomeworld() const;

    // Serialisable
    virtual bool Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement ) override;
    virtual bool Read( tinyxml2::XMLElement* pRootElement ) override;
    virtual int GetVersion() const override { return 3; }
    virtual void UpgradeFromVersion( int version ) override;

protected:
    void SetRegionalFleetPoints( int value );
    void UpdateContestedStatus();
    void UpdateRegionalFleet();
    void UpdateStarfort();
    ThreatRating CalculateThreatRating( int alliedScore, int hostileScore ) const;
    int CalculateThreatValue( FleetSharedPtr pFleet ) const;
    void ReadComponents( tinyxml2::XMLElement* pRootElement );

    glm::ivec2 m_Coordinates;

    std::string m_Name;
    Faction* m_pFaction;
    bool m_HasShipyard;
    bool m_HasProbe;
    bool m_HasStarfort;
    bool m_HasHyperspaceInhibitor;
    bool m_Contested;
    bool m_AutoResolve;
    bool m_IsPersonal;
    bool m_HasStar;
    bool m_HasProceduralSpawning;
    bool m_IsHomeworld;
    float m_RegionalFleetBasePoints;
    int m_RegionalFleetPoints;
    int m_StarfortHealth;
    FleetWeakPtrList m_ContestedFleets;
    const BackgroundInfo* m_pBackgroundInfo;
    StringSet m_ComponentNames;
};

} // namespace Hexterminate
