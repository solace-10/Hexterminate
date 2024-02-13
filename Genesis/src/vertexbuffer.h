// Copyright 2016 Pedro Nunes
//
// This file is part of Genesis.
//
// Genesis is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Genesis is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Genesis. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <array>
#include <vector>

#include "glm/gtx/transform.hpp"
#include "rendersystem.fwd.h"
#include "coredefines.h"

namespace Genesis
{

///////////////////////////////////////////////////////////////////////////////
// VertexBuffer
///////////////////////////////////////////////////////////////////////////////

static const unsigned int VBO_POSITION = 1;
static const unsigned int VBO_UV = 1 << 1;
static const unsigned int VBO_NORMAL = 1 << 2;
static const unsigned int VBO_COLOUR = 1 << 3;
static const unsigned int VB_2D = 1 << 4;

typedef std::vector<glm::vec3> PositionData;
typedef std::vector<glm::vec2> UVData;
typedef std::vector<glm::vec3> NormalData;
typedef std::vector<glm::vec4> ColourData;

enum class GeometryType
{
    Triangle,
    Line,
    LineStrip
};

class VertexBuffer
{
public:
    VertexBuffer( GeometryType type, unsigned int flags );
    ~VertexBuffer();

    void CopyPositions( const PositionData& data );
    void CopyPositions( const PositionData& data, size_t count );
    void CopyUVs( const UVData& data );
    void CopyUVs( const UVData& data, size_t count );
    void CopyNormals( const NormalData& data );
    void CopyNormals( const NormalData& data, size_t count );
    void CopyColours( const ColourData& data );
    void CopyColours( const ColourData& data, size_t count );
    void CopyData( const float* pData, size_t count, unsigned int destination );

    void Draw( uint32_t numVertices = 0 ); // Draw the vertex buffer. Passing 0 to this function will draw the entire buffer.
    void Draw( uint32_t startVertex, uint32_t numVertices, void* pIndices = nullptr );

    void CreateUntexturedQuad( float x, float y, float width, float height );
    void CreateUntexturedQuad( float x, float y, float width, float height, const glm::vec4& colour );
    void CreateTexturedQuad( float x, float y, float width, float height );
    void CreateTexturedQuad( float x, float y, float width, float height, const glm::vec4& colour );

private:
    void SetModeFromGeometryType( GeometryType type );
    unsigned int GetSizeIndex( unsigned int flag ) const;

    unsigned int m_Flags;
    GLuint m_VAO;
    GLuint m_Position;
    GLuint m_UV;
    GLuint m_Normal;
    GLuint m_Colour;
    std::array<uint32_t, 4> m_Size;
    GLenum m_Mode;
};
GENESIS_DECLARE_SMART_PTR( VertexBuffer );

// Translates one of the VBO_ flags into an actual size index which we can use.
// log( flag ) / log( 2 ) is the same as log2( flag ) but log2 isn't available in this version of VS for some reason.
inline unsigned int VertexBuffer::GetSizeIndex( unsigned int flag ) const
{
    return (unsigned int)( log( flag ) / log( 2 ) );
}
}
