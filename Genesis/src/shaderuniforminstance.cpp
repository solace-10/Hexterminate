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

#include "shaderuniforminstance.h"
#include "rendersystem.h"
#include "resources/resourceimage.h"

namespace Genesis
{

///////////////////////////////////////////////////////////////////////////////
// ShaderUniformInstance
///////////////////////////////////////////////////////////////////////////////

ShaderUniformInstance::ShaderUniformInstance( ShaderUniform* pShaderUniform )
    : m_pShaderUniform( pShaderUniform )
    , m_Integer( 0 )
    , m_Float( 0.0f )
    , m_Vector( 0.0f )
    , m_Matrix( 0.0f )
    , m_pTexture( nullptr )
    , m_TextureSlot( GL_TEXTURE0 )
{
    SDL_assert( pShaderUniform != nullptr );
}

void ShaderUniformInstance::Apply()
{
    if ( m_pShaderUniform->IsInstancingOverrideAllowed() == false )
    {
        return;
    }

    ShaderUniformType type = m_pShaderUniform->GetType();

    if ( type == ShaderUniformType::Integer )
    {
        m_pShaderUniform->Set( m_Integer );
    }
    else if ( type == ShaderUniformType::Float )
    {
        m_pShaderUniform->Set( m_Float );
    }
    else if ( type == ShaderUniformType::FloatVector2 )
    {
        m_pShaderUniform->Set( glm::vec2( m_Vector.x, m_Vector.y ) );
    }
    else if ( type == ShaderUniformType::FloatVector3 )
    {
        m_pShaderUniform->Set( glm::vec3( m_Vector.x, m_Vector.y, m_Vector.z ) );
    }
    else if ( type == ShaderUniformType::FloatVector4 )
    {
        m_pShaderUniform->Set( m_Vector );
    }
    else if ( type == ShaderUniformType::FloatMatrix44 )
    {
        m_pShaderUniform->Set( m_Matrix );
    }
    else if ( type == ShaderUniformType::Texture )
    {
        m_pShaderUniform->Set( m_pTexture, m_TextureSlot );
    }
    else
    {
        SDL_assert( false );
    }
}
}
