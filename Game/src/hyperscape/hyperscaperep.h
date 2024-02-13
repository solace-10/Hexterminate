// Copyright 2021 Pedro Nunes
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

#include <inputmanager.h>
#include <scene/layer.h>
#include <scene/sceneobject.h>
#include <shader.h>
#include <vertexbuffer.h>

namespace Hexterminate
{

class Hyperscape;

//-----------------------------------------------------------------------------
// HyperscapeRep
//-----------------------------------------------------------------------------

class HyperscapeRep : public Genesis::SceneObject
{
public:
    HyperscapeRep( Hyperscape* pHyperscape );
    virtual ~HyperscapeRep();

    void Initialise();
    void RemoveFromScene();

    virtual void Update( float delta );
    virtual void Render();

    void Show( bool state );
    bool IsVisible() const;

private:
    void UpdateInput();
    void UpdateDrawInfo();

    void BuildBackground();
    void DrawBackground();

    void OnLeftMouseButtonDown();

    Hyperscape* m_pHyperscape;
    bool m_Show;

    Genesis::Shader* m_pBackgroundShader;
    Genesis::VertexBuffer* m_pBackgroundVB;
    Genesis::Shader* m_pGridShader;
    Genesis::ShaderUniform* m_pGridDiffuseSampler;
    Genesis::ShaderUniform* m_pGridParallax;
    Genesis::VertexBuffer* m_pGridVB;

    Genesis::LayerSharedPtr m_pLayer;

    Genesis::InputCallbackToken m_LeftMouseButtonDownToken;

    glm::vec2 m_Parallax;
    bool m_WasBleachBypassEnabled;
    bool m_WasVignetteEnabled;
};

inline bool HyperscapeRep::IsVisible() const
{
    return m_Show;
}

} // namespace Hexterminate
