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

#include <glm/gtx/transform.hpp>

#include "rendersystem.fwd.h"

#include "resources/resourceimage.h"
#include "shaderuniformtype.h"

namespace Genesis
{

///////////////////////////////////////////////////////////////////////////////
// ShaderUniform
///////////////////////////////////////////////////////////////////////////////

class ShaderUniform
{
public:
    ShaderUniform( GLuint handle, ShaderUniformType type, bool allowInstancingOverride = true );
    ~ShaderUniform();

    void Apply();

	void Set( bool value );
    void Set( int value );
    void Set( float value );
    void Set( const glm::vec2& value );
    void Set( const glm::vec3& value );
    void Set( const glm::vec4& value );
    void Set( const glm::mat4& value );
    void Set( ResourceImage* pTexture, GLenum textureSlot );
    void Set( GLuint textureID, GLenum textureSlot );

	void Get( bool* pValue ) const;
    void Get( int* pValue ) const;
    void Get( float* pValue ) const;
    void Get( glm::vec2* pValue ) const;
    void Get( glm::vec3* pValue ) const;
    void Get( glm::vec4* pValue ) const;
    void Get( glm::mat4* pValue ) const;

    void AllowInstancingOverride( bool state );
    bool IsInstancingOverrideAllowed() const;

    GLuint GetHandle() const;
    ShaderUniformType GetType() const;

private:
    GLuint m_Handle;
    ShaderUniformType m_Type;
    void* m_pData;
    GLenum m_Slot;
    bool m_InstancingOverride;
};

inline void ShaderUniform::Set( bool value )
{
	SDL_assert( m_Type == ShaderUniformType::Boolean );
	*(bool*)m_pData = value;
}

inline void ShaderUniform::Set( int value )
{
    SDL_assert( m_Type == ShaderUniformType::Integer );
    *(int*)m_pData = value;
}

inline void ShaderUniform::Set( float value )
{
    SDL_assert( m_Type == ShaderUniformType::Float );
    *(float*)m_pData = value;
}

inline void ShaderUniform::Set( const glm::vec2& value )
{
    SDL_assert( m_Type == ShaderUniformType::FloatVector2 );
    *(glm::vec2*)m_pData = value;
}

inline void ShaderUniform::Set( const glm::vec3& value )
{
    SDL_assert( m_Type == ShaderUniformType::FloatVector3 );
    *(glm::vec3*)m_pData = value;
}

inline void ShaderUniform::Set( const glm::vec4& value )
{
    SDL_assert( m_Type == ShaderUniformType::FloatVector4 );
    *(glm::vec4*)m_pData = value;
}

inline void ShaderUniform::Set( const glm::mat4& value )
{
    SDL_assert( m_Type == ShaderUniformType::FloatMatrix44 );
    *(glm::mat4*)m_pData = value;
}

inline void ShaderUniform::Set( ResourceImage* pImage, GLenum textureSlot )
{
    SDL_assert( m_Type == ShaderUniformType::Texture );
    *(GLuint*)m_pData = pImage->GetTexture();
    m_Slot = textureSlot;
}

inline void ShaderUniform::Set( GLuint textureID, GLenum textureSlot )
{
    SDL_assert( m_Type == ShaderUniformType::Texture );
    *(GLuint*)m_pData = textureID;
    m_Slot = textureSlot;
}

inline void ShaderUniform::Get( bool* pValue ) const
{
	SDL_assert( m_Type == ShaderUniformType::Boolean );
	*pValue = *(bool*)m_pData;
}

inline void ShaderUniform::Get( int* pValue ) const
{
    SDL_assert( m_Type == ShaderUniformType::Integer );
    *pValue = *(int*)m_pData;
}

inline void ShaderUniform::Get( float* pValue ) const
{
    SDL_assert( m_Type == ShaderUniformType::Float );
    *pValue = *(float*)m_pData;
}

inline void ShaderUniform::Get( glm::vec2* pValue ) const
{
    SDL_assert( m_Type == ShaderUniformType::FloatVector2 );
    *pValue = *(glm::vec2*)m_pData;
}

inline void ShaderUniform::Get( glm::vec3* pValue ) const
{
    SDL_assert( m_Type == ShaderUniformType::FloatVector3 );
    *pValue = *(glm::vec3*)m_pData;
}

inline void ShaderUniform::Get( glm::vec4* pValue ) const
{
    SDL_assert( m_Type == ShaderUniformType::FloatVector4 );
    *pValue = *(glm::vec4*)m_pData;
}

inline void ShaderUniform::Get( glm::mat4* pValue ) const
{
    SDL_assert( m_Type == ShaderUniformType::FloatMatrix44 );
    *pValue = *(glm::mat4*)m_pData;
}

inline GLuint ShaderUniform::GetHandle() const
{
    return m_Handle;
}

inline ShaderUniformType ShaderUniform::GetType() const
{
    return m_Type;
}

inline void ShaderUniform::AllowInstancingOverride( bool state )
{
    SDL_assert( !( m_InstancingOverride == false && state == true ) );
    m_InstancingOverride = state;
}

inline bool ShaderUniform::IsInstancingOverrideAllowed() const
{
    return m_InstancingOverride;
}
}
