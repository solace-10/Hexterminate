#include "sceneobject.h"
#include "../rendersystem.h"

namespace Genesis
{

SceneObject::SceneObject()
    : m_Terminating( false )
{
}

SceneObject::~SceneObject()
{
}

void SceneObject::Update( float delta )
{
}

void SceneObject::Render()
{
}
}