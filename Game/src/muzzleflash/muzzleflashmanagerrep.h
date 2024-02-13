// Copyright 2019 Pedro Nunes
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

#include <rendersystem.h>
#include <scene/sceneobject.h>
#include <vertexbuffer.h>

namespace Genesis
{
class ResourceImage;
class Shader;
class ShaderParameters;
} // namespace Genesis

namespace Hexterminate
{

class TrailManager;
class Trail;

class MuzzleflashManagerRep : public Genesis::SceneObject
{
public:
    MuzzleflashManagerRep( MuzzleflashManager* pManager );
    virtual ~MuzzleflashManagerRep() override;
    virtual void Update( float delta ) override;
    virtual void Render() override;
    void SetManager( MuzzleflashManager* pManager );

private:
    void PushBackUVs( Genesis::UVData& uvData );
    void PushBackColours( Genesis::ColourData& colourData, const Genesis::Color& colour );

    MuzzleflashManager* m_pManager;
    Genesis::Shader* m_pShader;
    Genesis::VertexBuffer* m_pVertexBuffer;
    size_t m_NumVertices;
};

} // namespace Hexterminate