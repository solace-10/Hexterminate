// Copyright 2022 Pedro Nunes
//
// This file is part of Genesis.
//
// Genesis is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Genesis is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Genesis. If not, see <http://www.gnu.org/licenses/>.

#include "resources/resourceplaylist.h"

#include <chrono>
#include <sstream>

#include "genesis.h"
#include "logger.h"
#include "resources/resourcesound.h"
#include "sound/soundmanager.h"

namespace Genesis
{

ResourcePlaylist::ResourcePlaylist( const Filename& filename )
    : ResourceGeneric( filename )
    , m_CurrentTrackIdx( 0u )
{
    const unsigned int seed = static_cast<unsigned int>( std::chrono::system_clock::now().time_since_epoch().count() );
    m_Engine = std::default_random_engine( seed );
}

ResourcePlaylist::~ResourcePlaylist()
{
}

bool ResourcePlaylist::Load()
{
    std::ifstream playlist( GetFilename().GetFullPath() );
    if ( playlist.is_open() )
    {
        // M3U files are text files containing one relative path per line.
        std::string line;
        while ( getline( playlist, line ) )
        {
            std::stringstream trackPath;
            trackPath << GetFilename().GetDirectory() << line;

            ResourceSound* pTrack = FrameWork::GetResourceManager()->GetResource<ResourceSound*>( trackPath.str() );
            if ( pTrack )
            {
                m_LoadedTracks.push_back( pTrack );
            }
            else
            {
                FrameWork::GetLogger()->LogWarning( "Unable to load track '%s' for playlist '%s'.", line.c_str(), GetFilename().GetFullPath().c_str() );
            }
        }

        if ( m_LoadedTracks.empty() == false )
        {
            std::copy( m_LoadedTracks.begin(), m_LoadedTracks.end(), std::back_inserter( m_ShuffledTracks ) );
            std::shuffle( m_ShuffledTracks.begin(), m_ShuffledTracks.end(), m_Engine );
        }
    }
    else
    {
        FrameWork::GetLogger()->LogError( "Unable to open file: %s", GetFilename().GetFullPath().c_str() );
        return false;
    }

    m_State = ResourceState::Loaded;
    return true;
}

ResourceSound* ResourcePlaylist::GetNextTrack( bool shuffle /* = false */ )
{
    if ( m_LoadedTracks.empty() )
    {
        return nullptr;
    }

    if ( shuffle )
    {
        return GetNextRandomTrack();
    }
    else
    {
        return GetNextLinearTrack();
    }
}

ResourceSound* ResourcePlaylist::GetNextLinearTrack()
{
    ResourceSound* pTrack = m_LoadedTracks[ m_CurrentTrackIdx ];
    if ( ++m_CurrentTrackIdx >= m_LoadedTracks.size() )
    {
        m_CurrentTrackIdx = 0u;
    }
    return pTrack;
}

ResourceSound* ResourcePlaylist::GetNextRandomTrack()
{
    ResourceSound* pTrack = m_ShuffledTracks[ m_CurrentTrackIdx ];
    if ( ++m_CurrentTrackIdx >= m_ShuffledTracks.size() )
    {
        m_CurrentTrackIdx = 0u;
        std::shuffle( m_ShuffledTracks.begin(), m_ShuffledTracks.end(), m_Engine );
    }
    return pTrack;
}

} // namespace Genesis
