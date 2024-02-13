// Copyright 2021 Pedro Nunes
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

#include <genesis.h>
#include <logger.h>

#include "hyperscape/silverthread.h"
#include "hyperscape/silverthreadrep.h"
#include "misc/random.h"
#include "sector/starinfo.h"

namespace Hexterminate
{

SilverThread::SilverThread()
    : m_CurrentLocationIndex( 0 )
    , m_Scanned( 0 )
{
    m_Locations.reserve( 16 );
    m_pRep = std::make_unique<SilverThreadRep>( this );
    m_pRep->Initialise();
}

SilverThread::~SilverThread()
{
    m_pRep->RemoveFromScene();
}

void SilverThread::Add( const HyperscapeLocation& location )
{
    if ( location.GetType() == HyperscapeLocation::Type::Visited )
    {
        if ( !m_Locations.empty() && m_Locations.back().GetType() == HyperscapeLocation::Type::Scanned )
        {
            Genesis::FrameWork::GetLogger()->LogWarning( "Trying to add a Visited hyperscape location to a thread that still has scanned locations. Use MoveToScannedLocation() instead." );
            return;
        }

        m_Locations.push_back( location );
    }
    else if ( location.GetType() == HyperscapeLocation::Type::Scanned )
    {
        m_Scanned++;
        m_Locations.push_back( location );
    }
    else if ( location.GetType() == HyperscapeLocation::Type::Station )
    {
        if ( !m_Locations.empty() && m_Locations.back().GetType() != HyperscapeLocation::Type::Visited )
        {
            Genesis::FrameWork::GetLogger()->LogWarning( "Trying to add a Station hyperscape location to an invalid thread. " );
            return;
        }

        m_Locations.push_back( location );
    }
}

void SilverThread::Scan( size_t count )
{
    size_t depth = m_Locations.size() - m_Scanned;
    for ( size_t i = 0; i < count; ++i )
    {
        m_Locations.emplace_back( HyperscapeLocation::Type::Scanned, Random::Next(), static_cast<uint32_t>( depth ) );
    }
    m_Scanned += count;
}

void SilverThread::JumpToScannedLocation( size_t index )
{
    SDL_assert( index < m_Scanned );
    const HyperscapeLocation& scannedLocation = m_Locations[ m_Locations.size() - m_Scanned + index ];
    const uint32_t seed = scannedLocation.GetSeed();
    const uint32_t depth = scannedLocation.GetDepth();
    SDL_assert( scannedLocation.GetType() == HyperscapeLocation::Type::Scanned );
    for ( ; m_Scanned > 0; --m_Scanned )
    {
        m_Locations.pop_back();
    }
    Add( HyperscapeLocation( HyperscapeLocation::Type::Visited, seed, depth ) );
    m_CurrentLocationIndex = m_Locations.size() - 1;
}

void SilverThread::JumpToScannedLocation( const HyperscapeLocation& location )
{
    const size_t locationsCount = m_Locations.size();
    for ( size_t i = 0; i < m_Scanned; ++i )
    {
        if ( m_Locations[ locationsCount - i - 1 ] == location )
        {
            JumpToScannedLocation( i );
            return;
        }
    }

    SDL_assert( false ); // Tried to jump into a location that hasn't been scanned?
}

void SilverThread::CallStation()
{
    SDL_assert( m_Locations.empty() == false );
    const size_t locationsCount = m_Locations.size() - m_Scanned;
    for ( size_t i = 0; i < locationsCount; ++i )
    {
        if ( m_Locations[ i ].GetType() == HyperscapeLocation::Type::Station )
        {
            RebuildLocation( i, HyperscapeLocation::Type::Visited );
        }
    }

    RebuildLocation( m_CurrentLocationIndex, HyperscapeLocation::Type::Station );
}

void SilverThread::RebuildLocation( size_t index, HyperscapeLocation::Type newType )
{
    SDL_assert( index < m_Locations.size() );
    const HyperscapeLocation& location = m_Locations[ index ];
    m_Locations[ index ] = std::move( HyperscapeLocation( newType, location.GetSeed(), location.GetDepth() ) );
}

} // namespace Hexterminate
