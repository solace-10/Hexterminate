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

#include <utility>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "shape.fwd.h"

class btCollisionShape;
class btBoxShape;
class btSphereShape;

namespace Genesis
{
namespace Physics
{

class CompoundShape;
class Ghost;
class RigidBody;
class Shape;
class Simulation;

using ShapeContainer = std::vector< ShapeSharedPtr >;
static unsigned int sActiveShapes = 0u;


/////////////////////////////////////////////////////////////////////
// Shape
/////////////////////////////////////////////////////////////////////

class Shape
{
	friend Ghost;
	friend RigidBody;
	friend CompoundShape;
	friend Simulation;
public:
	Shape();
	virtual ~Shape();

	enum class Type
	{
		Box,
		Compound,
		Sphere,
		ConvexHull,
		Cylinder
	};

	virtual Type GetType() const = 0;
	void SetUserData( void* pUserData );
	void* GetUserData() const;

protected:
	btCollisionShape* m_pShape;
	void* m_pUserData;
};


/////////////////////////////////////////////////////////////////////
// BoxShape
/////////////////////////////////////////////////////////////////////

class BoxShape : public Shape
{
public:
	BoxShape( float width, float height, float depth );

	virtual Type GetType() const override { return Type::Box; }
};


/////////////////////////////////////////////////////////////////////
// CompoundShape
/////////////////////////////////////////////////////////////////////

class CompoundShape : public Shape
{
public:
	CompoundShape();

	void AddChildShape( ShapeSharedPtr pShape, const glm::mat4x4& localTransform );
	void RemoveChildShape( ShapeSharedPtr pShape );
	void RemoveChildShape( unsigned int index );
	ShapeSharedPtr GetChildShape( unsigned int index ) const;
	glm::mat4x4 GetChildTransform( unsigned int index ) const;
	std::size_t GetChildrenCount() const;

	virtual Type GetType() const override { return Type::Compound; }

private:
	using ChildShapeContainer = std::vector< std::pair< ShapeSharedPtr, glm::mat4x4 > >;
	ChildShapeContainer m_ChildShapes;
};


/////////////////////////////////////////////////////////////////////
// SphereShape
/////////////////////////////////////////////////////////////////////

class SphereShape : public Shape
{
public:
	SphereShape( float radius );

	virtual Type GetType() const override { return Type::Sphere; }
};


/////////////////////////////////////////////////////////////////////
// ConvexHullShape
/////////////////////////////////////////////////////////////////////

using ConvexHullVertices = std::vector< glm::vec3 >;

class ConvexHullShape : public Shape
{
public:
	ConvexHullShape( const ConvexHullVertices& vertices );

	virtual Type GetType() const override { return Type::ConvexHull; }
};


/////////////////////////////////////////////////////////////////////
// CylinderShape
/////////////////////////////////////////////////////////////////////

enum class CylinderShapeAxis
{
	X,
	Y,
	Z
};

class CylinderShape : public Shape
{
public:
	CylinderShape( CylinderShapeAxis axis, float width, float height, float depth );

	virtual Type GetType() const override { return Type::Cylinder; }
};

}
}