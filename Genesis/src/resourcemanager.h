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

#pragma once

#include <atomic>
#include <functional>
#include <list>
#include <unordered_map>
#include <string>

#include "filename.h"
#include "resources/resourcetypes.h"
#include "SDL.h"

namespace Genesis
{

//////////////////////////////////////////////////////////////////////////////
// ResourceGeneric
//////////////////////////////////////////////////////////////////////////////

class ResourceGeneric
{
public:
    ResourceGeneric( const Filename& filename );
    virtual ~ResourceGeneric() {}
    virtual void Preload(){};
    virtual bool Load() = 0;
    virtual ResourceType GetType() const;

    ResourceState GetState() const;
    void SetState( ResourceState state );
    const Filename& GetFilename() const;

protected:
    std::atomic<ResourceState> m_State;

private:
    Filename m_Filename;
    ResourceType m_Type;
};

inline ResourceState ResourceGeneric::GetState() const { return m_State; }
inline void ResourceGeneric::SetState( ResourceState state ) { m_State = state; }
inline const Filename& ResourceGeneric::GetFilename() const { return m_Filename; }
inline ResourceType ResourceGeneric::GetType() const { return m_Type; }

typedef std::function<ResourceGeneric*( const Filename& )> ResourceFactoryFunction;

//////////////////////////////////////////////////////////////////////////////
// ExtensionData
//////////////////////////////////////////////////////////////////////////////

class ExtensionData
{
public:
    ExtensionData( const std::string& name, ResourceFactoryFunction& func )
        : m_Name( name )
        , m_ResourceFactoryFunction( func )
    {
    }

    const std::string& GetName() const;
    ResourceFactoryFunction GetFactoryFunction() const;

private:
    std::string m_Name; // "obj", "tga"...
    ResourceFactoryFunction m_ResourceFactoryFunction;
};

inline const std::string& ExtensionData::GetName() const { return m_Name; }
inline ResourceFactoryFunction ExtensionData::GetFactoryFunction() const { return m_ResourceFactoryFunction; }

using ExtensionMap = std::unordered_map<std::string, ExtensionData*>;
using ResourceMap = std::unordered_map<std::string, ResourceGeneric*>;

//////////////////////////////////////////////////////////////////////////////
// ResourceManager
//////////////////////////////////////////////////////////////////////////////

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();
    void RegisterExtension( const std::string& extension, ResourceFactoryFunction& func );

    bool CanLoadResource( const Filename& filename );

    // Retrieves a resource. This is a blocking operation.
    ResourceGeneric* GetResource( const Filename& filename );
    template <typename T>
    T GetResource( const Filename& filename )
    {
        return static_cast<T>( GetResource( filename ) );
    }

private:
    ExtensionMap mRegisteredExtensions;
    ResourceMap mResources;
};

}
