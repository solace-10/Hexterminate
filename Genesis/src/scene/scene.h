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

#include <list>

#include <SDL.h>
#include <glm/vec3.hpp>

#include "../taskmanager.h"
#include "layer.h"


namespace Genesis
{

class Camera;

//---------------------------------------------------------------
// Scene
//---------------------------------------------------------------

class Scene : public Task
{
public:
    Scene();
    virtual ~Scene();
    TaskStatus Update( float delta );
    void Render();
    LayerSharedPtr AddLayer( uint32_t depth, bool isBackground = false );
    void RemoveLayer( uint32_t depth );
    void SetLayerMask( uint32_t mask ); // Allows only layers which match this mask to be rendered
    uint32_t GetLayerMask() const;
    Camera* GetCamera() const;

private:
	using LayerList = std::list< LayerSharedPtr >;

    LayerList mLayerList;
    LayerList mLayersToDelete;
    Camera* mCamera;
    uint32_t mMask;
};

inline Camera* Scene::GetCamera() const
{
    return mCamera;
}

inline void Scene::SetLayerMask( uint32_t mask )
{
    mMask = mask;
}

inline uint32_t Scene::GetLayerMask() const
{
    return mMask;
}

//---------------------------------------------------------------
// Camera
//---------------------------------------------------------------

class Camera
{
public:
    Camera();
    ~Camera();
    void SetPosition( const glm::vec3& vec );
    void SetPosition( float x, float y, float z );
    void SetTargetPosition( const glm::vec3& vec );
    void SetTargetPosition( float x, float y, float z );
    const glm::vec3& GetPosition() const;
    const glm::vec3& GetTargetPosition() const;

private:
    glm::vec3 mPosition;
    glm::vec3 mTargetPosition;
};

inline void Camera::SetPosition( const glm::vec3& vec )
{
    mPosition = vec;
}

inline void Camera::SetPosition( float x, float y, float z )
{
    mPosition.x = x;
    mPosition.y = y;
    mPosition.z = z;
}

inline const glm::vec3& Camera::GetPosition() const
{
    return mPosition;
}

inline void Camera::SetTargetPosition( const glm::vec3& vec )
{
    mTargetPosition = vec;
}

inline void Camera::SetTargetPosition( float x, float y, float z )
{
    mTargetPosition.x = x;
    mTargetPosition.y = y;
    mTargetPosition.z = z;
}

inline const glm::vec3& Camera::GetTargetPosition() const
{
    return mTargetPosition;
}
}
