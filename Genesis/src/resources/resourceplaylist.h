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

#pragma once

#include <random>
#include <string>
#include <vector>

#include "../resourcemanager.h"

namespace Genesis
{

class ResourceSound;

class ResourcePlaylist : public ResourceGeneric
{
public:
    ResourcePlaylist( const Filename& filename );
    virtual ~ResourcePlaylist();
    virtual ResourceType GetType() const override;
    virtual bool Load() override;

    ResourceSound* GetNextTrack( bool shuffle = false );
    std::vector<ResourceSound*> GetTracks() const;

private:
    ResourceSound* GetNextLinearTrack();
    ResourceSound* GetNextRandomTrack();

    using ResourceSoundVector = std::vector<ResourceSound*>;
    ResourceSoundVector m_LoadedTracks;
    ResourceSoundVector m_ShuffledTracks;
    unsigned int m_CurrentTrackIdx;
    std::default_random_engine m_Engine;
};

inline ResourceType ResourcePlaylist::GetType() const
{
    return ResourceType::Playlist;
}

inline std::vector<ResourceSound*> ResourcePlaylist::GetTracks() const
{
    return m_LoadedTracks;
}

} // namespace Genesis