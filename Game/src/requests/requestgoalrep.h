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

#include <memory>

#include "glm/vec2.hpp"
#include "requests/requestgoal.h"

namespace Genesis
{
class Shader;
class VertexBuffer;
class ResourceImage;

namespace Gui
{
    class Text;
    class Panel;
} // namespace Gui
} // namespace Genesis

namespace Hexterminate
{

class RequestGoalRep;
class GalaxyRep;
class PointOfInterest;

typedef std::shared_ptr<RequestGoalRep> RequestGoalRepSharedPtr;
typedef std::weak_ptr<RequestGoalRep> RequestGoalRepWeakPtr;

/////////////////////////////////////////////////////////////////////
// RequestGoalRep
// Representation of a goal in the Galaxy view.
/////////////////////////////////////////////////////////////////////

class RequestGoalRep
{
public:
    RequestGoalRep( RequestGoal* pOwner, const Color& colour );
    ~RequestGoalRep();

    void Update( GalaxyRep* pGalaxyRep );
    void Render();
    void Show( bool state );
    bool IsVisible() const;

private:
    RequestGoal* m_pOwner;
    glm::vec2 m_Position;

    Genesis::Gui::Text* m_pDescription;
    Genesis::Gui::Panel* m_pBackground;
    PointOfInterest* m_pPointOfInterest;
    bool m_Show;
    Genesis::Shader* m_pShader;
    Genesis::VertexBuffer* m_pVertexBuffer;
    Genesis::ResourceImage* m_pGoalTargetImage;
};

} // namespace Hexterminate
