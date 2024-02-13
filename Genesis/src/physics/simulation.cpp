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

#include <algorithm>

#include "beginexternalheaders.h"
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "endexternalheaders.h"

#include "physics/private/customrayresultcallback.h"
#include "physics/debugrender.h"
#include "physics/ghost.h"
#include "physics/rigidbody.h"
#include "physics/shape.h"
#include "physics/simulation.h"
#include "physics/window.h"
#include "render/debugrender.h"

namespace Genesis
{
namespace Physics
{

void InternalTickCallback( btDynamicsWorld *pDynamicsWorld, btScalar timeStep ) 
{
	CollisionDataSet& collisionDataSet = *reinterpret_cast< CollisionDataSet* >( pDynamicsWorld->getWorldUserInfo() );
	collisionDataSet.clear();

	const int numManifolds = pDynamicsWorld->getDispatcher()->getNumManifolds();
	for ( int i = 0; i < numManifolds; i++ ) 
	{
		btPersistentManifold* pContactManifold = pDynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);

		const btCollisionObject* pObjA = pContactManifold->getBody0();
		const btCollisionObject* pObjB = pContactManifold->getBody1();
		SDL_assert( pObjA->getInternalType() == btCollisionObject::CO_RIGID_BODY );
		SDL_assert( pObjB->getInternalType() == btCollisionObject::CO_RIGID_BODY );

		CollisionObject* pCollisionObjectA = reinterpret_cast< CollisionObject* >( pObjA->getUserPointer() );
		CollisionObject* pCollisionObjectB = reinterpret_cast< CollisionObject* >( pObjB->getUserPointer() );
		if ( pCollisionObjectA->GetType() != CollisionObject::Type::RigidBody ||
			 pCollisionObjectB->GetType() != CollisionObject::Type::RigidBody )
		{
			continue;
		}

		RigidBody* pRigidBodyA = static_cast< RigidBody* >( pCollisionObjectA );
		RigidBody* pRigidBodyB = static_cast< RigidBody* >( pCollisionObjectB );
		auto extractShapeFn = []( RigidBody* pRigidBody, int shapeIndex ) -> ShapeWeakPtr
		{
			if ( pRigidBody->GetShape().expired() )
			{
				return ShapeWeakPtr();
			}
			else
			{
				ShapeSharedPtr pShape = pRigidBody->GetShape().lock();
				if ( shapeIndex >= 0 )
				{
					SDL_assert( pShape->GetType() == Shape::Type::Compound );
					CompoundShapeSharedPtr pCompoundShape = std::static_pointer_cast< CompoundShape >( pShape );
					return pCompoundShape->GetChildShape( shapeIndex );
				}
				else
				{
					return pShape;
				}
			}
		};

		const int numContacts = pContactManifold->getNumContacts();
		for ( int j = 0; j < numContacts; j++ ) 
		{
			btManifoldPoint& pt = pContactManifold->getContactPoint(j);
			const btVector3& ptA = pt.getPositionWorldOnA();
			const btVector3& ptB = pt.getPositionWorldOnB();
			const glm::vec3 gptA( ptA.x(), ptA.y(), ptA.z() );
			const glm::vec3 gptB( ptB.x(), ptB.y(), ptB.z() );

			// extractShapeFn shouldn't ever return nullptr, but there is an
			// issue with the index potentially pointing to an already-removed
			// shape, with the Genesis::Physics::Shape and the underlying btShape
			// not agreeing as to how many shapes they have.
			// This only appears to happen during this internal tick callback.
			ShapeWeakPtr pShapeA = extractShapeFn( pRigidBodyA, pt.m_index0 );
			ShapeWeakPtr pShapeB = extractShapeFn( pRigidBodyB, pt.m_index1 );
			if ( pShapeA.expired() || pShapeB.expired() )
			{
				continue;
			}

			collisionDataSet.emplace(
				pRigidBodyA,
				pRigidBodyB,
				pShapeA,
				pShapeB,
				( gptA + gptB ) / 2.0f 
			);
		}
	}
}

Simulation::Simulation()
{
	m_pCollisionConfiguration = new btDefaultCollisionConfiguration();
	m_pDispatcher = new btCollisionDispatcher( m_pCollisionConfiguration );
	m_pBroadphase = new btDbvtBroadphase();
	m_pSolver = new btSequentialImpulseConstraintSolver;
	m_pWorld = new btDiscreteDynamicsWorld( m_pDispatcher, m_pBroadphase, m_pSolver, m_pCollisionConfiguration );
	m_pWorld->setInternalTickCallback( &InternalTickCallback, &m_CollisionDataSet );
	m_pWorld->setGravity( btVector3( 0, 0, 0 ) );

	m_pDebugRender = new DebugRender();
	m_pWorld->setDebugDrawer( m_pDebugRender );

	m_pDebugWindow = new Window( this );
	m_IsPaused = false;
	m_ProcessingCallbacks = false;
}

Simulation::~Simulation()
{
	delete m_pWorld;
	delete m_pSolver;
	delete m_pBroadphase;
	delete m_pDispatcher;
	delete m_pCollisionConfiguration;
	delete m_pDebugRender;
}

TaskStatus Simulation::Update( float delta )
{
	if ( m_IsPaused == false )
	{
		m_pWorld->stepSimulation( delta, 5 );
		ProcessCollisionCallbacks();
	}

	m_pWorld->debugDrawWorld();
	RenderAdditionalInformation();

	m_pDebugWindow->Update( delta );

	return TaskStatus::Continue;
}

void Simulation::Add( RigidBody* pRigidBody )
{
	m_pWorld->addRigidBody( pRigidBody->m_pRigidBody.get() );
	m_RigidBodies.push_back( pRigidBody );
}

void Simulation::Add( Ghost* pGhost )
{
	m_pWorld->addCollisionObject( pGhost->m_pRigidBody.get() );
	m_Ghosts.push_back( pGhost );
}

void Simulation::Remove( RigidBody* pRigidBody )
{
	m_pWorld->removeRigidBody( pRigidBody->m_pRigidBody.get() );
	m_RigidBodies.remove( pRigidBody );
}

void Simulation::Remove( Ghost* pGhost )
{
	m_pWorld->removeRigidBody( pGhost->m_pRigidBody.get() );
	m_Ghosts.remove( pGhost );
}

void Simulation::RayTest( const glm::vec3& from, const glm::vec3& to, RayTestResultVector& results )
{
	results.clear();

	btVector3 btFrom( from.x, from.y, from.z );
	btVector3 btTo( to.x, to.y, to.z );

	Private::CustomRayResultCallback rayCallback( btFrom, btTo );
	m_pWorld->rayTest( btFrom, btTo, rayCallback );

	if ( m_pDebugRender->IsEnabled( DebugRender::Mode::RayTests ) )
	{
		m_pDebugRender->drawLine( btFrom, btTo, btVector3( 1.0f, 1.0f, 1.0f ) );
	}

	const int numHits = rayCallback.m_hitFractions.size();
	btScalar hitFraction = 0.0f;
	btVector3 hitPosition;
	btVector3 hitNormal;
	const btCollisionObject* pBtCollisionObject = nullptr;

	for ( int i = 0; i < numHits; ++i )
	{
		hitFraction = rayCallback.m_hitFractions[ i ];
		hitPosition = btFrom + ( btTo - btFrom ) * hitFraction;
		hitNormal = rayCallback.m_hitNormalWorld[ i ];
		pBtCollisionObject = rayCallback.m_collisionObjects[ i ];

		// Retrieve the child shape we've hit, if we've collided with an object which uses
		// a compound shape.
		CollisionObject* pCollisionObject = reinterpret_cast< CollisionObject* >( pBtCollisionObject->getUserPointer() );
		ShapeSharedPtr pShape = pCollisionObject->GetShape().lock();
		if ( pShape == nullptr )
		{
			continue;
		}

		ShapeSharedPtr pChildShape;
		const int childShapeIndex = rayCallback.m_hitChildShapeIndex[ i ];
		if ( pShape->GetType() == Shape::Type::Compound )
		{
			CompoundShapeSharedPtr pCompoundShape = std::static_pointer_cast< CompoundShape >( pShape );
			SDL_assert( childShapeIndex != -1 && static_cast< size_t >( childShapeIndex ) < pCompoundShape->GetChildrenCount() );
			pChildShape = pCompoundShape->GetChildShape( childShapeIndex );
		}

		results.emplace_back(
			glm::vec3( hitPosition.x(), hitPosition.y(), hitPosition.z() ),
			glm::vec3( hitNormal.x(), hitNormal.y(), hitNormal.z() ),
			pShape,
			pChildShape,
			hitFraction
		);

		if ( m_pDebugRender->IsEnabled( DebugRender::Mode::RayTests ) )
		{
			m_pDebugRender->drawSphere( hitPosition, 5.0f, btVector3( 1.0f, 0.0f, 0.0f ) );
			m_pDebugRender->drawLine( hitPosition, hitPosition + hitNormal * 10.0f, btVector3( 0.0f, 1.0f, 0.0f ) );
		}
	}

	// Sort result by hit distance.
	std::sort(
		results.begin(),
		results.end(),
		[]( const RayTestResult& resultA, const RayTestResult& resultB) -> bool { return resultA.GetFraction() < resultB.GetFraction(); } 
	);
}

void Simulation::RenderAdditionalInformation()
{
	if ( m_pDebugRender->IsEnabled( DebugRender::Mode::Transforms ) )
	{
		for ( auto& pRigidBody : m_RigidBodies )
		{
			btTransform transform;
			transform.setFromOpenGLMatrix( glm::value_ptr( pRigidBody->GetWorldTransform() ) );
			m_pDebugRender->drawTransform( transform, 20.0f );
		}
	}

	if ( m_pDebugRender->IsEnabled( DebugRender::Mode::ContactPoints ) )
	{
		Render::DebugRender* pDebugRender = FrameWork::GetDebugRender();
		for ( auto& contactPoint : m_CollisionDataSet )
		{
			pDebugRender->DrawCircle( contactPoint.position, 5.0f, glm::vec3( 0.0f, 1.0f, 0.0f ) );
		}
	}
}

void Simulation::Pause( bool state )
{
	m_IsPaused = state;
}

CollisionCallbackHandle Simulation::RegisterCollisionCallback( const CollisionCallback& callbackFn )
{
	SDL_assert( !m_ProcessingCallbacks );

	static CollisionCallbackHandle sHandle = 0UL;
	sHandle++;
	m_CollisionCallbacks.push_back( std::pair< CollisionCallbackHandle, CollisionCallback >( sHandle, callbackFn ) );
	return sHandle;
}

void Simulation::UnregisterCollisionCallback( CollisionCallbackHandle handle )
{
	SDL_assert( !m_ProcessingCallbacks );

	for ( CollisionCallbackList::iterator it = m_CollisionCallbacks.begin(); it != m_CollisionCallbacks.end(); ++it )
	{
		if ( it->first == handle )
		{
			m_CollisionCallbacks.erase( it );
			return;
		}
	}

	FrameWork::GetLogger()->LogError( "Attempting to remove CollisionCallbackHandle '%ul', which doesn't exist.", handle );
}

void Simulation::ProcessCollisionCallbacks()
{
	if ( m_CollisionCallbacks.empty() )
	{
		return;
	}

	m_ProcessingCallbacks = true;
	
	for ( auto& callback : m_CollisionCallbacks )
	{
		for ( auto& collisionData : m_CollisionDataSet )
		{
			callback.second( collisionData.pRigidBodyA, collisionData.pRigidBodyB, collisionData.pShapeA, collisionData.pShapeB, collisionData.position );
		}
	}

	m_ProcessingCallbacks = false;
}

} // namespace Physics
} // namespace Genesis
