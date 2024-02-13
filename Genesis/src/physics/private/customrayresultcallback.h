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

#include "BulletCollision/CollisionDispatch/btCollisionWorld.h"

namespace Genesis
{
namespace Physics
{
namespace Private
{

class Shape;

struct CustomRayResultCallback : public btCollisionWorld::RayResultCallback
{
	CustomRayResultCallback( const btVector3& rayFromWorld, const btVector3& rayToWorld ) :
		m_rayFromWorld( rayFromWorld ),
		m_rayToWorld( rayToWorld )
	{
	}

	btAlignedObjectArray< const btCollisionObject* > m_collisionObjects;

	btVector3 m_rayFromWorld; // Used to calculate hitPointWorld from hitFraction.
	btVector3 m_rayToWorld;

	btAlignedObjectArray< btVector3 > m_hitNormalWorld;
	btAlignedObjectArray< btVector3 > m_hitPointWorld;
	btAlignedObjectArray< btScalar > m_hitFractions;
	btAlignedObjectArray< int > m_hitChildShapeIndex;

	virtual	btScalar addSingleResult( btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace ) override
	{
		m_collisionObject = rayResult.m_collisionObject;
		m_collisionObjects.push_back(rayResult.m_collisionObject);
		btVector3 hitNormalWorld;
		if ( normalInWorldSpace )
		{
			hitNormalWorld = rayResult.m_hitNormalLocal;
		} 
		else
		{
			// Need to transform normal into worldspace.
			hitNormalWorld = m_collisionObject->getWorldTransform().getBasis() * rayResult.m_hitNormalLocal;
		}
		m_hitNormalWorld.push_back( hitNormalWorld );
		btVector3 hitPointWorld;
		hitPointWorld.setInterpolate3( m_rayFromWorld, m_rayToWorld, rayResult.m_hitFraction );
		m_hitPointWorld.push_back( hitPointWorld );
		m_hitFractions.push_back( rayResult.m_hitFraction );

		// In a compound shape, the index of the child shape we've hit is stored in m_triangleIndex.
		m_hitChildShapeIndex.push_back( rayResult.m_localShapeInfo == nullptr ? -1 : rayResult.m_localShapeInfo->m_triangleIndex );

		return m_closestHitFraction;
	}
};

} // namespace Private
} // namespace Physics
} // namespace Genesis
