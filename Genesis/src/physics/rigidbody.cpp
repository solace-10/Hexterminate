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

#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <SDL.h>

#include "rigidbody.h"
#include "shape.h"

class btRigidBody;

namespace Genesis
{
namespace Physics
{

/////////////////////////////////////////////////////////////////////
// RigidBody
/////////////////////////////////////////////////////////////////////

RigidBody::RigidBody( const RigidBodyConstructionInfo& ci ) :
m_pRigidBody( nullptr ),
m_pMotionState( nullptr ),
m_MotionType( MotionType::Static ),
m_Mass( 0 ),
m_CentreOfMass( 0.0f ),
m_LinearDamping( 0.0f ),
m_AngularDamping( 0.0f )
{
	m_pShape = nullptr;
	SDL_assert( ( ci.GetMass() > 0 && ci.GetMotionType() == MotionType::Dynamic ) || 
				( ci.GetMass() == 0 && ci.GetMotionType() == MotionType::Static ) );

	SDL_assert( ci.GetShape() != nullptr );

	m_Mass = ci.GetMass();
	m_CentreOfMass = ci.GetCentreOfMass();
	m_MotionType = ci.GetMotionType();

	btTransform worldTransform;
	worldTransform.setFromOpenGLMatrix( glm::value_ptr( ci.GetWorldTransform() ) );
	m_pMotionState = std::make_unique< btDefaultMotionState >( worldTransform );

	m_pShape = ci.GetShape();
	btCollisionShape* pCollisionShape = ci.GetShape()->m_pShape;
	btVector3 localInertia( 0.0f, 0.0f, 0.0f );
	if ( m_MotionType == MotionType::Dynamic )
	{
		pCollisionShape->calculateLocalInertia( static_cast< btScalar >( m_Mass ), localInertia );
	}

	btRigidBody::btRigidBodyConstructionInfo rbInfo( 
		static_cast< btScalar >( ci.GetMass() ),
		m_pMotionState.get(),
		ci.GetShape()->m_pShape,
		localInertia 
	);

	m_LinearDamping = ci.GetLinearDamping();
	m_AngularDamping = ci.GetAngularDamping();
	rbInfo.m_linearDamping = ci.GetLinearDamping();
	rbInfo.m_angularDamping = ci.GetAngularDamping();

	m_pRigidBody = std::make_unique< btRigidBody >( rbInfo );
	m_pRigidBody->setActivationState( DISABLE_DEACTIVATION );
	m_pRigidBody->setCollisionFlags( m_pRigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );
	m_pRigidBody->setUserPointer( this );
}

glm::mat4x4 RigidBody::GetWorldTransform() const
{
	btTransform tr;
	m_pMotionState->getWorldTransform(tr);

	float mat[16];
	tr.getOpenGLMatrix(mat);

	return glm::make_mat4x4(mat);
}

glm::vec3 RigidBody::GetPosition() const
{
	btTransform tr;
	m_pMotionState->getWorldTransform(tr);
	const btVector3& position = tr.getOrigin();
	return glm::vec3( position.x(), position.y(), position.z() );
}

glm::vec3 RigidBody::GetLinearVelocity() const
{
	const btVector3& linearVelocity = m_pRigidBody->getLinearVelocity();
	return glm::vec3( linearVelocity.x(), linearVelocity.y(), linearVelocity.z() );
}

glm::vec3 RigidBody::GetAngularVelocity() const
{
	const btVector3& angularVelocity = m_pRigidBody->getAngularVelocity();
	return glm::vec3( angularVelocity.x(), angularVelocity.y(), angularVelocity.z() );
}

void RigidBody::SetLinearDamping( float value )
{
	m_LinearDamping = value;
	m_pRigidBody->setDamping( m_LinearDamping, m_AngularDamping ); 
}

void RigidBody::SetAngularDamping( float value )
{
	m_AngularDamping = value;
	m_pRigidBody->setDamping( m_LinearDamping, m_AngularDamping ); 
}

void RigidBody::SetWorldTransform( const glm::mat4x4 worldTransform )
{
	btTransform tr;
	tr.setFromOpenGLMatrix( glm::value_ptr( worldTransform ) );
	m_pRigidBody->setWorldTransform( tr );
	m_pMotionState->setWorldTransform( tr );
	m_pRigidBody->clearForces();
}

void RigidBody::SetLinearVelocity( const glm::vec3& linearVelocity )
{
	m_pRigidBody->setLinearVelocity( btVector3( linearVelocity.x, linearVelocity.y, linearVelocity.z ) );
}

void RigidBody::SetAngularVelocity( const glm::vec3& angularVelocity )
{
	m_pRigidBody->setAngularVelocity( btVector3( angularVelocity.x, angularVelocity.y, angularVelocity.z ) );
}

void RigidBody::SetMotionType( MotionType motionType )
{
	int flags = m_pRigidBody->getCollisionFlags();
	if ( motionType == MotionType::Static )
	{
		flags |= btCollisionObject::CF_STATIC_OBJECT;
	}
	else
	{
		flags &= btCollisionObject::CF_STATIC_OBJECT;
	}
	m_pRigidBody->setCollisionFlags( flags );

	m_MotionType = motionType;
}

void RigidBody::ApplyAngularForce( const glm::vec3& force )
{
	m_pRigidBody->applyTorque( btVector3( force.x, force.y, force.z ) );
}

void RigidBody::ApplyLinearForce( const glm::vec3& force )
{
	m_pRigidBody->applyCentralForce( btVector3( force.x, force.y, force.z ) );
}

void RigidBody::ApplyAngularImpulse( const glm::vec3& impulse )
{
	m_pRigidBody->applyTorqueImpulse( btVector3( impulse.x, impulse.y, impulse.z ) );
}

void RigidBody::ApplyLinearImpulse( const glm::vec3& impulse )
{
	m_pRigidBody->applyCentralImpulse( btVector3( impulse.x, impulse.y, impulse.z ) );
}

void RigidBody::SetLinearFactor( const glm::vec3& linearFactor )
{
	m_pRigidBody->setLinearFactor( btVector3( linearFactor.x, linearFactor.y, linearFactor.z ) );
}

void RigidBody::SetAngularFactor( const glm::vec3& angularFactor )
{
	m_pRigidBody->setAngularFactor( btVector3( angularFactor.x, angularFactor.y, angularFactor.z ) );
}

}
}