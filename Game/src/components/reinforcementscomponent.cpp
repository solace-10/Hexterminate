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

#include "components/reinforcementscomponent.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include <genesis.h>

#include "fleet/fleet.h"
#include "hexterminate.h"
#include "sector/sector.h"
#include "ship/shipinfo.h"

namespace Hexterminate
{

IMPLEMENT_COMPONENT( ReinforcementsComponent )

static const float sReinforcementsTimer = 40.0f;

bool ReinforcementsComponent::Initialise()
{
    m_Timer = sReinforcementsTimer / 2.0f; // The first wave of reinforcements appears faster.
    m_CurrentReinforcements = 0;
    m_Reinforcements.reserve( 8 ); // We can't have more than 8 bordering sectors anyway.
    m_pReinforcementsFaction = nullptr;

    LoadReinforcements();

    return true;
}

void ReinforcementsComponent::LoadReinforcements()
{
    SectorInfo* pSectorInfo = g_pGame->GetCurrentSector()->GetSectorInfo();
    m_pReinforcementsFaction = pSectorInfo->GetFaction();
    std::string factionName = m_pReinforcementsFaction->GetName();
    std::transform( factionName.begin(), factionName.end(), factionName.begin(), []( char c ) -> char { return static_cast<char>( std::tolower( c ) ); } );

    int numReinforcements = 0;
    SectorInfoVector borderingSectors;
    pSectorInfo->GetBorderingSectors( borderingSectors );
    for ( auto& pBorderingSector : borderingSectors )
    {
        if ( pBorderingSector->GetFaction() == pSectorInfo->GetFaction() )
        {
            numReinforcements++;
        }
    }
    m_Reinforcements.resize( numReinforcements );

    for ( int i = 0; i < numReinforcements; ++i )
    {
        std::stringstream filename;
        filename << "data/xml/reinforcements/" << factionName << "_" << i + 1 << ".sl";
        ShipInfoVector shipInfos = LoadShipListFile( m_pReinforcementsFaction, filename.str() );
        m_Reinforcements[ i ].reserve( shipInfos.size() );
        for ( const ShipInfo* pShipInfo : shipInfos )
        {
            m_Reinforcements[ i ].push_back( pShipInfo );
        }
    }

    if ( numReinforcements >= 4 )
    {
        g_pGame->AddIntel(
            GameCharacter::FleetIntelligence,
            "Captain, there are still too many hostile sectors surrounding this one. "
            "This will make the battle much more challenging as they'll get reinforcements very quickly." );
    }
}

ShipInfoVector ReinforcementsComponent::LoadShipListFile( Faction* pFaction, const std::string& filename ) const
{
    ShipInfoVector shipInfos;
    std::ifstream fs( filename );
    if ( fs.is_open() )
    {
        std::string shipName;
        ShipInfoManager* pShipInfoManager = g_pGame->GetShipInfoManager();
        while ( fs.good() )
        {
            fs >> shipName;

            const ShipInfo* pShipInfo = pShipInfoManager->Get( pFaction, shipName );
            if ( pShipInfo == nullptr )
            {
                Genesis::FrameWork::GetLogger()->LogWarning( "Couldn't load ship '%s' for faction '%s'", shipName.c_str(), pFaction->GetName().c_str() );
            }
            else
            {
                shipInfos.push_back( pShipInfo );
            }
        }
        fs.close();
    }
    else
    {
        Genesis::FrameWork::GetLogger()->LogWarning( "Could not open ship list file '%s'", filename.c_str() );
    }

    return shipInfos;
}

void ReinforcementsComponent::Update( float delta )
{
    if ( m_CurrentReinforcements < static_cast<int>( m_Reinforcements.size() ) )
    {
        if ( m_Timer <= 0.0f )
        {
            SpawnReinforcements( m_Reinforcements[ m_CurrentReinforcements++ ] );
            m_Timer = sReinforcementsTimer;
        }
        else
        {
            m_Timer -= delta;
        }
    }
}

void ReinforcementsComponent::SpawnReinforcements( const ShipInfoVector& shipInfos )
{
    Sector* pSector = g_pGame->GetCurrentSector();
    FleetSharedPtr pTemporaryFleet = std::make_shared<Fleet>();
    pTemporaryFleet->Initialise( m_pReinforcementsFaction, pSector->GetSectorInfo() );

    for ( const ShipInfo* pShipInfo : shipInfos )
    {
        pTemporaryFleet->AddShip( pShipInfo );
    }

    if ( pSector->Reinforce( pTemporaryFleet ) )
    {
        m_TemporaryFleets.push_back( pTemporaryFleet );
    }
}

} // namespace Hexterminate