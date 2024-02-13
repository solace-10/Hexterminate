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

#include <glm/mat4x4.hpp>

#include "collisionobject.h"
#include "shape.fwd.h"

class btRigidBody;

namespace Genesis
{
namespace Physics
{

class Shape;
class Simulation;


/////////////////////////////////////////////////////////////////////
// Ghost
/////////////////////////////////////////////////////////////////////

class Ghost : public CollisionObject
{
	friend Simulation;
public:
	Ghost( ShapeSharedPtr pShape, const glm::mat4x4& worldTransform );
	virtual Type GetType() const override;
	void SetWorldTransform( const glm::mat4x4& worldTransform );

private:
	std::unique_ptr< btRigidBody > m_pRigidBody;
	glm::mat4x4 m_WorldTransform;
};
GENESIS_DECLARE_SMART_PTR( Ghost );

inline CollisionObject::Type Ghost::GetType() const
{
	return CollisionObject::Type::Ghost;
}

} // namespace Physics
} // namespace Genesis