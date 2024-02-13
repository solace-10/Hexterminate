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

#include "beginexternalheaders.h"
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include "endexternalheaders.h"

#include "shape.h"

#include "ghost.h"

namespace Genesis
{
namespace Physics
{

/////////////////////////////////////////////////////////////////////
// Ghost
/////////////////////////////////////////////////////////////////////

Ghost::Ghost( ShapeSharedPtr pShape, const glm::mat4x4& worldTransform )
{
	m_pShape = pShape;

	btCollisionShape* pCollisionShape = pShape->m_pShape;

	btRigidBody::btRigidBodyConstructionInfo rbInfo( 
		0.0f,
		nullptr,
		pCollisionShape
	);

	m_pRigidBody = std::make_unique< btRigidBody >( rbInfo );
	m_pRigidBody->setActivationState( DISABLE_DEACTIVATION );
	m_pRigidBody->setCollisionFlags( m_pRigidBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE );
	m_pRigidBody->setUserPointer( this );

	SetWorldTransform( worldTransform );
}

void Ghost::SetWorldTransform( const glm::mat4x4& worldTransform )
{
	btTransform tr;
	tr.setFromOpenGLMatrix( glm::value_ptr( worldTransform ) );
	m_pRigidBody->setWorldTransform( tr );
}

} // namespace Physics
} // namespace Genesis