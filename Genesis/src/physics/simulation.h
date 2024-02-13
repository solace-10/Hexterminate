// Copyright 2018 Pedro Nunes
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
#include <list>
#include <vector>
#include <unordered_set>

#include "physics/raytestresult.h"
#include "taskmanager.h"

class btCollisionDispatcher;
class btDefaultCollisionConfiguration;
struct btDbvtBroadphase;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btCollisionShape;

namespace Genesis::Physics
{

class DebugRender;
class Ghost;
class RigidBody;
class Shape;
class Window;
using RigidBodyList = std::list< RigidBody* >;
using GhostList = std::list< Ghost* >;
using RayTestResultVector = std::vector< RayTestResult >;
using CollisionCallback = std::function< void( RigidBody*, RigidBody*, ShapeWeakPtr, ShapeWeakPtr, const glm::vec3& ) >;
using CollisionCallbackHandle = unsigned long;
using CollisionCallbackList = std::list< std::pair< CollisionCallbackHandle, CollisionCallback > >;

static const CollisionCallbackHandle InvalidCollisionCallbackHandle = ~0UL;

struct CollisionData
{
	CollisionData( RigidBody* _pRigidBodyA, RigidBody* _pRigidBodyB, ShapeWeakPtr _pShapeA, ShapeWeakPtr _pShapeB, const glm::vec3& _position ) :
		pRigidBodyA( _pRigidBodyA ),
		pRigidBodyB( _pRigidBodyB ),
		pShapeA( _pShapeA ),
		pShapeB( _pShapeB ),
		position( _position )
	{ }

	RigidBody* pRigidBodyA;
	RigidBody* pRigidBodyB;
	ShapeWeakPtr pShapeA;
	ShapeWeakPtr pShapeB;
	glm::vec3 position;
};

struct CollisionDataHashFn
{
	size_t operator()( const CollisionData& cd ) const
	{
		size_t h1 = std::hash< float >()( cd.position.x );
		size_t h2 = std::hash< float >()( cd.position.y );
		return h1 ^ ( h2 << 1 );
	}
};

struct CollisionDataEqualsFn
{
	bool operator()( const CollisionData& lhs, const CollisionData& rhs ) const
	{
		return (lhs.position.x == rhs.position.x) && (lhs.position.y == rhs.position.y);
	}
};

using CollisionDataSet = std::unordered_set< CollisionData, CollisionDataHashFn, CollisionDataEqualsFn >;

class Simulation : public Task
{
	friend Window;
public:
	Simulation();
	virtual ~Simulation();

	Genesis::TaskStatus Update( float delta );

	void Add( RigidBody* pRigidBody );
	void Add( Ghost* pGhost );
	void Remove( RigidBody* pRigidBody );
	void Remove( Ghost* pGhost );

	// Performs a ray test between two points, returning all collisions in-between.
	// The collisions are ordered by distance from the starting point.
	void RayTest( const glm::vec3& from, const glm::vec3& to, RayTestResultVector& results );

	// Pauses the simulation from stepping.
	// Objects can still be added and removed from the world and raytests can be performed,
	// but without the simulation being stepped no new callbacks will be issued.
	void Pause( bool state );

	// Register or unregister a collision callback. 
	// These are sent to all listeners after the world is stepped.
	// Don't register / unregister callbacks from a callback.
	CollisionCallbackHandle RegisterCollisionCallback( const CollisionCallback& callbackFn );
	void UnregisterCollisionCallback( CollisionCallbackHandle handle );

private:
	void RenderAdditionalInformation();
	void ProcessCollisionCallbacks();

	btDefaultCollisionConfiguration* m_pCollisionConfiguration;
	btCollisionDispatcher* m_pDispatcher;
	btDbvtBroadphase* m_pBroadphase;
	btSequentialImpulseConstraintSolver* m_pSolver;
	btDiscreteDynamicsWorld* m_pWorld;
	RigidBodyList m_RigidBodies;
	GhostList m_Ghosts;
	DebugRender* m_pDebugRender;
	Window* m_pDebugWindow;
	bool m_IsPaused;
	bool m_ProcessingCallbacks;
	CollisionCallbackList m_CollisionCallbacks;
	CollisionDataSet m_CollisionDataSet;
};

} // namespace Genesis::Physics
