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

#include <gui/atlas.h>
#include <gui/gui.h>
#include <rendersystem.h>

namespace Genesis
{
class Shader;
class VertexBuffer;
} // namespace Genesis

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// PointOfInterestTarget
///////////////////////////////////////////////////////////////////////////////

class PointOfInterestTarget
{
public:
    const glm::vec2& GetPointOfInterestEnd() const;
    void SetPointOfInterestEnd( const glm::vec2& position );

private:
    glm::vec2 m_PointOfInterestEnd;
};

inline const glm::vec2& PointOfInterestTarget::GetPointOfInterestEnd() const
{
    return m_PointOfInterestEnd;
}

inline void PointOfInterestTarget::SetPointOfInterestEnd( const glm::vec2& position )
{
    m_PointOfInterestEnd = position;
}

///////////////////////////////////////////////////////////////////////////
// PointOfInterest
// Used to indicate a location in 2D space which is relevant to the player.
// It is represented by a line starting at the start position and ending at
// the point of interest's end with a small circular icon.
// A dynamic origin can be set by calling SetOrigin() with a class inheriting
// from PointOfInterestTarget.
///////////////////////////////////////////////////////////////////////////////

class PointOfInterest : public Genesis::Gui::GuiElement
{
public:
    PointOfInterest();
    virtual ~PointOfInterest();
    virtual void Render() override;

    void SetStart( const glm::vec2& position );
    void SetEnd( const glm::vec2& position );
    void SetEnd( PointOfInterestTarget* pPointOfInterestTarget );
    void SetColour( const Genesis::Color& colour );

private:
    glm::vec2 m_Start;
    glm::vec2 m_StaticEnd;
    PointOfInterestTarget* m_pDynamicEnd;

    Genesis::Shader* m_pShader;
    Genesis::VertexBuffer* m_pVertexBuffer;

    Genesis::Color m_Colour;
};

inline void PointOfInterest::SetStart( const glm::vec2& position )
{
    m_Start = position;
}

inline void PointOfInterest::SetEnd( const glm::vec2& position )
{
    m_StaticEnd = position;
    m_pDynamicEnd = nullptr;
}

inline void PointOfInterest::SetEnd( PointOfInterestTarget* pPointOfInterestTarget )
{
    m_pDynamicEnd = pPointOfInterestTarget;
}

inline void PointOfInterest::SetColour( const Genesis::Color& colour )
{
    m_Colour = colour;
}

} // namespace Hexterminate