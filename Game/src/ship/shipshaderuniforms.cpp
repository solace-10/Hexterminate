// Copyright 2016 Pedro Nunes
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

#include <genesis.h>
#include <rendersystem.h>
#include <shader.h>
#include <shadercache.h>
#include <shaderuniform.h>

#include "shipshaderuniforms.h"

namespace Hexterminate
{

ShipShaderUniforms::ShipShaderUniforms()
    : m_pShader( nullptr )
{
    using namespace Genesis;

    m_pShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "shipmodule" );

    m_Uniforms[ (int)ShipShaderUniform::PrimaryPaint ] = m_pShader->RegisterUniform( "k_primaryPaint", ShaderUniformType::FloatVector4, false );
    m_Uniforms[ (int)ShipShaderUniform::SecondaryPaint ] = m_pShader->RegisterUniform( "k_secondaryPaint", ShaderUniformType::FloatVector4, false );
    m_Uniforms[ (int)ShipShaderUniform::Health ] = m_pShader->RegisterUniform( "k_health", ShaderUniformType::Float, false );
    m_Uniforms[ (int)ShipShaderUniform::RepairEdgeAlpha ] = m_pShader->RegisterUniform( "k_repairEdgeAlpha", ShaderUniformType::Float, false );
    m_Uniforms[ (int)ShipShaderUniform::RepairEdgeOffset ] = m_pShader->RegisterUniform( "k_repairEdgeOffset", ShaderUniformType::Float, false );
    m_Uniforms[ (int)ShipShaderUniform::ClipActive ] = m_pShader->RegisterUniform( "k_clipActive", ShaderUniformType::Integer, false );
    m_Uniforms[ (int)ShipShaderUniform::Clip ] = m_pShader->RegisterUniform( "k_clip", ShaderUniformType::FloatVector4, false );
    m_Uniforms[ (int)ShipShaderUniform::ClipForward ] = m_pShader->RegisterUniform( "k_clipForward", ShaderUniformType::FloatVector4, false );
    m_Uniforms[ (int)ShipShaderUniform::AmbientColour ] = m_pShader->RegisterUniform( "k_a", ShaderUniformType::FloatVector4, false );
    m_Uniforms[ (int)ShipShaderUniform::EmissiveColour ] = m_pShader->RegisterUniform( "k_e", ShaderUniformType::FloatVector4, false );
    m_Uniforms[ (int)ShipShaderUniform::OverlayColour ] = m_pShader->RegisterUniform( "k_overlayColour", ShaderUniformType::FloatVector4, false );
    m_Uniforms[ (int)ShipShaderUniform::EMPActive ] = m_pShader->RegisterUniform( "k_empActive", ShaderUniformType::Integer, false );
    m_Uniforms[ (int)ShipShaderUniform::DiffuseMap ] = m_pShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    m_Uniforms[ (int)ShipShaderUniform::SpecularMap ] = m_pShader->RegisterUniform( "k_sampler1", ShaderUniformType::Texture );
    m_Uniforms[ (int)ShipShaderUniform::PaintMap ] = m_pShader->RegisterUniform( "k_sampler2", ShaderUniformType::Texture );
    m_Uniforms[ (int)ShipShaderUniform::DamageMap ] = m_pShader->RegisterUniform( "k_sampler3", ShaderUniformType::Texture );
}

void ShipShaderUniforms::Set( ShipShaderUniform shipShaderUniform, int value )
{
    Genesis::ShaderUniform* pShaderUniform = m_Uniforms[ (int)shipShaderUniform ];
    if ( pShaderUniform != nullptr )
    {
        pShaderUniform->Set( value );
    }
}

void ShipShaderUniforms::Set( ShipShaderUniform shipShaderUniform, float value )
{
    Genesis::ShaderUniform* pShaderUniform = m_Uniforms[ (int)shipShaderUniform ];
    if ( pShaderUniform != nullptr )
    {
        pShaderUniform->Set( value );
    }
}

void ShipShaderUniforms::Set( ShipShaderUniform shipShaderUniform, const glm::vec4& value )
{
    Genesis::ShaderUniform* pShaderUniform = m_Uniforms[ (int)shipShaderUniform ];
    if ( pShaderUniform != nullptr )
    {
        pShaderUniform->Set( value );
    }
}

void ShipShaderUniforms::Set( ShipShaderUniform shipShaderUniform, Genesis::ResourceImage* pTexture )
{
    Genesis::ShaderUniform* pShaderUniform = m_Uniforms[ (int)shipShaderUniform ];
    if ( pShaderUniform != nullptr && pTexture != nullptr )
    {
        pShaderUniform->Set( pTexture, UniformToGL( shipShaderUniform ) );
    }
}

GLenum ShipShaderUniforms::UniformToGL( ShipShaderUniform uniform ) const
{
    SDL_assert( uniform >= ShipShaderUniform::DiffuseMap && uniform <= ShipShaderUniform::DamageMap );
    if ( uniform == ShipShaderUniform::DiffuseMap )
    {
        return GL_TEXTURE0;
    }
    else if ( uniform == ShipShaderUniform::SpecularMap )
    {
        return GL_TEXTURE1;
    }
    else if ( uniform == ShipShaderUniform::PaintMap )
    {
        return GL_TEXTURE2;
    }
    else if ( uniform == ShipShaderUniform::DamageMap )
    {
        return GL_TEXTURE3;
    }
    else
    {
        return GL_TEXTURE0;
    }
}

} // namespace Hexterminate
