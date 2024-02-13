// Copyright 2015 Pedro Nunes
//
// This file is part of Hexterminate.
//
// Hexterminate is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Hexterminate is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hexterminate. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "ship/addon/addon.h"
#include "ship/ship.h"

#include <rendersystem.h>
#include <scene/sceneobject.h>

namespace Genesis
{
class ResourceImage;
class VertexBuffer;

namespace Physics
{
    class Ghost;
    class Shape;
} // namespace Physics
} // namespace Genesis

namespace Hexterminate
{

class LaserBeam;

class AddonPhaseBarrier : public Addon
{
public:
    AddonPhaseBarrier( AddonModule* pModule, Ship* pOwner );
    virtual ~AddonPhaseBarrier();

    virtual void Initialise() override;
    virtual void Activate() override;
    virtual void Deactivate() override;

    virtual void Update( float delta ) override;
    virtual void Render( const glm::mat4& modelTransform ) override;

    float GetCharge() const;

private:
    void UpdateCharge( float delta );
    void CreatePhysicsGhost();
    void DestroyPhysicsGhost();
    Genesis::Physics::ShapeSharedPtr CreatePhysicsShape();
    void CreateGeometry();
    void UpdateColour();
    void CreateLasers( const glm::vec3& barrierOrigin, const glm::vec3& barrierPosition );
    void CalculateSphereCenter();
    void CalculateSphereRadius();

    float m_Charge;

    Genesis::ResourceImage* m_pTexture;
    Genesis::Shader* m_pShader;
    Genesis::VertexBuffer* m_pVertexBuffer;
    Genesis::ShaderUniform* m_pShieldStrengthUniform;
    Genesis::ShaderUniform* m_pClipActiveUniform;
    Genesis::ShaderUniform* m_pClipUniform;
    Genesis::ShaderUniform* m_pClipForwardUniform;
    Genesis::ShaderUniform* m_pAmbientColourUniform;
    Genesis::ShaderUniform* m_pActiveUniform;

    Genesis::Physics::Ghost* m_pGhost;

    glm::mat4x4 m_RenderTransform;
    ShipCollisionInfoUniquePtr m_pCollisionInfo;
    float m_Coverage;
    float m_SphereCenter;
    float m_SphereRadius;
};

} // namespace Hexterminate
