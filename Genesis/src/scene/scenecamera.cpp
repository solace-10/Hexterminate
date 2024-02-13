#include "scenecamera.h"

namespace Genesis
{

SceneCamera::SceneCamera()
    : mPosition( 0.0f, 0.0f, -50.0f )
    , mTarget( 0.0f, 0.0f, 0.0f )
    , mUp( 0.0f, 1.0f, 0.0f )
{
}

void SceneCamera::CalculateUpVector()
{
    mUp = glm::vec3( 0.0f, 1.0f, 0.0f );
}

} // namespace Genesis
