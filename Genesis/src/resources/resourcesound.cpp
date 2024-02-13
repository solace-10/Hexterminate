// Copyright 2014 Pedro Nunes
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

#include "sound/soundmanager.h"
#include "../genesis.h"
#include "../logger.h"

#include "resourcesound.h"

namespace Genesis
{

ResourceSound::ResourceSound( const Filename& filename )
    : ResourceGeneric( filename )
    , m_Flags( 0 )
{
    using namespace std::literals;
    m_InstancingLimit = 0ms;
    m_LastInstancedAt = std::chrono::system_clock::now();
}

ResourceSound::~ResourceSound()
{

}

bool ResourceSound::Load()
{
    m_State = ResourceState::Loaded;
    return true;
}

bool ResourceSound::Initialise( int flags /* = 0 */ )
{
    // m_Flags might already have some values in it (e.g. this resource being a playlist) which have been injected after the object was constructed
    // but before Initialise() was called.
    m_Flags |= flags;
    m_Flags |= SOUND_FLAG_INITIALISED;

    return true;
}

ResourceSound* ResourceSound::LoadAs3D( const std::string& filename )
{
    using namespace std::literals;
    ResourceSound* pResource = nullptr;
    if ( filename.empty() == false )
    {
        pResource = (ResourceSound*)FrameWork::GetResourceManager()->GetResource( filename );
        if ( pResource != nullptr )
        {
            int flags = SOUND_FLAG_3D | SOUND_FLAG_FX;
            pResource->Initialise( flags );
            pResource->SetInstancingLimit( 100ms );
        }
    }

    return pResource;
}

} // namespace Genesis
