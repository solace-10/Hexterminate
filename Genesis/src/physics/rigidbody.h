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

#include "beginexternalheaders.h"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "endexternalheaders.h"

#include "collisionobject.h"
#include "shape.fwd.h"

class btRigidBody;
class btMotionState;

namespace Genesis
{
namespace Physics
{

class Shape;
class Simulation;

enum class MotionType
{
	Static,
	Dynamic
};

/////////////////////////////////////////////////////////////////////
// RigidBodyConstructionInfo
/////////////////////////////////////////////////////////////////////

class RigidBodyConstructionInfo
{
public:
	RigidBodyConstructionInfo();

	void SetMass( int value );
	int GetMass() const;

	void SetMotionType( MotionType motionType );
	MotionType GetMotionType() const;

	void SetWorldTransform( const glm::mat4x4& worldTransform );
	const glm::mat4x4& GetWorldTransform() const;

	void SetShape( ShapeSharedPtr pShape );
	ShapeSharedPtr GetShape() const;

	void SetLinearDamping( float value );
	float GetLinearDamping() const;

	void SetAngularDamping( float value );
	float GetAngularDamping() const;

	void SetFriction( float value );
	float GetFriction() const;

	void SetCentreOfMass( const glm::vec3& centreOfMass );
	const glm::vec3& GetCentreOfMass() const;

private:
	int m_Mass;
	MotionType m_MotionType;
	glm::mat4x4 m_WorldTransform;
	ShapeSharedPtr m_pShape;
	float m_LinearDamping;
	float m_AngularDamping;
	float m_Friction;
	glm::vec3 m_CentreOfMass;
};


/////////////////////////////////////////////////////////////////////
// RigidBody
/////////////////////////////////////////////////////////////////////

class RigidBody : public CollisionObject
{
	friend Simulation;
public:
	RigidBody( const RigidBodyConstructionInfo& ci );
	virtual Type GetType() const override;

	glm::mat4x4 GetWorldTransform() const;
	glm::vec3 GetPosition() const;
	glm::vec3 GetLinearVelocity() const;
	glm::vec3 GetAngularVelocity() const;
	MotionType GetMotionType() const;
	int GetMass() const;
	const glm::vec3& GetCentreOfMass() const;
	void SetLinearDamping( float value );
	float GetLinearDamping() const;
	void SetAngularDamping( float value );
	float GetAngularDamping() const;

	void SetWorldTransform( const glm::mat4x4 worldTransform );
	void SetLinearVelocity( const glm::vec3& linearVelocity );
	void SetAngularVelocity( const glm::vec3& angularVelocity );
	void SetMotionType( MotionType motionType );

	void ApplyAngularForce( const glm::vec3& force );
	void ApplyLinearForce( const glm::vec3& force );
	void ApplyAngularImpulse( const glm::vec3& impulse );
	void ApplyLinearImpulse( const glm::vec3& impulse );

	void SetLinearFactor( const glm::vec3& linearFactor );
	const glm::vec3& GetLinearFactor() const;
	void SetAngularFactor( const glm::vec3& angularFactor );
	const glm::vec3& GetAngularFactor() const;

private:
	std::unique_ptr< btRigidBody > m_pRigidBody;
	std::unique_ptr< btMotionState > m_pMotionState;
	MotionType m_MotionType;
	int m_Mass;
	float m_LinearDamping;
	float m_AngularDamping;
	glm::vec3 m_CentreOfMass;
	glm::vec3 m_LinearFactor;
	glm::vec3 m_AngularFactor;
};
GENESIS_DECLARE_SMART_PTR( RigidBody );

} // namespace Physics
} // namespace Genesis

#include "rigidbody.inl"