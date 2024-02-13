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

#include <vector>

#include "glm/gtx/transform.hpp"
#include "rendersystem.fwd.h"
#include "shaderuniform.h"

namespace Genesis
{

class ResourceImage;
class ShaderUniformInstance;

typedef std::vector<ShaderUniformInstance> ShaderUniformInstances;

///////////////////////////////////////////////////////////////////////////////
// ShaderUniformInstance
///////////////////////////////////////////////////////////////////////////////

class ShaderUniformInstance
{
public:
    ShaderUniformInstance( ShaderUniform* pShaderUniform );

    void Apply();

    void Set( int value );
    void Set( float value );
    void Set( const glm::vec2& value );
    void Set( const glm::vec3& value );
    void Set( const glm::vec4& value );
    void Set( const glm::mat4& value );
    void Set( ResourceImage* pTexture, GLenum textureSlot );

private:
    ShaderUniform* m_pShaderUniform;
    int m_Integer;
    float m_Float;
    glm::vec4 m_Vector;
    glm::mat4 m_Matrix;
    ResourceImage* m_pTexture;
    GLenum m_TextureSlot;
};

inline void ShaderUniformInstance::Set( int value )
{
    m_Integer = value;
}

inline void ShaderUniformInstance::Set( float value )
{
    m_Float = value;
}

inline void ShaderUniformInstance::Set( const glm::vec2& value )
{
    m_Vector = glm::vec4( value.x, value.y, 0.0f, 0.0f );
}

inline void ShaderUniformInstance::Set( const glm::vec3& value )
{
    m_Vector = glm::vec4( value.x, value.y, value.z, 0.0f );
}

inline void ShaderUniformInstance::Set( const glm::vec4& value )
{
    m_Vector = value;
}

inline void ShaderUniformInstance::Set( const glm::mat4& value )
{
    m_Matrix = value;
}

inline void ShaderUniformInstance::Set( ResourceImage* pTexture, GLenum textureSlot )
{
    m_pTexture = pTexture;
    m_TextureSlot = textureSlot;
}
}
