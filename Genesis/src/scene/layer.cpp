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

#include "layer.h"
#include "../genesis.h"
#include "../rendersystem.h"
#include "sceneobject.h"

namespace Genesis
{

Layer::Layer( uint32_t depth, bool isBackground )
    : mDepth( depth )
    , mIsBackground( isBackground )
    , mMarkedForDeletion( false )
{
}

Layer::~Layer()
{
    // Remove any remaining objects from the layer
    LayerObjectList::iterator itEnd = mObjectList.end();
    for ( LayerObjectList::iterator it = mObjectList.begin(); it != itEnd; ++it )
    {
        if ( it->hasOwnership )
        {
            delete it->pSceneObject;
        }
    }
}

// Update all objects in this layer
void Layer::Update( float delta )
{
    if ( IsMarkedForDeletion() )
        return;

    {
        LayerObjectList::const_iterator itEnd = mObjectList.end();
        for ( LayerObjectList::const_iterator it = mObjectList.begin(); it != itEnd; ++it )
        {
            it->pSceneObject->Update( delta );
        }
    }

    if ( mToRemove.empty() == false )
    {
        SceneObjectList::iterator itEnd = mToRemove.end();
        for ( SceneObjectList::iterator it = mToRemove.begin(); it != itEnd; ++it )
        {
            LayerObjectList::iterator it2End = mObjectList.end();
            for ( LayerObjectList::iterator it2 = mObjectList.begin(); it2 != it2End; ++it2 )
            {
                if ( it2->pSceneObject == *it )
                {
                    if ( it2->hasOwnership )
                    {
                        delete *it;
                    }
                    mObjectList.erase( it2 );
                    break;
                }
            }
        }
        mToRemove.clear();
    }
}

// Render all objects in this layer
void Layer::Render()
{
    if ( IsMarkedForDeletion() )
        return;

    RenderSystem* pRenderSystem = FrameWork::GetRenderSystem();
    if ( IsBackground() )
    {
        pRenderSystem->ViewOrtho();
    }
    else
    {
        pRenderSystem->ViewPerspective();
    }

    LayerObjectList::const_iterator itEnd = mObjectList.end();
    for ( LayerObjectList::const_iterator it = mObjectList.begin(); it != itEnd; ++it )
    {
        if ( it->pSceneObject->IsTerminating() == false )
        {
            it->pSceneObject->Render();
        }
    }
}

void Layer::AddSceneObject( SceneObject* pObject, bool hasOwnership /* = true */ )
{
#ifdef _DEBUG
    for ( auto& pLayerObject : mObjectList )
    {
        if ( pLayerObject.pSceneObject == pObject )
        {
            FrameWork::GetLogger()->LogWarning( "Object has already been added to the layer!" );
        }
    }
#endif

    LayerObject obj;
    obj.pSceneObject = pObject;
    obj.hasOwnership = hasOwnership;
    mObjectList.push_back( obj );
}

// We can't remove the objects immediately, since they may be
// being stepped by the Update. Therefore we only remove them
// when the update is finished.
void Layer::RemoveSceneObject( SceneObject* object )
{
    object->SetTerminating();
    mToRemove.push_back( object );
}
}