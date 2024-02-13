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
#include <glm/gtc/type_ptr.hpp>
#include "endexternalheaders.h"

#include <SDL.h>

#include "shape.h"

namespace Genesis
{
namespace Physics
{

/////////////////////////////////////////////////////////////////////
// Shape
/////////////////////////////////////////////////////////////////////

Shape::Shape() :
m_pShape( nullptr ),
m_pUserData( nullptr )
{
	sActiveShapes++;
}

Shape::~Shape() 
{
	SDL_assert( m_pShape != nullptr );
	delete m_pShape;
	sActiveShapes--;
}

void Shape::SetUserData( void* pUserData )
{
	m_pUserData = pUserData;
}

void* Shape::GetUserData() const
{
	return m_pUserData;
}


/////////////////////////////////////////////////////////////////////
// BoxShape
/////////////////////////////////////////////////////////////////////

BoxShape::BoxShape( float width, float height, float depth )
{
	m_pShape = new btBoxShape( btVector3( width / 2.0f, height / 2.0f, depth / 2.0f ) ); 
	m_pShape->setUserPointer( this );
}


/////////////////////////////////////////////////////////////////////
// CompoundShape
/////////////////////////////////////////////////////////////////////

CompoundShape::CompoundShape()
{
	m_pShape = new btCompoundShape( true, 4 );
	m_pShape->setUserPointer( this );
	m_ChildShapes.reserve( 4 );
}

void CompoundShape::AddChildShape( ShapeSharedPtr pShape, const glm::mat4x4& localTransform )
{
	btCompoundShape* pCompoundShape = static_cast< btCompoundShape* >( m_pShape );
	btTransform tr;
	tr.setFromOpenGLMatrix( glm::value_ptr( localTransform ) );
	pCompoundShape->addChildShape( tr, pShape->m_pShape ); 
	m_ChildShapes.push_back( std::pair< ShapeSharedPtr, glm::mat4x4 >( pShape, localTransform ) );
}

std::size_t CompoundShape::GetChildrenCount() const
{
	return m_ChildShapes.size();
}

// Removes the first instance of pShape from the CompoundShape's list of children.
void CompoundShape::RemoveChildShape( ShapeSharedPtr pShape )
{
	const size_t numChildShapes = m_ChildShapes.size();
	for ( unsigned int idx = 0; idx < numChildShapes; ++idx )
	{
		if ( m_ChildShapes[ idx ].first == pShape )
		{
			RemoveChildShape( idx );
			break;
		}
	}
}

void CompoundShape::RemoveChildShape( unsigned int index )
{
	btCompoundShape* pCompoundShape = static_cast< btCompoundShape* >( m_pShape );
	pCompoundShape->removeChildShapeByIndex( index );

	// This needs to be a swap and pop to match Bullet's underlying implementation,
	// otherwise we end up with mismatched indices.
	std::swap( m_ChildShapes[ index ], m_ChildShapes[ m_ChildShapes.size() - 1 ] );
	m_ChildShapes.pop_back();
}

ShapeSharedPtr CompoundShape::GetChildShape( unsigned int index ) const
{
	if ( index >= m_ChildShapes.size() )
	{
		return nullptr;
	}

	ShapeSharedPtr pChildShape = m_ChildShapes[ index ].first;
	SDL_assert( pChildShape->m_pShape == static_cast< btCompoundShape* >( m_pShape )->getChildShape( index ) );

	return pChildShape;
}

glm::mat4x4 CompoundShape::GetChildTransform( unsigned int index ) const
{
	SDL_assert( index < m_ChildShapes.size() );
	return m_ChildShapes[ index ].second;
}


/////////////////////////////////////////////////////////////////////
// SphereShape
/////////////////////////////////////////////////////////////////////

SphereShape::SphereShape( float radius )
{
	m_pShape = new btSphereShape( radius );
	m_pShape->setUserPointer( this );
}


/////////////////////////////////////////////////////////////////////
// ConvexHullShape
/////////////////////////////////////////////////////////////////////

ConvexHullShape::ConvexHullShape( const ConvexHullVertices& vertices )
{
	m_pShape = new btConvexHullShape( reinterpret_cast< const btScalar* >( vertices.data() ), static_cast<int>(vertices.size()), sizeof( glm::vec3 ) );
	m_pShape->setUserPointer( this );
}


/////////////////////////////////////////////////////////////////////
// CylinderShape
/////////////////////////////////////////////////////////////////////

CylinderShape::CylinderShape( CylinderShapeAxis axis, float width, float height, float depth )
{
	btVector3 halfExtents( width / 2.0f, height / 2.0f, depth / 2.0f );
	if ( axis == CylinderShapeAxis::X )
	{
		m_pShape = new btCylinderShapeX( halfExtents );
	}
	else if ( axis == CylinderShapeAxis::Y )
	{
		m_pShape = new btCylinderShape( halfExtents );
	}
	else if ( axis == CylinderShapeAxis::Z )
	{
		m_pShape = new btCylinderShapeZ( halfExtents );
	}
	m_pShape->setUserPointer( this );
}

}
}