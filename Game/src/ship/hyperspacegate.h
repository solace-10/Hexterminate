#pragma once

#include "ship/hyperspacecore.h"
#include <scene/sceneobject.h>

namespace Genesis
{

class ResourceModel;
class ResourceSound;

} // namespace Genesis

namespace Hexterminate
{

class Ship;

enum HyperspaceGateState
{
    Invalid,
    Opening,
    Transitioning,
    Closing,
    Closed
};

class HyperspaceGate : public Genesis::SceneObject
{
public:
    HyperspaceGate( Ship* pOwner );

    void Initialise();

    void Open( HyperspaceJumpDirection jumpDirection );
    bool HasClosed() const;

    virtual void Update( float delta ) override;
    void Render() override{};
    void Render( const glm::mat4& modelTransform );

    const glm::vec3& GetGatePosition() const;

    void CalculateBoundingBox();

private:
    void LoadSFX();
    void PlaySFX();

    Ship* m_pOwner;
    Genesis::ResourceModel* m_pModel;
    glm::vec3 m_GatePosition;
    glm::vec3 m_GatePositionLocal;
    float m_GateScale;

    HyperspaceGateState m_State;
    float m_JumpTimer;
    float m_OpeningTimer;
    HyperspaceJumpDirection m_Direction;

    glm::vec3 m_BoundingBoxTopLeft;
    glm::vec3 m_BoundingBoxBottomRight;

    Genesis::ResourceSound* m_pSFX;
};

inline const glm::vec3& HyperspaceGate::GetGatePosition() const
{
    return m_GatePosition;
}

} // namespace Hexterminate
