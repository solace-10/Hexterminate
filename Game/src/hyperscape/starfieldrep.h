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

class Starfield;

//-----------------------------------------------------------------------------
// StarfieldRep
//-----------------------------------------------------------------------------

class StarfieldRep : public Genesis::SceneObject
{
public:
    StarfieldRep( Starfield* pStarfield );
    virtual ~StarfieldRep();

    void Initialise();
    void RemoveFromScene();

    virtual void Update( float delta );
    virtual void Render();

private:
    void BuildStarfield();

    Starfield* m_pStarfield;

    Genesis::Shader* m_pStarShader;
    Genesis::ShaderUniform* m_pStarfieldParallax;
    Genesis::VertexBuffer* m_pStarfieldVB;
    Genesis::PositionData m_PositionData;
    Genesis::UVData m_UVData;
    Genesis::ColourData m_ColorData;

    Genesis::LayerSharedPtr m_pLayer;

    glm::vec2 m_Parallax;
};

} // namespace Hexterminate
