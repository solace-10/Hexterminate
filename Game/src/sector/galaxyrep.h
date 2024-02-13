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

#include <vector>

// clang-format off
#include <beginexternalheaders.h>
#include <glm/vec2.hpp>
#include <endexternalheaders.h>
// clang-format on

#include <color.h>
#include <scene/layer.h>
#include <scene/sceneobject.h>
#include <shader.h>
#include <shaderuniforminstance.h>

#include "menus/sectordetails.h"
#include "misc/mathaux.h"
#include "requests/requestgoal.h"
#include "ui/window.h"

namespace Genesis
{
class Layer;
class ResourceImage;
class VertexBuffer;

namespace Gui
{
    class Text;
}
} // namespace Genesis

namespace Hexterminate
{

class Galaxy;
class SectorInfo;
class SectorDetails;
class GalaxyWindow;
using GalaxyWindowSharedPtr = std::shared_ptr<GalaxyWindow>;

/////////////////////////////////////////////////////////////////////
// SectorDrawInfo
/////////////////////////////////////////////////////////////////////

struct SectorDrawInfo
{
    SectorDrawInfo()
    {
        x = 0;
        y = 0;
    };

    SectorDrawInfo( int _x, int _y )
    {
        x = _x;
        y = _y;
    };

    Genesis::Color colour;
    int x, y;
};

typedef std::vector<SectorDrawInfo> SectorDrawInfoVector;

/////////////////////////////////////////////////////////////////////
// GalaxyRep
/////////////////////////////////////////////////////////////////////

class GalaxyRep : public Genesis::SceneObject
{
public:
    GalaxyRep( Galaxy* pGalaxy );
    virtual ~GalaxyRep();

    void Initialise();
    void OnGalaxyPopulated();
    void OnGalaxyReset();
    void RemoveFromScene();

    virtual void Update( float delta );
    virtual void Render();

    void Show( bool state );
    inline bool IsVisible() const { return m_Show; }
    inline void GetOffset( float& offsetX, float& offsetY ) const
    {
        offsetX = m_OffsetX;
        offsetY = m_OffsetY;
    }
    Math::FPoint2 GetOffset() const { return Math::FPoint2( m_OffsetX, m_OffsetY ); }

    GalaxyWindow* GetGalaxyWindow() const;
    const glm::vec2& GetSize() const;

private:
    void UpdateInput();
    void UpdateDrawInfo();
    void UpdateGoalDrawInfo();

    void DrawBackground();
    void DrawSectors( SectorDrawInfoVector& drawInfoVec, Genesis::Shader* pShader, Genesis::ShaderUniformInstances* pShaderUniforms, bool useFactionColour );
    void DrawSectorsThreatRatings();
    void DrawHomeworldSectors();
    void DrawGrid();
    void DrawGoals();

    void FocusOnPlayerFleet(); // Sets the offsets needed to draw the galaxy correctly, while trying to player's fleet centered.
    void SetHoverSector(); // Decides which sector lies underneath the mouse pointer, if any. Result can be nullptr.

    void OnLeftMouseButtonDown();

    Galaxy* m_pGalaxy;
    bool m_Show;

    Genesis::Shader* m_pBackgroundShader;
    Genesis::VertexBuffer* m_pBackgroundVB;

    Genesis::Shader* m_pSectorShader;
    Genesis::Shader* m_pSectorShipyardShader;
    Genesis::Shader* m_pSectorProbeShader;
    Genesis::Shader* m_pSectorStarfortShader;
    Genesis::Shader* m_pSectorInhibitorShader;
    Genesis::Shader* m_pSectorHomeworldShader;
    Genesis::VertexBuffer* m_pSectorVB;
    Genesis::VertexBuffer* m_pSectorInhibitorVB;
    Genesis::VertexBuffer* m_pSectorHomeworldVB;

    Genesis::Shader* m_pSectorCrossShader;
    Genesis::VertexBuffer* m_pSectorCrossVB;
    Genesis::ShaderUniformInstances m_SectorCrossUniforms;

    Genesis::ShaderUniform* m_pSectorHomeworldDiffuseSampler;
    std::array<Genesis::ResourceImage*, (int)FactionId::Count> m_HomeworldImages;

    Genesis::Shader* m_pSectorThreatShader;
    std::array<Genesis::ShaderUniformInstances, static_cast<size_t>( ThreatRating::Count )> m_pSectorThreatUniforms;

    Genesis::VertexBuffer* m_pGoalTargetVB;

    Genesis::LayerSharedPtr m_pLayer;

    float m_OffsetX;
    float m_OffsetY;

    SectorInfo* m_pHoverSector;
    SectorDetails* m_pSectorDetails;

    bool m_ExitMenu;

    SectorDrawInfoVector m_SectorDrawInfo;
    SectorDrawInfoVector m_SectorDrawInfoShipyard;
    SectorDrawInfoVector m_SectorDrawInfoProbes;
    SectorDrawInfoVector m_SectorDrawInfoStarforts;
    SectorDrawInfoVector m_SectorDrawInfoInhibitors;

    GalaxyWindowSharedPtr m_pGalaxyWindow;
    Genesis::InputCallbackToken m_LeftMouseButtonDownToken;

    bool m_InputPending;
    uint32_t m_InputTimer;

    glm::vec2 m_Size;
};

inline const glm::vec2& GalaxyRep::GetSize() const
{
    return m_Size;
}

} // namespace Hexterminate