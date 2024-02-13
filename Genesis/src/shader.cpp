// Copyright 2015 Pedro Nunes
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

#include <cmath>

#include "configuration.h"
#include "genesis.h"
#include "memory.h"
#include "rendersystem.h"
#include "shader.h"
#include "shadercache.h"
#include "shaderuniform.h"
#include "shaderuniforminstance.h"

namespace Genesis
{

Shader::Shader( const std::string& programName, GLuint programHandle )
    : m_ShaderName( programName )
    , m_ProgramHandle( programHandle )
    , m_pModelViewProjectionUniform( nullptr )
    , m_pModelUniform( nullptr )
    , m_pModelInverseUniform( nullptr )
    , m_pModelInverseTransposeUniform( nullptr )
    , m_pViewInverseUniform( nullptr )
    , m_pTimeUniform( nullptr )
    , m_pResolutionUniform( nullptr )
{
    RegisterCoreUniforms();
}

Shader::~Shader()
{
    glDeleteProgram( m_ProgramHandle );

    for ( auto& pShaderUniform : m_Uniforms )
    {
        delete pShaderUniform;
    }
}

void Shader::RegisterCoreUniforms()
{
    m_pModelViewProjectionUniform = RegisterUniform( "k_worldViewProj", ShaderUniformType::FloatMatrix44 );
    m_pModelUniform = RegisterUniform( "k_world", ShaderUniformType::FloatMatrix44 );
    m_pModelInverseUniform = RegisterUniform( "k_worldInverse", ShaderUniformType::FloatMatrix44 );
    m_pModelInverseTransposeUniform = RegisterUniform( "k_worldInverseTranspose", ShaderUniformType::FloatMatrix44 );
    m_pViewInverseUniform = RegisterUniform( "k_viewInverse", ShaderUniformType::FloatMatrix44 );
    m_pTimeUniform = RegisterUniform( "k_time", ShaderUniformType::Float );
    m_pResolutionUniform = RegisterUniform( "k_resolution", ShaderUniformType::FloatVector2 );
}

ShaderUniform* Shader::RegisterUniform( const char* pUniformName, ShaderUniformType type, bool allowInstancingOverride /*= true */ )
{
    GLuint handle = glGetUniformLocation( m_ProgramHandle, pUniformName );
    for ( auto& pUniform : m_Uniforms )
    {
        if ( pUniform->GetHandle() == handle )
        {
            if ( pUniform->IsInstancingOverrideAllowed() == true && allowInstancingOverride == false )
            {
                pUniform->AllowInstancingOverride( false );
            }

            return pUniform;
        }
    }

    ShaderUniform* pUniform = nullptr;
    if ( handle != -1 )
    {
        pUniform = new ShaderUniform( handle, type, allowInstancingOverride );
        m_Uniforms.push_back( pUniform );
    }
    return pUniform;
}

void Shader::Use( ShaderUniformInstances* pShaderUniformInstances /* = nullptr */ )
{
    glm::mat4 modelMatrix( 1.0f ); // identity
    Use( modelMatrix, pShaderUniformInstances );
}

void Shader::Use( const glm::mat4& modelMatrix, ShaderUniformInstances* pShaderUniformInstances /* = nullptr */ )
{
    glUseProgram( m_ProgramHandle );

    UpdateParameters( modelMatrix, pShaderUniformInstances );
}

void Shader::UpdateParameters( const glm::mat4& modelMatrix, ShaderUniformInstances* pShaderUniformInstances )
{
    RenderSystem* renderSystem = FrameWork::GetRenderSystem();

    if ( m_pModelViewProjectionUniform != nullptr )
    {
        const glm::mat4 mvp = renderSystem->GetProjectionMatrix() * renderSystem->GetViewMatrix() * modelMatrix;
        m_pModelViewProjectionUniform->Set( mvp );
    }

    if ( m_pModelUniform != nullptr )
    {
        m_pModelUniform->Set( modelMatrix );
    }

    if ( m_pModelInverseUniform != nullptr )
    {
        glm::mat3x3 modelMatrix3( modelMatrix );
        glm::mat3x3 modelInverseMatrix = glm::inverse( modelMatrix3 );

        m_pModelInverseUniform->Set( glm::mat4( modelInverseMatrix ) /*glm::transpose( glm::inverse( modelMatrix ) )*/ );
    }

    if ( m_pModelInverseTransposeUniform != nullptr )
    {
        glm::mat3x3 modelMatrix3( renderSystem->GetViewMatrix() * modelMatrix );
        glm::mat3x3 normalMatrix = glm::transpose( glm::inverse( modelMatrix3 ) );

        m_pModelInverseTransposeUniform->Set( glm::mat4( normalMatrix ) /*glm::transpose( glm::inverse( modelMatrix ) )*/ );
    }

    if ( m_pViewInverseUniform != nullptr )
    {
        m_pViewInverseUniform->Set( glm::inverse( renderSystem->GetViewMatrix() ) );
    }

    if ( m_pTimeUniform != nullptr )
    {
        m_pTimeUniform->Set( FrameWork::GetRenderSystem()->GetShaderTimer() );
    }

    if ( m_pResolutionUniform != nullptr )
    {
        m_pResolutionUniform->Set( glm::vec2( (float)Configuration::GetScreenWidth(), (float)Configuration::GetScreenHeight() ) );
    }

    if ( pShaderUniformInstances != nullptr )
    {
        for ( auto& shaderUniformInstance : *pShaderUniformInstances )
        {
            shaderUniformInstance.Apply();
        }
    }

    for ( auto& pUniform : m_Uniforms )
    {
        pUniform->Apply();
    }
}

//---------------------------------------------------------------
// ShaderParameters

ShaderParameters::ShaderParameters()
    : mUsedTextureMaps( 0 )
{
    memset( mTextureMap, 0, 8 * sizeof( GLuint ) );
}

ShaderParameters::~ShaderParameters()
{
}

// Warning: if you set a texture map after the Shader was already created
// you'll have to call UpdateTextureParameters() on it.
void ShaderParameters::SetTextureMap( TextureMap textureMap, GLuint texture )
{
    mTextureMap[ static_cast<unsigned int>( textureMap ) ] = texture;

    mUsedTextureMaps |= ( 0x1 << static_cast<unsigned int>( textureMap ) );
}

//---------------------------------------------------------------
// Custom parameters

float2::float2()
    : f1( 0.0f )
    , f2( 0.0f )
{
}

float2::float2( float v1, float v2 )
    : f1( v1 )
    , f2( v2 )
{
}

float3::float3()
    : f1( 0.0f )
    , f2( 0.0f )
    , f3( 0.0f )
{
}

float3::float3( float v1, float v2, float v3 )
    : f1( v1 )
    , f2( v2 )
    , f3( v3 )
{
}

float4::float4()
    : f1( 0.0f )
    , f2( 0.0f )
    , f3( 0.0f )
    , f4( 0.0f )
{
}

float4::float4( float v1, float v2, float v3, float v4 )
    : f1( v1 )
    , f2( v2 )
    , f3( v3 )
    , f4( v4 )
{
}
}