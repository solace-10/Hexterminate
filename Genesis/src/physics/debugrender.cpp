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

#include <render/debugrender.h>

#include "debugrender.h"

namespace Genesis
{
namespace Physics
{

DebugRender::DebugRender() :
m_BulletDebugMode( 0 ),
m_DebugMode( (int)Mode::None )
{

}

void DebugRender::SetEnabled( Mode mode, bool state )
{
	int f = ToBulletFlag( mode );
	if ( state ) 
	{
		m_DebugMode |= static_cast< int >( mode );
		m_BulletDebugMode |= f;
	}
	else
	{
		m_DebugMode &= ~static_cast< int >( mode );
		m_BulletDebugMode &= ~f;
	}
}

int DebugRender::ToBulletFlag( Mode mode ) const
{
	switch ( mode )
	{
	case Mode::Wireframe: return DBG_DrawWireframe;
	case Mode::AABB: return DBG_DrawAabb;
	default: return 0;
	};
}

void DebugRender::draw3dText( const btVector3& location, const char* textString )
{

}

void DebugRender::drawLine( const btVector3& from, const btVector3& to, const btVector3& color )
{
	FrameWork::GetDebugRender()->DrawLine(
		glm::vec3( from.x(), from.y(), from.z() ),
		glm::vec3( to.x(), to.y(), to.z() ),
		glm::vec3( color.x(), color.y(), color.z() ) 
	);
}

void DebugRender::drawSphere( const btVector3& p, btScalar radius, const btVector3& color ) 
{
	FrameWork::GetDebugRender()->DrawCircle(
		glm::vec3( p.x(), p.y(), p.z() ),
		radius,
		glm::vec3( color.x(), color.y(), color.z() )
	);
}

void DebugRender::drawContactPoint( const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color )
{

}

void DebugRender::reportErrorWarning( const char* warningString )
{
	FrameWork::GetLogger()->LogWarning( warningString );
}

void DebugRender::setDebugMode( int debugMode )
{
	m_BulletDebugMode = debugMode;
}

int DebugRender::getDebugMode() const
{
	return m_BulletDebugMode;
}

bool DebugRender::IsEnabled( Mode mode ) const
{
	return ( m_DebugMode & static_cast< int >( mode ) ) > 0;
}

} // namespace Physics
} // namespace Genesis