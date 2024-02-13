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

#include "galaxycreationinfo.h"
#include "globals.h"
#include "serialisable.h"
#include <vector>

namespace Hexterminate
{

class FogOfWar;
class Sector;
class SectorInfo;
class GalaxyRep;

class Galaxy : public Serialisable
{
public:
    Galaxy();
    ~Galaxy();

    void Reset();
    void Update( float delta );
    void Show( bool state );

    inline SectorInfo* GetSectorInfo( int x, int y ) const;
    inline bool IsInitialised() const { return m_Initialised; }
    inline GalaxyRep* GetRepresentation() const { return m_pRep; }
    bool IsVisible() const;

    void Create( const GalaxyCreationInfo& creationInfo );
    void ForceNextTurn();

    inline float GetCompression() const { return m_Compression; }
    FogOfWar* GetFogOfWar() const;

    // Serialisable
    virtual bool Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement ) override;
    virtual bool Read( tinyxml2::XMLElement* pRootElement ) override;
    virtual int GetVersion() const override { return 5; }
    virtual void UpgradeFromVersion( int version ) override;

private:
    void GenerateProceduralGalaxy( const GalaxyCreationInfo& galaxyCreationInfo );
    void CalculateCompression();
    void UpdateDebugUI();
    void EndGameCheck();

    bool m_Initialised;
    SectorInfo* m_Sectors[ NumSectorsX ][ NumSectorsY ];
    GalaxyRep* m_pRep;
    float m_Compression;
    bool m_DebugWindowOpen;
    FogOfWar* m_pFogOfWar;
};

inline SectorInfo* Galaxy::GetSectorInfo( int x, int y ) const
{
    if ( x < 0 || x >= NumSectorsX || y < 0 || y >= NumSectorsY )
    {
        return nullptr;
    }
    else
    {
        return m_Sectors[ x ][ y ];
    }
}

inline FogOfWar* Galaxy::GetFogOfWar() const
{
    return m_pFogOfWar;
}

} // namespace Hexterminate
