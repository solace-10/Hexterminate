// Copyright 2014 Pedro Nunes
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

#include <map>
#include <vector>

#include <glm/gtx/transform.hpp>

#include "genesis.h"
#include "logger.h"
#include "rendersystem.fwd.h"
#include "memory.h"
#include "shaderuniformtype.h"

namespace Genesis
{

enum class TextureMap
{
    TEXTURE_MAP_0 = 0,
    TEXTURE_MAP_1,
    TEXTURE_MAP_2,
    TEXTURE_MAP_3,
    TEXTURE_MAP_4,
    TEXTURE_MAP_5,
    TEXTURE_MAP_6,
    TEXTURE_MAP_7,
    TEXTURE_MAP_MAX,
};

class ShaderParameters;
class ShaderUniform;
class ShaderUniformInstance;

typedef std::vector<ShaderUniform*> ShaderUniforms;
typedef std::vector<ShaderUniformInstance> ShaderUniformInstances;

class Shader
{
public:
    Shader( const std::string& programName, GLuint programHandle );
    ~Shader();

    const std::string& GetName() const;

    void Use( ShaderUniformInstances* pShaderUniformInstances = nullptr );
    void Use( const glm::mat4& modelTransform, ShaderUniformInstances* pShaderUniformInstances = nullptr );

    ShaderUniform* RegisterUniform( const char* pUniformName, ShaderUniformType type, bool allowInstancingOverride = true );

private:
    void RegisterCoreUniforms();

    void BindTextureMap( TextureMap textureMap, GLuint texture );
    void UpdateParameters( const glm::mat4& modelMatrix, ShaderUniformInstances* pShaderUniformInstances );

    std::string m_ShaderName;

    GLuint m_ProgramHandle;
    ShaderUniform* m_pModelViewProjectionUniform;
    ShaderUniform* m_pModelUniform;
    ShaderUniform* m_pModelInverseUniform;
    ShaderUniform* m_pModelInverseTransposeUniform;
    ShaderUniform* m_pViewInverseUniform;
    ShaderUniform* m_pTimeUniform;
    ShaderUniform* m_pResolutionUniform;

    ShaderUniforms m_Uniforms;
};

inline const std::string& Shader::GetName() const
{
    return m_ShaderName;
}

enum class ParameterType
{
    PARAMETER_TYPE_INT = 0,
    PARAMETER_TYPE_FLOAT,
    PARAMETER_TYPE_FLOAT2,
    PARAMETER_TYPE_FLOAT3,
    PARAMETER_TYPE_FLOAT4,
};

class ShaderParameters
{
public:
    ShaderParameters();
    ~ShaderParameters();
    template <typename T>
    void SetParameter( const std::string& name, ParameterType type, T value );

    const std::string& GetParameterName( int id ) const;
    ParameterType GetParameterType( int id ) const;
    template <typename T>
    void GetParameterValue( int id, T* value ) const;

    int GetParametersNumber() const;

    void SetTextureMap( TextureMap textureMap, GLuint texture );
    int GetUsedTextureMaps() const;
    GLuint GetTextureMap( TextureMap textureMap ) const;

private:
    struct Parameter
    {
        std::string name;
        ParameterType type;
        void* value;
    };

    typedef std::vector<Parameter*> ParameterGroup;
    ParameterGroup mParameters;
    GLuint mTextureMap[ static_cast<unsigned int>( TextureMap::TEXTURE_MAP_MAX ) ];
    int mUsedTextureMaps;
};

inline int ShaderParameters::GetParametersNumber() const
{
    return static_cast<int>(mParameters.size());
}

inline const std::string& ShaderParameters::GetParameterName( int id ) const
{
    return mParameters[ id ]->name;
}

inline ParameterType ShaderParameters::GetParameterType( int id ) const
{
    return mParameters[ id ]->type;
}

template <typename T>
void ShaderParameters::GetParameterValue( int id, T* value ) const
{
    T* v = (T*)( mParameters[ id ]->value );
    *value = *v;
}

inline int ShaderParameters::GetUsedTextureMaps() const
{
    return mUsedTextureMaps;
}

inline GLuint ShaderParameters::GetTextureMap( TextureMap textureMap ) const
{
    return mTextureMap[ static_cast<unsigned int>( textureMap ) ];
}

template <typename T>
void ShaderParameters::SetParameter( const std::string& name, ParameterType type, T value )
{
    Parameter* parameter = 0;

    // See if the parameter already exists
    ParameterGroup::iterator it = mParameters.begin();
    ParameterGroup::iterator itEnd = mParameters.end();
    for ( ; it != itEnd; it++ )
    {
        if ( ( *it )->name == name )
        {
            if ( ( *it )->type != type )
            {
                FrameWork::GetLogger()->LogError( "Trying to set shader parameter to a different type than it was initialized with." );
            }
            parameter = ( *it );
            break;
        }
    }

    // If it doesn't create a new parameter
    if ( !parameter )
    {
        parameter = new Parameter();
        parameter->name = name;
        parameter->type = type;
        parameter->value = new T();
        mParameters.push_back( parameter );
    }
    *( (T*)( parameter->value ) ) = value;
}

//---------------------------------------------------------------
// Custom parameters

class float2
{
public:
    float2();
    float2( float v1, float v2 );
    float f1;
    float f2;
};

class float3
{
public:
    float3();
    float3( float v1, float v2, float v3 );
    float f1;
    float f2;
    float f3;
};

class float4
{
public:
    float4();
    float4( float v1, float v2, float v3, float v4 );
    float f1;
    float f2;
    float f3;
    float f4;
};
}
