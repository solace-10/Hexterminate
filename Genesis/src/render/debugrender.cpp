// Copyright 2015 Pedro Nunes
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

#include <cassert>

#include "math/constants.h"
#include "resources/resourcefont.h"
#include "rendersystem.h"
#include "shadercache.h"
#include "vertexbuffer.h"

#include "debugrender.h"

namespace Genesis
{
namespace Render
{

DebugRender::DebugRender() :
m_pShader( nullptr ),
m_pVertexBuffer( nullptr )
{
	using namespace Genesis;

	const float theta = 2.0f * static_cast<float>( Genesis::kPi ) / static_cast<float>( DEBUG_RENDER_CIRCLE_SIDES );
	for ( int i = 0; i < DEBUG_RENDER_CIRCLE_SIDES; ++i )
	{
		m_LookupCos[ i ] = cosf( theta * i );
		m_LookupSin[ i ] = sinf( theta * i );
	}

	m_pVertexBuffer = new VertexBuffer( GeometryType::Line, VBO_POSITION | VBO_COLOR );
	m_pShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "untextured_vertex_colored" );
}

DebugRender::~DebugRender()
{
	delete m_pVertexBuffer;
}

void DebugRender::Render()
{
	RenderLines();
	RenderCircles();
	RenderText();
}

void DebugRender::RenderLines()
{
	using namespace Genesis;

	if ( m_Lines.empty() )
	{
		return;
	}

	PositionData posData;
	ColorData colorData;

	for ( auto& line : m_Lines )
	{
		posData.push_back( line.start );
		posData.push_back( line.end );

		glm::vec4 color( line.color, 1.0f );
		colorData.push_back( color );
		colorData.push_back( color );
	}

	m_pVertexBuffer->CopyPositions( posData );
	m_pVertexBuffer->CopyColors( colorData );

	RenderSystem* pRenderSystem = FrameWork::GetRenderSystem();
	pRenderSystem->SetBlendMode( BlendMode::Blend );

	m_pShader->Use();

	pRenderSystem->SetRenderTarget( RenderTargetId::Default );
	m_pVertexBuffer->Draw(static_cast<uint32_t>(m_Lines.size() * 2));

	m_Lines.clear();
}

void DebugRender::RenderCircles()
{
	using namespace Genesis;

	if ( m_Circles.empty() )
	{
		return;
	}

	PositionData posData;
	ColorData colorData;

	for ( auto& circle : m_Circles )
	{
		glm::vec4 color( circle.color, 1.0f );

		for ( int j = 0; j < DEBUG_RENDER_CIRCLE_SIDES - 1; ++j )
		{
			posData.push_back( glm::vec3( circle.origin.x + circle.radius * m_LookupCos[ j     ], circle.origin.y + circle.radius * m_LookupSin[ j     ], circle.origin.z ) );
			posData.push_back( glm::vec3( circle.origin.x + circle.radius * m_LookupCos[ j + 1 ], circle.origin.y + circle.radius * m_LookupSin[ j + 1 ], circle.origin.z ) );
			colorData.push_back( color );
			colorData.push_back( color );
		}

		posData.push_back( glm::vec3( circle.origin.x + circle.radius * m_LookupCos[ DEBUG_RENDER_CIRCLE_SIDES - 1 ], circle.origin.y + circle.radius * m_LookupSin[ DEBUG_RENDER_CIRCLE_SIDES - 1 ], circle.origin.z ) );
		posData.push_back( glm::vec3( circle.origin.x + circle.radius * m_LookupCos[ 0 ], circle.origin.y + circle.radius * m_LookupSin[ 0 ], circle.origin.z ) );
		colorData.push_back( color );
		colorData.push_back( color );
	}

	m_pVertexBuffer->CopyPositions( posData );
	m_pVertexBuffer->CopyColors( colorData );

	RenderSystem* pRenderSystem = FrameWork::GetRenderSystem();
	pRenderSystem->SetBlendMode( BlendMode::Blend );

	m_pShader->Use();

	pRenderSystem->SetRenderTarget( RenderTargetId::Default );
	m_pVertexBuffer->Draw(static_cast<uint32_t>(m_Circles.size() * DEBUG_RENDER_CIRCLE_SIDES * 2));

	m_Circles.clear();
}

void DebugRender::RenderText()
{	
	// TODO_RENDERER

	//Genesis::ResourceFont* pFont = (Genesis::ResourceFont*)Genesis::FrameWork::GetResourceManager()->GetResource( "data/fonts/fixedsys16.fnt" );
	//Genesis::FrameWork::GetRenderSystem()->ViewOrtho();

	//for ( auto& data : m_Texts )
	//{
	//	pFont->Render( data.x, data.y, 0.0f, 0.0f, data.text, Genesis::Color( data.color.x, data.color.y, data.color(2), data.color(3) ) );
	//}

	//Genesis::FrameWork::GetRenderSystem()->ViewPerspective();

	m_Texts.clear();
}

void DebugRender::DrawLine( const glm::vec2& start, const glm::vec2& end, const glm::vec3& color )
{
	DrawLine( glm::vec3( start, 0.0f ), glm::vec3( end, 0.0f ), color );
}

void DebugRender::DrawLine( const glm::vec3& start, const glm::vec3& end, const glm::vec3& color )
{
	DebugRenderLine line;
	line.start = start;
	line.end = end;
	line.color = color;
	m_Lines.push_back( line );
}

void DebugRender::DrawCircle( const glm::vec2& origin, float radius, const glm::vec3& color )
{
	DrawCircle( glm::vec3( origin, 0.0f ), radius, color );
}

void DebugRender::DrawCircle( const glm::vec3& origin, float radius, const glm::vec3& color )
{
	DebugRenderCircle circle;
	circle.origin = origin;
	circle.radius = radius;
	circle.color = color;
	m_Circles.push_back( circle );
}

void DebugRender::DrawText( float x, float y, const std::string& str, const glm::vec3& color )
{
	DebugRenderText text;
	text.x = x;
	text.y = y;
	text.text = str;
	text.color = color;
	m_Texts.push_back( text );
}

} // namespace Render
} // namespace Genesis