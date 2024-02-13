#include "scene.h"
#include "rendersystem.fwd.h"
#include "layer.h"

namespace Genesis
{

//---------------------------------------------------------------
// Scene
//---------------------------------------------------------------

Scene::Scene()
{
    mCamera = new Camera();
    mMask = 0xFFFFFFFF;
}

Scene::~Scene()
{
    delete mCamera;
}

TaskStatus Scene::Update( float delta )
{
    for ( auto& pLayer : mLayerList )
	{
        pLayer->Update( delta );
	}

    return TaskStatus::Continue;
}

void Scene::Render()
{
    for ( auto& pLayer : mLayerList )
    {
        if ( pLayer->GetLayerDepth() & GetLayerMask() )
        {
            glClear( GL_DEPTH_BUFFER_BIT );
            pLayer->Render();
        }
    }

    for ( auto& pLayerToDelete : mLayersToDelete )
    {
        for ( LayerList::iterator it = mLayerList.begin(), itEnd = mLayerList.end(); it != itEnd; )
        {
            if ( *it == pLayerToDelete )
            {
                it = mLayerList.erase( it );
            }
            else
			{
                it++;
			}
        }
    }
    mLayersToDelete.clear();
}

LayerSharedPtr Scene::AddLayer( uint32_t depth, bool isBackground /* = false */ )
{
    for ( auto& pLayer : mLayerList )
    {
        if ( pLayer->GetLayerDepth() == depth && pLayer->IsBackground() == isBackground )
        {
            return pLayer;
        }
    }

	LayerSharedPtr pLayer = std::make_shared< Layer >( depth, isBackground );
	mLayerList.push_back( pLayer );
    return pLayer;
}

void Scene::RemoveLayer( uint32_t depth )
{
    for ( auto& pLayer : mLayerList )
    {
        if ( pLayer->GetLayerDepth() == depth )
        {
            mLayersToDelete.push_back( pLayer );
            pLayer->MarkForDeletion();
        }
    }
}

//------------------------------------------------------------------
// Camera
//------------------------------------------------------------------

Camera::Camera()
    : mPosition( 0.0f, 0.0f, 0.0f )
    , mTargetPosition( 0.0f, 0.0f, 1.0f )
{
}

Camera::~Camera()
{
}
}