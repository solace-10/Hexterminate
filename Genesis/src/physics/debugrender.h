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
#include <LinearMath/btIDebugDraw.h>
#include "endexternalheaders.h"

namespace Genesis
{
namespace Physics
{

/////////////////////////////////////////////////////////////////////
// DebugRender
/////////////////////////////////////////////////////////////////////

class DebugRender : public btIDebugDraw
{
public:
	DebugRender();

	enum class Mode
	{
		None				= 0,
		Wireframe			= 1 << 0,
		AABB				= 1 << 1,
		Transforms			= 1 << 2,
		RayTests			= 1 << 3,
		ContactPoints		= 1 << 4
	};

	void SetEnabled( Mode mode, bool state );
	bool IsEnabled( Mode mode ) const;

	void draw3dText( const btVector3& location, const char* textString ) override;
	void drawLine( const btVector3& from, const btVector3& to, const btVector3& color ) override;
	void drawSphere( const btVector3& p, btScalar radius, const btVector3& color ) override;
	void drawContactPoint( const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color ) override;
	void reportErrorWarning( const char* warningString ) override;
	void setDebugMode( int debugMode ) override;
	int	getDebugMode() const override;

private:
	int ToBulletFlag( Mode mode ) const;

	int m_BulletDebugMode;
	int m_DebugMode;
};

} // namespace Physics
} // namespace Genesis
