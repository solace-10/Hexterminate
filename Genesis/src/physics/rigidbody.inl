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

#include <SDL.h>

namespace Genesis
{
namespace Physics
{

/////////////////////////////////////////////////////////////////////
// RigidBodyConstructionInfo
/////////////////////////////////////////////////////////////////////

inline RigidBodyConstructionInfo::RigidBodyConstructionInfo() :
m_Mass( 0 ),
m_MotionType( MotionType::Dynamic ),
m_WorldTransform( 1.0f ),
m_pShape( nullptr ),
m_LinearDamping( 0.0f ),
m_AngularDamping( 0.0f ),
m_Friction( 0.0f ),
m_CentreOfMass( 0.0f )
{

}

inline void RigidBodyConstructionInfo::SetMass( int value )
{
	SDL_assert( value >= 0 );
	m_Mass = value;
}

inline int RigidBodyConstructionInfo::GetMass() const
{
	return m_Mass;
}

inline void RigidBodyConstructionInfo::SetMotionType( MotionType motionType )
{
	m_MotionType = motionType;
}

inline MotionType RigidBodyConstructionInfo::GetMotionType() const
{
	return m_MotionType;
}

inline void RigidBodyConstructionInfo::SetWorldTransform( const glm::mat4x4& worldTransform )
{
	m_WorldTransform = worldTransform;
}

inline const glm::mat4x4& RigidBodyConstructionInfo::GetWorldTransform() const
{
	return m_WorldTransform;
}

inline void RigidBodyConstructionInfo::SetShape( ShapeSharedPtr pShape )
{
	m_pShape = pShape;
}

inline ShapeSharedPtr RigidBodyConstructionInfo::GetShape() const
{
	return m_pShape;
}

inline void RigidBodyConstructionInfo::SetLinearDamping( float value )
{
	SDL_assert( value >= 0.0f );
	m_LinearDamping = value;
}

inline float RigidBodyConstructionInfo::GetLinearDamping() const
{
	return m_LinearDamping;
}

inline void RigidBodyConstructionInfo::SetAngularDamping( float value )
{
	SDL_assert( value >= 0.0f );
	m_AngularDamping = value;
}

inline float RigidBodyConstructionInfo::GetAngularDamping() const
{
	return m_AngularDamping;
}

inline void RigidBodyConstructionInfo::SetFriction( float value )
{
	SDL_assert( value >= 0.0f );
	m_Friction = value;
}

inline float RigidBodyConstructionInfo::GetFriction() const
{
	return m_Friction;
}

inline void RigidBodyConstructionInfo::SetCentreOfMass( const glm::vec3& centreOfMass )
{
	m_CentreOfMass = centreOfMass;
}

inline const glm::vec3& RigidBodyConstructionInfo::GetCentreOfMass() const
{
	return m_CentreOfMass;
}


/////////////////////////////////////////////////////////////////////
// RigidBody
/////////////////////////////////////////////////////////////////////

inline CollisionObject::Type RigidBody::GetType() const
{
	return CollisionObject::Type::RigidBody;
}

inline MotionType RigidBody::GetMotionType() const
{
	return m_MotionType;
}

inline int RigidBody::GetMass() const
{
	return m_Mass;
}

inline const glm::vec3& RigidBody::GetCentreOfMass() const 
{
	return m_CentreOfMass;
}

inline float RigidBody::GetAngularDamping() const
{
	return m_AngularDamping;
}

inline float RigidBody::GetLinearDamping() const
{
	return m_LinearDamping;
}

inline const glm::vec3& RigidBody::GetLinearFactor() const
{
	return m_LinearFactor;
}

inline const glm::vec3& RigidBody::GetAngularFactor() const
{
	return m_AngularFactor;
}

} // namespace Physics
} // namespace Genesis