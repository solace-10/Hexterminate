// Copyright 2016 Pedro Nunes
//
// This file is part of Genesis.
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

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <genesis.h>
#include <resourcemanager.h>
#include <scene/sceneobject.h>

namespace Genesis
{
class VertexBuffer;
class Shader;

namespace Render
{

///////////////////////////////////////////////////////////
// DebugRender
// Allows rendering of debug lines, circles and text 
///////////////////////////////////////////////////////////

static const int DEBUG_RENDER_CIRCLE_SIDES = 12;

class DebugRender : public SceneObject
{
public:
	DebugRender();
	virtual ~DebugRender() override;

	virtual void Render() override;

	void DrawLine( const glm::vec2& start, const glm::vec2& end, const glm::vec3& colour );
	void DrawLine( const glm::vec3& start, const glm::vec3& end, const glm::vec3& colour );
	void DrawCircle( const glm::vec2& origin, float radius, const glm::vec3& colour );
	void DrawCircle( const glm::vec3& origin, float radius, const glm::vec3& colour );
	void DrawText( float x, float y, const std::string& text, const glm::vec3& colour ); // Text is top-left aligned

private:

	void RenderLines();
	void RenderCircles();
	void RenderText();

	struct DebugRenderLine
	{
		glm::vec3 start;
		glm::vec3 end;
		glm::vec3 colour;
	};

	struct DebugRenderCircle
	{
		glm::vec3 origin;
		glm::vec3 colour;
		float radius;
	};

	struct DebugRenderText
	{
		float x;
		float y;
		glm::vec3 colour;
		std::string text;
	};

	typedef std::vector< DebugRenderLine > DebugRenderLineVec;
	typedef std::vector< DebugRenderCircle > DebugRenderCircleVec;
	typedef std::vector< DebugRenderText > DebugRenderTextVec;

	DebugRenderLineVec m_Lines;
	DebugRenderCircleVec m_Circles;
	DebugRenderTextVec m_Texts;

	float m_LookupCos[ DEBUG_RENDER_CIRCLE_SIDES ];
	float m_LookupSin[ DEBUG_RENDER_CIRCLE_SIDES ];

	Shader* m_pShader;
	VertexBuffer* m_pVertexBuffer;
};

} // namespace Render
} // namespace Genesis