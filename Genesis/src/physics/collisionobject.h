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

#include "shape.fwd.h"

namespace Genesis
{
namespace Physics
{

class Shape;
class Simulation;


/////////////////////////////////////////////////////////////////////
// CollisionObject
/////////////////////////////////////////////////////////////////////

class CollisionObject
{
	friend Simulation;
public:
	CollisionObject() {};
	virtual ~CollisionObject() {};
	ShapeWeakPtr GetShape() const;

	enum class Type
	{
		RigidBody,
		Ghost
	};
	virtual Type GetType() const = 0;

protected:
	ShapeSharedPtr m_pShape;
};

inline ShapeWeakPtr CollisionObject::GetShape() const
{
	return m_pShape;
}

} // namespace Physics
} // namespace Genesis