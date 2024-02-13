// Copyright 2017 Pedro Nunes
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

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Genesis
{

/////////////////////////////////////////////////////////////////////
// Component
// A component automatically registers itself with the 
// ComponentFactory, as well as declaring smart pointers for the
// templated type.
//
// To declare a new Component (.h file):
//
//	class ShieldInhibitorComponent : public Genesis::Component 
//	{
//		DECLARE_COMPONENT( ShieldInhibitorComponent )
//	public:
//		virtual bool Initialise() override;
//		virtual void Update( float delta ) override;
//	};
//
// And in the .cpp file:
//
//	IMPLEMENT_COMPONENT( ShieldInhibitorComponent )
//
/////////////////////////////////////////////////////////////////////

class Component;
typedef unsigned int ComponentTypeID;

#define DECLARE_COMPONENT_SMART_PTR( COMPONENT ) \
	typedef std::shared_ptr<COMPONENT> COMPONENT ## SharedPtr; \
	typedef std::weak_ptr<COMPONENT> COMPONENT ## WeakPtr;

#define DECLARE_COMPONENT( COMPONENT ) \
public: \
	static std::shared_ptr<Component> Create() { return std::static_pointer_cast<Component>( std::make_shared<COMPONENT>() ); } \
	virtual std::string GetName() const override; \
private: \
	static const Genesis::ComponentAutoRegister<COMPONENT> m_AutoRegister;

#define IMPLEMENT_COMPONENT( COMPONENT ) \
	const Genesis::ComponentAutoRegister<COMPONENT> COMPONENT::m_AutoRegister( #COMPONENT ); \
	std::string COMPONENT::GetName() const { return #COMPONENT; } \
	DECLARE_COMPONENT_SMART_PTR( COMPONENT )

DECLARE_COMPONENT_SMART_PTR( Component )
class Component
{
public:
	virtual ~Component() {}
	virtual bool Initialise() { return true; }
	virtual void Update( float delta ) {}
	void SetTypeID( ComponentTypeID id ) { m_TypeID = id; }
	ComponentTypeID GetTypeID() const { return m_TypeID; }
	virtual std::string GetName() const = 0;

private:
	ComponentTypeID m_TypeID;
};

typedef std::function<ComponentSharedPtr()> ComponentCreationFn;


/////////////////////////////////////////////////////////////////////
// ComponentFactory
/////////////////////////////////////////////////////////////////////

class ComponentFactory
{
public:
	static ComponentSharedPtr Create( const std::string& componentName )
	{
		const ComponentIDPair& pair = GetRegistryMap()[componentName];
		ComponentSharedPtr component = pair.creationFn();
		component->SetTypeID( pair.id );
		return component;
	}

	static void Register( const std::string& componentName, ComponentCreationFn creationFn )
	{
		static ComponentTypeID sId = 0u;

		ComponentIDPair pair;
		pair.creationFn = creationFn;
		pair.id = ++sId;

		GetRegistryMap()[componentName] = pair;
	}

	struct ComponentIDPair
	{
		ComponentIDPair()
        {
			id = 0u;
        }

		ComponentCreationFn creationFn;
		ComponentTypeID id;
	};

	typedef std::unordered_map<std::string, ComponentIDPair> RegistryMap;

private:
	static RegistryMap& GetRegistryMap()
	{
		static RegistryMap sRegistryMap;
		return sRegistryMap;
	}
};


/////////////////////////////////////////////////////////////////////
// ComponentContainer
/////////////////////////////////////////////////////////////////////

typedef std::unordered_map< std::string, ComponentSharedPtr > ComponentMap;

class ComponentContainer
{
public:
	void Add( ComponentSharedPtr component )
	{
		m_Components.insert( std::pair< std::string, ComponentSharedPtr >( component->GetName(), component ) );
	}

	ComponentSharedPtr Get( const std::string& name ) const
	{
		ComponentMap::const_iterator it = m_Components.find( name );
		if ( it == m_Components.cend() )
		{
			return nullptr;
		}
		else
		{
			return it->second;
		}
	}

	ComponentMap::iterator begin()
	{
		return m_Components.begin();
	}

	ComponentMap::iterator end()
	{
		return m_Components.end();
	}

private:
	ComponentMap m_Components;
};


/////////////////////////////////////////////////////////////////////
// ComponentAutoRegister
/////////////////////////////////////////////////////////////////////

template<class T>
class ComponentAutoRegister
{
public:
	ComponentAutoRegister<T>( const std::string& componentName )
	{
		ComponentFactory::Register( componentName, &T::Create );
	}
};

}