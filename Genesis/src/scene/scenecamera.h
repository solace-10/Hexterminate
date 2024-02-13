#pragma once

#include <glm/vec3.hpp>

namespace Genesis
{

class SceneCamera
{
public:
    SceneCamera();
    virtual ~SceneCamera(){};

    const glm::vec3& GetPosition() const;
    void SetPosition( const glm::vec3& position );
    void SetPosition( float x, float y, float z );

    const glm::vec3& GetTarget() const;
    void SetTarget( const glm::vec3& position );
    void SetTarget( float x, float y, float z );

    void CalculateUpVector();
    const glm::vec3& GetUpVector() const;

private:
    glm::vec3 mPosition;
    glm::vec3 mTarget;
    glm::vec3 mUp;
};

inline const glm::vec3& SceneCamera::GetPosition() const { return mPosition; }
inline void SceneCamera::SetPosition( const glm::vec3& position ) { mPosition = position; }
inline void SceneCamera::SetPosition( float x, float y, float z ) { SetPosition( glm::vec3( x, y, z ) ); }

inline const glm::vec3& SceneCamera::GetTarget() const { return mTarget; }
inline void SceneCamera::SetTarget( const glm::vec3& target ) { mTarget = target; }
inline void SceneCamera::SetTarget( float x, float y, float z ) { SetTarget( glm::vec3( x, y, z ) ); }

inline const glm::vec3& SceneCamera::GetUpVector() const { return mUp; }

} // namespace Genesis
