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

namespace Genesis
{
namespace Physics
{

inline RayTestResult::RayTestResult( const glm::vec3& position, const glm::vec3& normal, ShapeWeakPtr pShape, ShapeWeakPtr pChildShape, float fraction ) :
	m_Position( position ),
	m_Normal( normal ),
	m_pShape ( pShape ),
	m_pChildShape( pChildShape ),
	m_Fraction( fraction )
{

}

inline const glm::vec3& RayTestResult::GetPosition() const
{
	return m_Position;
}

inline const glm::vec3& RayTestResult::GetNormal() const
{
	return m_Normal;
}

inline ShapeWeakPtr RayTestResult::GetShape() const
{
	return m_pShape;
}

inline ShapeWeakPtr RayTestResult::GetChildShape() const
{
	return m_pChildShape;
}

inline float RayTestResult::GetFraction() const
{
	return m_Fraction;
}

} // namespace Physics
} // namespace Genesis