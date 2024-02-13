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

#include "coredefines.h"

namespace Genesis
{

class SceneObject;

struct LayerObject
{
    SceneObject* pSceneObject;
    bool hasOwnership;
};

typedef std::list<LayerObject> LayerObjectList;
typedef std::list<SceneObject*> SceneObjectList;

class Layer
{
public:
    Layer( uint32_t depth, bool isBackground );
    ~Layer();
    void Update( float delta );
    void Render();
    void AddSceneObject( SceneObject* object, bool hasOwnership = true );
    void RemoveSceneObject( SceneObject* object );
    uint32_t GetLayerDepth() const;
    bool IsBackground() const;
    bool IsMarkedForDeletion() const;
    void MarkForDeletion();

private:
    LayerObjectList mObjectList;
    SceneObjectList mToRemove;
    uint32_t mDepth;
    bool mIsBackground;
    bool mMarkedForDeletion;
};
GENESIS_DECLARE_SMART_PTR( Layer );

inline uint32_t Layer::GetLayerDepth() const { return mDepth; }
inline bool Layer::IsBackground() const { return mIsBackground; }
inline bool Layer::IsMarkedForDeletion() const { return mMarkedForDeletion; }
inline void Layer::MarkForDeletion() { mMarkedForDeletion = true; }
}
