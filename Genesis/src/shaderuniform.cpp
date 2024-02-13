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

#include "shaderuniform.h"
#include "rendersystem.h"
#include "resources/resourceimage.h"

namespace Genesis
{

///////////////////////////////////////////////////////////////////////////////
// ShaderUniform
///////////////////////////////////////////////////////////////////////////////

ShaderUniform::ShaderUniform( GLuint handle, ShaderUniformType type, bool allowInstancingOverride /* = true */ )
    : m_Handle( handle )
    , m_Type( type )
    , m_pData( nullptr )
    , m_Slot( GL_TEXTURE0 )
    , m_InstancingOverride( allowInstancingOverride )
{
    SDL_assert( handle != -1 );

	if ( m_Type == ShaderUniformType::Boolean )
	{
		m_pData = new bool;
	}
    else if ( m_Type == ShaderUniformType::Integer )
    {
        m_pData = new int;
    }
    else if ( m_Type == ShaderUniformType::Float )
    {
        m_pData = new float;
    }
    else if ( m_Type == ShaderUniformType::FloatVector2 )
    {
        m_pData = new glm::vec2();
    }
    else if ( m_Type == ShaderUniformType::FloatVector3 )
    {
        m_pData = new glm::vec3();
    }
    else if ( m_Type == ShaderUniformType::FloatVector4 )
    {
        m_pData = new glm::vec4();
    }
    else if ( m_Type == ShaderUniformType::FloatMatrix44 )
    {
        m_pData = new glm::mat4();
    }
    else if ( m_Type == ShaderUniformType::Texture )
    {
        m_pData = new GLuint;
    }
}

ShaderUniform::~ShaderUniform()
{
	if ( m_Type == ShaderUniformType::Boolean )
	{
		delete (bool*)m_pData;
	}
    else if ( m_Type == ShaderUniformType::Integer )
    {
        delete (int*)m_pData;
    }
    else if ( m_Type == ShaderUniformType::Float )
    {
        delete (float*)m_pData;
    }
    else if ( m_Type == ShaderUniformType::FloatVector2 )
    {
        delete (glm::vec2*)m_pData;
    }
    else if ( m_Type == ShaderUniformType::FloatVector3 )
    {
        delete (glm::vec3*)m_pData;
    }
    else if ( m_Type == ShaderUniformType::FloatVector4 )
    {
        delete (glm::vec4*)m_pData;
    }
    else if ( m_Type == ShaderUniformType::FloatMatrix44 )
    {
        delete (glm::mat4*)m_pData;
    }
    else if ( m_Type == ShaderUniformType::Texture )
    {
        delete (GLuint*)m_pData;
    }
}

void ShaderUniform::Apply()
{
	if ( m_Type == ShaderUniformType::Boolean )
	{
		const int v = (*(bool*)m_pData) ? 1 : 0;
		glUniform1i( m_Handle, v);
	}
    else if ( m_Type == ShaderUniformType::Integer )
    {
        glUniform1i( m_Handle, *(int*)m_pData );
    }
    else if ( m_Type == ShaderUniformType::Float )
    {
        glUniform1f( m_Handle, *(float*)m_pData );
    }
    else if ( m_Type == ShaderUniformType::FloatVector2 )
    {
        glUniform2fv( m_Handle, 1, &( *(glm::vec2*)m_pData )[ 0 ] );
    }
    else if ( m_Type == ShaderUniformType::FloatVector3 )
    {
        glUniform3fv( m_Handle, 1, &( *(glm::vec3*)m_pData )[ 0 ] );
    }
    else if ( m_Type == ShaderUniformType::FloatVector4 )
    {
        glUniform4fv( m_Handle, 1, &( *(glm::vec4*)m_pData )[ 0 ] );
    }
    else if ( m_Type == ShaderUniformType::FloatMatrix44 )
    {
        glUniformMatrix4fv( m_Handle, 1, GL_FALSE, &( *(glm::mat4*)m_pData )[ 0 ][ 0 ] );
    }
    else if ( m_Type == ShaderUniformType::Texture )
    {
        glActiveTexture( m_Slot );
        glBindTexture( GL_TEXTURE_2D, *(GLuint*)m_pData );
        glUniform1i( m_Handle, (int)( m_Slot - GL_TEXTURE0 ) );
    }
    else
    {
        SDL_assert( false ); // ShaderUniformType enum entry not implemented
    }
}
}
