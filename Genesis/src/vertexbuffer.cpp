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

#include "vertexbuffer.h"
#include "genesis.h"
#include "rendersystem.h"

namespace Genesis
{

///////////////////////////////////////////////////////////////////////////////
// VertexBuffer
///////////////////////////////////////////////////////////////////////////////

VertexBuffer::VertexBuffer( GeometryType type, unsigned int flags )
    : m_Flags( flags )
    , m_VAO( 0 )
    , m_Position( 0 )
    , m_UV( 0 )
    , m_Normal( 0 )
    , m_Colour( 0 )
    , m_Mode( GL_TRIANGLES )
{
    m_Size.fill( 0 );

    SDL_assert( flags & VBO_POSITION );

    glGenVertexArrays( 1, &m_VAO );
    glBindVertexArray( m_VAO );

    glGenBuffers( 1, &m_Position );

    if ( flags & VBO_UV )
    {
        glGenBuffers( 1, &m_UV );
    }

    if ( flags & VBO_NORMAL )
    {
        glGenBuffers( 1, &m_Normal );
    }

    if ( flags & VBO_COLOUR )
    {
        glGenBuffers( 1, &m_Colour );
    }

    SetModeFromGeometryType( type );
}

VertexBuffer::~VertexBuffer()
{
    if ( m_Position != -1 )
    {
        glDeleteBuffers( 1, &m_Position );
    }

    if ( m_UV != -1 )
    {
        glDeleteBuffers( 1, &m_UV );
    }

    if ( m_Normal != -1 )
    {
        glDeleteBuffers( 1, &m_Normal );
    }

    if ( m_Colour != -1 )
    {
        glDeleteBuffers( 1, &m_Colour );
    }

    glDeleteVertexArrays( 1, &m_VAO );
}

void VertexBuffer::SetModeFromGeometryType( GeometryType type )
{
    if ( type == GeometryType::Triangle )
    {
        m_Mode = GL_TRIANGLES;
    }
    else if ( type == GeometryType::Line )
    {
        m_Mode = GL_LINES;
    }
    else if ( type == GeometryType::LineStrip )
    {
        m_Mode = GL_LINE_STRIP;
    }
    else
    {
        SDL_assert( false );
    }
}

void VertexBuffer::CreateUntexturedQuad( float x, float y, float width, float height )
{
    const float x1 = x;
    const float x2 = x + width;
    const float y1 = y;
    const float y2 = y + height;

    const float vertices[] = {
        x1, y2, 0.0f,
        x1, y1, 0.0f,
        x2, y1, 0.0f,
        x1, y2, 0.0f,
        x2, y1, 0.0f,
        x2, y2, 0.0f,
    };

    CopyData( vertices, 18, VBO_POSITION );
}

void VertexBuffer::CreateUntexturedQuad( float x, float y, float width, float height, const glm::vec4& colour )
{
    CreateUntexturedQuad( x, y, width, height );

    const float colours[] = {
        colour.r, colour.g, colour.b, colour.a,
        colour.r, colour.g, colour.b, colour.a,
        colour.r, colour.g, colour.b, colour.a,
        colour.r, colour.g, colour.b, colour.a,
        colour.r, colour.g, colour.b, colour.a,
        colour.r, colour.g, colour.b, colour.a
    };

    CopyData( colours, 24, Genesis::VBO_COLOUR );
}

void VertexBuffer::CreateTexturedQuad( float x, float y, float width, float height )
{
    CreateUntexturedQuad( x, y, width, height );

    const float uvs[] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
    };

    CopyData( uvs, 12, Genesis::VBO_UV );
}

void VertexBuffer::CreateTexturedQuad( float x, float y, float width, float height, const glm::vec4& colour )
{
    CreateTexturedQuad( x, y, width, height );

    const float colours[] = {
        colour.r, colour.g, colour.b, colour.a,
        colour.r, colour.g, colour.b, colour.a,
        colour.r, colour.g, colour.b, colour.a,
        colour.r, colour.g, colour.b, colour.a,
        colour.r, colour.g, colour.b, colour.a,
        colour.r, colour.g, colour.b, colour.a
    };

    CopyData( colours, 24, Genesis::VBO_COLOUR );
}

void VertexBuffer::CopyPositions( const PositionData& data )
{
    CopyData( &data[ 0 ][ 0 ], data.size() * 3, VBO_POSITION );
}

void VertexBuffer::CopyPositions( const PositionData& data, size_t count )
{
	CopyData( &data[ 0 ][ 0 ], count * 3, VBO_POSITION );
} 

void VertexBuffer::CopyUVs( const UVData& data )
{
    CopyData( &data[ 0 ][ 0 ], data.size() * 2, VBO_UV );
}

void VertexBuffer::CopyUVs( const UVData& data, size_t count )
{
    CopyData( &data[ 0 ][ 0 ], count * 2, VBO_UV );
}

void VertexBuffer::CopyNormals( const NormalData& data )
{
    CopyData( &data[ 0 ][ 0 ], data.size() * 3, VBO_NORMAL );
}

void VertexBuffer::CopyNormals( const NormalData& data, size_t count )
{
    CopyData( &data[ 0 ][ 0 ], count * 3, VBO_NORMAL );
}

void VertexBuffer::CopyColours( const ColourData& data )
{
    CopyData( &data[ 0 ][ 0 ], data.size() * 4, VBO_COLOUR );
}

void VertexBuffer::CopyColours( const ColourData& data, size_t count )
{
    CopyData( &data[ 0 ][ 0 ], count * 4, VBO_COLOUR );
}

void VertexBuffer::CopyData( const float* pData, size_t size, unsigned int destination )
{
    size *= sizeof( float );

    if ( destination == VBO_POSITION )
    {
        SDL_assert( m_Flags & VBO_POSITION );
        glBindBuffer( GL_ARRAY_BUFFER, m_Position );
    }
    else if ( destination == VBO_UV )
    {
        SDL_assert( m_Flags & VBO_UV );
        glBindBuffer( GL_ARRAY_BUFFER, m_UV );
    }
    else if ( destination == VBO_NORMAL )
    {
        SDL_assert( m_Flags & VBO_NORMAL );
        glBindBuffer( GL_ARRAY_BUFFER, m_Normal );
    }
    else if ( destination == VBO_COLOUR )
    {
        SDL_assert( m_Flags & VBO_COLOUR );
        glBindBuffer( GL_ARRAY_BUFFER, m_Colour );
    }

    const unsigned int idx = GetSizeIndex( destination );
    if ( size <= m_Size[ idx ] )
    {
        glBufferSubData( GL_ARRAY_BUFFER, 0, size, pData );
    }
    else
    {
        glBufferData( GL_ARRAY_BUFFER, size, pData, GL_DYNAMIC_DRAW );
        m_Size[ idx ] = static_cast<uint32_t>(size);
    }
}

void VertexBuffer::Draw( uint32_t numVertices /* = 0 */ )
{
    Draw( 0, numVertices );
}

void VertexBuffer::Draw( uint32_t startVertex, uint32_t numVertices, void* pIndices /* = nullptr */ )
{
    glBindVertexArray( m_VAO );

	uint32_t maxVertices = m_Size[ GetSizeIndex( VBO_POSITION ) ] / ( (m_Flags & VB_2D) ? 2 : 3) / sizeof( float );
	SDL_assert( maxVertices > 0 );
	SDL_assert( startVertex + numVertices <= maxVertices );

	if ( numVertices == 0 )
	{
		numVertices = maxVertices;
	}

    if ( m_Flags & VBO_POSITION )
    {
        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, m_Position );
        glVertexAttribPointer(
            0, // attribute 0
			( m_Flags & VB_2D ) ? 2 : 3, // size
            GL_FLOAT,
            GL_FALSE, // normalised?
            0,
            (void*)0 // array buffer offset
            );
    }

    if ( m_Flags & VBO_UV )
    {
        glEnableVertexAttribArray( 1 );
        glBindBuffer( GL_ARRAY_BUFFER, m_UV );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    }

    if ( m_Flags & VBO_NORMAL )
    {
        glEnableVertexAttribArray( 2 );
        glBindBuffer( GL_ARRAY_BUFFER, m_Normal );
        glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    }

    if ( m_Flags & VBO_COLOUR )
    {
        glEnableVertexAttribArray( 3 );
        glBindBuffer( GL_ARRAY_BUFFER, m_Colour );
        glVertexAttribPointer( 3, 4, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    }

	if ( pIndices == nullptr )
	{
		glDrawArrays( m_Mode, startVertex, numVertices );
	}
	else
	{
		glDrawElements( m_Mode, numVertices, GL_UNSIGNED_SHORT, pIndices );
	}

    if ( m_Flags & VBO_POSITION )
    {
        glDisableVertexAttribArray( 0 );
    }

    if ( m_Flags & VBO_UV )
    {
        glDisableVertexAttribArray( 1 );
    }

    if ( m_Flags & VBO_NORMAL )
    {
        glDisableVertexAttribArray( 2 );
    }

    if ( m_Flags & VBO_COLOUR )
    {
        glDisableVertexAttribArray( 3 );
    }

    FrameWork::GetRenderSystem()->IncreaseDrawCallCount();
}
}
