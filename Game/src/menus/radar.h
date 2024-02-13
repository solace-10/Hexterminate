// Copyright 2014 Pedro Nunes
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

#include <gui/atlas.h>
#include <gui/gui.h>
#include <rendersystem.h>
#include <vector>
#include <vertexbuffer.h>

namespace Genesis
{
class ResourceImage;
class Shader;
} // namespace Genesis

namespace Hexterminate
{

/////////////////////////////////////////////////////////////////////
// RadarIcon
/////////////////////////////////////////////////////////////////////

class RadarIcon
{
public:
    RadarIcon( int atlasIndex, float x, float y, float scale );
    int GetAtlasIndex() const;
    float GetX() const;
    float GetY() const;
    float GetScale() const;

private:
    int m_AtlasIndex;
    float m_X;
    float m_Y;
    float m_Scale;
};

typedef std::vector<RadarIcon> RadarIconVector;

/////////////////////////////////////////////////////////////////////
// Radar
/////////////////////////////////////////////////////////////////////

class Radar : public Genesis::Gui::GuiElement
{
public:
    Radar();
    virtual ~Radar();
    virtual void Update( float delta ) override;
    virtual void Render() override;

private:
    void CachePlayerShipPosition();
    void AddQuad( int atlasIndex, float x, float y, float scale = 1.0f );
    void AddShips();
    void AddShipyard();
    void AddIcon( int atlasIndex, const glm::vec3& position );

    Genesis::ResourceImage* m_pTexture;
    Genesis::Gui::Atlas m_Atlas;
    RadarIconVector m_Icons;

    // Atlas indices
    int m_BackgroundIdx;
    int m_BackgroundMaskIdx;
    int m_FriendlyShipIdx;
    int m_HostileShipIdx;
    int m_ShipyardIdx;

    // Rendering
    Genesis::VertexBuffer* m_pVertexBuffer;
    Genesis::Shader* m_pShader;
    glm::vec3 m_CachedPlayerShipPosition;
    Genesis::PositionData m_PositionData;
    Genesis::UVData m_UVData;
    size_t m_QuadCount;
};

} // namespace Hexterminate

#include "radar.inl"
