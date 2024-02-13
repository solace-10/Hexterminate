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

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>

#include "genesis.h"
#include "memory.h"
#include "resourcemanager.h"
#include "resources/resourcefont.h"
#include "resources/resourceimage.h"
#include "resources/resourcemodel.h"
#include "resources/resourceplaylist.h"
#include "resources/resourcesound.h"
#include "resources/resourcevideo.h"

namespace Genesis
{

//////////////////////////////////////////////////////////////////////////////
// ResourceGeneric
//////////////////////////////////////////////////////////////////////////////

ResourceGeneric::ResourceGeneric( const Filename& filename )
    : m_State( ResourceState::Unloaded )
    , m_Filename( filename )
    , m_Type( ResourceType::Unknown )
{
}

//////////////////////////////////////////////////////////////////////////////
// ResourceManager
// The ResourceManager allows for asynchronous pre-loading of assets.
// This lets us perform most disk I/O outside of the main thread during
// Preload(), while Load() itself does run on the main thread and can then
// be used to deal with systems that do not play well in a multi-threaded
// environment, such as OpenGL.
//////////////////////////////////////////////////////////////////////////////

ResourceManager::ResourceManager()
{
    ResourceFactoryFunction fCreateResourceImage = []( const Filename& filename ) { return new ResourceImage( filename ); };
    RegisterExtension( "bmp", fCreateResourceImage );
    RegisterExtension( "jpg", fCreateResourceImage );
    RegisterExtension( "png", fCreateResourceImage );
    RegisterExtension( "tga", fCreateResourceImage );

    ResourceFactoryFunction fCreateResourceModel = []( const Filename& filename ) { return new ResourceModel( filename ); };
    RegisterExtension( "tmf", fCreateResourceModel );

    ResourceFactoryFunction fCreateResourcePlaylist = []( const Filename& filename ) { return new ResourcePlaylist( filename ); };
    RegisterExtension( "m3u", fCreateResourcePlaylist );

    ResourceFactoryFunction fCreateResourceSound = []( const Filename& filename ) { return new ResourceSound( filename ); };
    RegisterExtension( "mp3", fCreateResourceSound );
    RegisterExtension( "wav", fCreateResourceSound );

    ResourceFactoryFunction fCreateResourceFont = []( const Filename& filename ) { return new ResourceFont( filename ); };
    RegisterExtension( "fnt", fCreateResourceFont );

    ResourceFactoryFunction fCreateResourceVideo = []( const Filename& filename ) { return new ResourceVideo( filename ); };
    RegisterExtension( "ivf", fCreateResourceVideo );
}

ResourceManager::~ResourceManager()
{
    // Clear up the registered extensions
    ExtensionMap::iterator it;
    for ( it = mRegisteredExtensions.begin(); it != mRegisteredExtensions.end(); it++ )
    {
        delete it->second;
    }

    // Clear up the resources we loaded
    ResourceMap::iterator it2;
    for ( it2 = mResources.begin(); it2 != mResources.end(); it2++ )
    {
        delete it2->second;
    }
}

void ResourceManager::RegisterExtension( const std::string& extension, ResourceFactoryFunction& func )
{
    // Check if the extension isn't already registered
    ExtensionMap::iterator it = mRegisteredExtensions.find( extension );
    if ( it != mRegisteredExtensions.end() )
        return;

    ExtensionData* data = new ExtensionData( extension, func );
    mRegisteredExtensions[ extension ] = data;
}

bool ResourceManager::CanLoadResource( const Filename& filename )
{
    const std::string& extension = filename.GetExtension();

    // If the extension isn't registered, we can't load this resource
    ExtensionMap::iterator extensionIter = mRegisteredExtensions.find( extension );
    return ( extensionIter != mRegisteredExtensions.end() );
}

// GetResource() returns a ready-to-use resource. If the resource hadn't been previously loaded,
// it will block the main thread until the resource has finished loading.
ResourceGeneric* ResourceManager::GetResource( const Filename& filename )
{
    // Check if we already have this resource loaded
    ResourceMap::iterator resourceMapIter = mResources.find( filename.GetFullPath() );
    if ( resourceMapIter != mResources.end() )
    {
        return resourceMapIter->second;
    }

    const std::string& extension = filename.GetExtension();
    ExtensionMap::iterator extensionIter = mRegisteredExtensions.find( extension );
    if ( extensionIter == mRegisteredExtensions.end() )
    {
        FrameWork::GetLogger()->LogWarning( "Trying to load unsupported resource: %s.", filename.GetFullPath().c_str() );
        return nullptr;
    }

    ExtensionData* extensionData = extensionIter->second;
    ResourceGeneric* pResource = extensionData->GetFactoryFunction()( filename );
    SDL_assert( pResource != nullptr );

    mResources[ filename.GetFullPath() ] = pResource;

    pResource->Preload();
    pResource->Load();

    SDL_assert( pResource->GetState() == ResourceState::Loaded );
    return pResource;
}

}