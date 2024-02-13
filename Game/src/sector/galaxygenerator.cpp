// Copyright 2020 Pedro Nunes
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
#include <random>

#include "faction/faction.h"
#include "globals.h"
#include "hexterminate.h"
#include "sector/galaxy.h"
#include "sector/galaxygenerator.h"

namespace Hexterminate
{

GalaxyGenerator::GalaxyGenerator()
{
}

void GalaxyGenerator::Run( Galaxy* pGalaxy, const GalaxyCreationInfo& creationInfo )
{
    for ( int i = 0; i < static_cast<int>( FactionId::Count ); ++i )
    {
        FactionId factionId = static_cast<FactionId>( i );
        FactionPresence presence = creationInfo.GetFactionPresence( factionId );
        if ( presence != FactionPresence::None )
        {
            Data data;
            data.pHomeworldSector = nullptr;
            data.pFaction = g_pGame->GetFaction( factionId );
            data.necessarySectors = CalculateNecessarySectors( presence );
            m_Data.push_back( data );
        }
    }

    GenerateHomeworlds( pGalaxy, creationInfo );
    GenerateSectors( pGalaxy, creationInfo );
    GenerateNames( pGalaxy, creationInfo );
}

bool GalaxyGenerator::IsFinished() const
{
    for ( const Data& data : m_Data )
    {
        if ( data.necessarySectors > 0 )
        {
            return false;
        }
    }

    return true;
}

int GalaxyGenerator::CalculateNecessarySectors( FactionPresence presence ) const
{
    if ( presence == FactionPresence::None )
        return 0;
    else if ( presence == FactionPresence::Light )
        return 3 + rand() % 2;
    else if ( presence == FactionPresence::Standard )
        return 6 + rand() % 3;
    else if ( presence == FactionPresence::Heavy )
        return 10 + rand() % 4;
    else
        return 0;
}

void GalaxyGenerator::GenerateHomeworlds( Galaxy* pGalaxy, const GalaxyCreationInfo& creationInfo )
{
    for ( Data& data : m_Data )
    {
        if ( creationInfo.HasHomeworld( data.pFaction->GetFactionId() ) )
        {
            while ( true )
            {
                int x = rand() % NumSectorsX;
                int y = rand() % NumSectorsY;
                SectorInfo* pSectorInfo = pGalaxy->GetSectorInfo( x, y );
                if ( pSectorInfo->GetFaction()->GetFactionId() == FactionId::Neutral )
                {
                    data.pHomeworldSector = pSectorInfo;
                    data.pHomeworldSector->SetShipyard( true );
                    data.pHomeworldSector->SetHomeworld( true );
                    data.pHomeworldSector->SetFaction( data.pFaction, true, false );
                    data.pFaction->SetHomeworld( data.pHomeworldSector );

                    // All homeworld sectors (except the player's) are Personal, so the AI won't capture them.
                    // Also give them the HomeworldComponent, so they will spawn special encounters.
                    if ( data.pFaction->GetFactionId() != FactionId::Empire )
                    {
                        data.pHomeworldSector->SetPersonal( true );
                        data.pHomeworldSector->AddComponentName( "HomeworldComponent" );
                    }
                    // For the Empire, give the homeworld sector a starfort.
                    else
                    {
                        data.pHomeworldSector->SetStarfort( true );
                    }

                    break;
                }
            }
        }
    }
}

void GalaxyGenerator::GenerateSectors( Galaxy* pGalaxy, const GalaxyCreationInfo& creationInfo )
{
    while ( IsFinished() == false )
    {
        for ( Data& data : m_Data )
        {
            if ( data.necessarySectors == 0 )
            {
                continue;
            }

            GalaxyCreationInfo::GenerationType generationType = creationInfo.GetGenerationType( data.pFaction->GetFactionId() );
            if ( generationType == GalaxyCreationInfo::GenerationType::Expansion )
            {
                const SectorInfoVector& controlledSectors = data.pFaction->GetControlledSectors();
                SectorInfoVector borderingSectors;
                int validBorderingSectors = 0;
                for ( SectorInfo* pControlledSector : controlledSectors )
                {
                    borderingSectors.clear();
                    pControlledSector->GetBorderingSectors( borderingSectors, false );

                    SectorInfo* pSectorToAdd = nullptr;

                    for ( SectorInfo* pBorderingSector : borderingSectors )
                    {
                        if ( pBorderingSector->GetFaction()->GetFactionId() == FactionId::Neutral && std::find( controlledSectors.cbegin(), controlledSectors.cend(), pBorderingSector ) == controlledSectors.cend() )
                        {
                            validBorderingSectors++;
                            if ( rand() % 4 == 0 )
                            {
                                pSectorToAdd = pBorderingSector;
                                break;
                            }
                        }
                    }

                    if ( pSectorToAdd != nullptr )
                    {
                        data.necessarySectors--;
                        pSectorToAdd->SetFaction( data.pFaction, true, false );
                        break;
                    }

                    // Failure edge case in case this faction actually can't expand further.
                    if ( validBorderingSectors == 0 )
                    {
                        data.necessarySectors = 0;
                    }
                }
            }
            else if ( generationType == GalaxyCreationInfo::GenerationType::Scattered )
            {
                int x = rand() % NumSectorsX;
                int y = rand() % NumSectorsY;
                SectorInfo* pSectorInfo = pGalaxy->GetSectorInfo( x, y );
                if ( pSectorInfo->GetFaction()->GetFactionId() == FactionId::Neutral )
                {
                    data.necessarySectors--;
                    pSectorInfo->SetFaction( data.pFaction, true, false );
                }
            }
            else
            {
                Genesis::FrameWork::GetLogger()->LogError( "Unsupported generation type." );
            }
        }
    }
}

void GalaxyGenerator::GenerateNames( Galaxy* pGalaxy, const GalaxyCreationInfo& creationInfo )
{
    if ( creationInfo.GetMode() != GalaxyCreationInfo::CreationMode::InfiniteWar )
    {
        return;
    }

    std::vector<std::string> names = LoadNames();
    if ( names.size() < NumSectorsX * NumSectorsY )
    {
        Genesis::FrameWork::GetLogger()->LogError( "Insufficient sector names for galaxy generation." );
    }

    std::random_device dev;
    std::mt19937 rnd( dev() );
    std::shuffle( std::begin( names ), std::end( names ), rnd );
    int nameIndex = 0;
    for ( int x = 0; x < NumSectorsX; ++x )
    {
        for ( int y = 0; y < NumSectorsY; ++y )
        {
            pGalaxy->GetSectorInfo( x, y )->SetName( names[ nameIndex++ ] );
        }
    }
}

std::vector<std::string> GalaxyGenerator::LoadNames() const
{
    std::vector<std::string> names;
    std::ifstream fs( "data/xml/sectors/names.txt" );
    if ( fs.is_open() )
    {
        while ( fs.good() )
        {
            std::string name;
            fs >> name;
            names.push_back( name );
        }
        fs.close();
    }

    return names;
}

} // namespace Hexterminate
