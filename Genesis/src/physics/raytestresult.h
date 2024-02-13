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

#include <glm/vec3.hpp>

#include "shape.fwd.h"

namespace Genesis
{
namespace Physics
{

class Shape;

class RayTestResult
{
public:
	RayTestResult( const glm::vec3& position, const glm::vec3& normal, ShapeWeakPtr pShape, ShapeWeakPtr pChildShape, float fraction );
	const glm::vec3& GetPosition() const;
	const glm::vec3& GetNormal() const;

	// Value between 0 and 1, representing how far along the ray the collision has happened.
	float GetFraction() const;

	// The shape of the object we've collided with.
	ShapeWeakPtr GetShape() const;

	// The specific child shape we've hit. This will be null if we haven't hit a CompoundObject.
	ShapeWeakPtr GetChildShape() const;

private:
	glm::vec3 m_Position;
	glm::vec3 m_Normal;
	ShapeWeakPtr m_pShape;
	ShapeWeakPtr m_pChildShape;
	float m_Fraction;
};

} // namespace Physics
} // namespace Genesis

#include "raytestresult.inl"