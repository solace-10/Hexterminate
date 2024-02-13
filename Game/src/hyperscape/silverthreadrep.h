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
#include <vertexbuffer.h>

namespace Genesis
{
class Shader;
class ShaderUniform;
} // namespace Genesis

namespace Hexterminate
{

class SilverThread;

//-----------------------------------------------------------------------------
// SilverThreadRep
//-----------------------------------------------------------------------------

class SilverThreadRep : public Genesis::SceneObject
{
public:
    SilverThreadRep( SilverThread* pSilverThread );
    virtual ~SilverThreadRep();

    void Initialise();
    void RemoveFromScene();

    virtual void Update( float delta );
    virtual void Render();

    void Show( bool state );
    bool IsVisible() const;

private:
    void UpdateInput();

    void RebuildLocationVB();
    void RebuildLinkVB();
    void DrawBackground();

    void OnLeftMouseButtonDown();

    SilverThread* m_pSilverThread;
    bool m_Show;
    glm::vec2 m_Parallax;
    Genesis::Shader* m_pStarShader;
    Genesis::ShaderUniform* m_pStarParallax;
    Genesis::VertexBuffer* m_pLocationVB;
    Genesis::PositionData m_LocationPosData;
    Genesis::UVData m_LocationUVData;
    Genesis::ColourData m_LocationColorData;

    struct LocationScreenData
    {
        glm::vec2 position;
        float size;
    };
    std::vector<LocationScreenData> m_LocationScreenData;

    Genesis::Shader* m_pLinkShader;
    Genesis::ShaderUniform* m_pLinkParallax;
    Genesis::VertexBuffer* m_pLinkVB;
    Genesis::PositionData m_LinkPosData;
    Genesis::UVData m_LinkUVData;
    Genesis::ColourData m_LinkColorData;

    Genesis::LayerSharedPtr m_pLayer;

    Genesis::InputCallbackToken m_LeftMouseButtonDownToken;
};

inline bool SilverThreadRep::IsVisible() const
{
    return m_Show;
}

} // namespace Hexterminate
