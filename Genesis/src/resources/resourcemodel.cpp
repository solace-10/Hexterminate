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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include "resourcemodel.h"
#include "../genesis.h"
#include "../logger.h"
#include "../rendersystem.h"
#include "../shadercache.h"
#include "../shaderuniform.h"

namespace Genesis
{

///////////////////////////////////////////////////////
// TMFObject
///////////////////////////////////////////////////////

TMFObject::TMFObject()
    : m_NumVertices( 0 )
    , m_NumUVs( 0 )
    , m_NumTriangles( 0 )
    , m_pVertexBuffer( nullptr )
    , m_MaterialIndex( 0u )
{
}

TMFObject::~TMFObject()
{
    delete m_pVertexBuffer;
}

// Preload is not called on the main thread, so it can't operate on OpenGL directly.
// We can load the TMFObject prepare the data for the vertex buffer, but the actual
// create of the VB / copy can only happen in Load().
void TMFObject::Preload( FILE* fp )
{
    Serialise( fp );
    BuildVertexBufferData();
}

void TMFObject::Load()
{
    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_NORMAL );
    m_pVertexBuffer->CopyPositions( m_VertexBufferPosData );
    m_pVertexBuffer->CopyNormals( m_VertexBufferNormalData );
    m_pVertexBuffer->CopyUVs( m_VertexBufferUvData );
}

bool TMFObject::Serialise( FILE* fp )
{
    // Read the object header
    Uint32 materialIndex;
    fread( &materialIndex, sizeof( Uint32 ), 1, fp );
    fread( &m_NumVertices, sizeof( Uint32 ), 1, fp );
    fread( &m_NumUVs, sizeof( Uint32 ), 1, fp );
    fread( &m_NumTriangles, sizeof( Uint32 ), 1, fp );

    // Allocate space for our data
    m_VertexList.reserve( m_NumVertices );
    m_UvList.reserve( m_NumUVs );
    m_TriangleList.reserve( m_NumTriangles );

    m_MaterialIndex = materialIndex - 1;

    // Read the vertexes
    glm::vec3 vertex;
    for ( uint32_t i = 0; i < m_NumVertices; i++ )
    {
        fread( &vertex.x, sizeof( float ), 3, fp );
        m_VertexList.push_back( vertex );
    }

    // Read the UVs
    glm::vec2 uv;
    for ( uint32_t i = 0; i < m_NumUVs; i++ )
    {
        fread( &uv, sizeof( float ), 2, fp );

        // WARNING:
        // SDL_Image and OpenGL interpret the UVs in different ways.
        // Therefore, SDL_Images are apparently loaded upside down.
        // So we invert the UV's here - it will appear correctly
        // when rendering.
        uv.y = 1.0f - uv.y;

        m_UvList.push_back( uv );
    }

    // Read the triangles and their normals
    Triangle triangle;
    for ( uint32_t i = 0; i < m_NumTriangles; i++ )
    {
        fread( &triangle.vertex, sizeof( uint32_t ), 3, fp );
        fread( &triangle.uv, sizeof( uint32_t ), 3, fp );
        fread( &triangle.normal[ 0 ], sizeof( float ), 9, fp );
        m_TriangleList.push_back( triangle );
    }

    return true;
}

void TMFObject::BuildVertexBufferData()
{
    const unsigned int numVertices = m_NumTriangles * 3;
    m_VertexBufferPosData.reserve( numVertices );
    m_VertexBufferNormalData.reserve( numVertices );
    m_VertexBufferUvData.reserve( numVertices );

    for ( Uint32 i = 0; i < m_NumTriangles; i++ )
    {
        m_VertexBufferPosData.push_back( m_VertexList[ m_TriangleList[ i ].vertex.v1 ] );
        m_VertexBufferPosData.push_back( m_VertexList[ m_TriangleList[ i ].vertex.v2 ] );
        m_VertexBufferPosData.push_back( m_VertexList[ m_TriangleList[ i ].vertex.v3 ] );

        m_VertexBufferNormalData.push_back( m_TriangleList[ i ].normal[ 0 ] );
        m_VertexBufferNormalData.push_back( m_TriangleList[ i ].normal[ 1 ] );
        m_VertexBufferNormalData.push_back( m_TriangleList[ i ].normal[ 2 ] );

        m_VertexBufferUvData.push_back( m_UvList[ m_TriangleList[ i ].uv.v1 ] );
        m_VertexBufferUvData.push_back( m_UvList[ m_TriangleList[ i ].uv.v2 ] );
        m_VertexBufferUvData.push_back( m_UvList[ m_TriangleList[ i ].uv.v3 ] );
    }
}

void TMFObject::Render( const glm::mat4& modelTransform, const MaterialList& materials )
{
    Material* pMaterial = materials[ m_MaterialIndex ];
    pMaterial->shader->Use( modelTransform, &pMaterial->uniforms );
    m_pVertexBuffer->Draw();
}

void TMFObject::Render( const glm::mat4& modelTransform, Material* pOverrideMaterial )
{
	pOverrideMaterial->shader->Use( modelTransform, &pOverrideMaterial->uniforms );
	m_pVertexBuffer->Draw();
}


///////////////////////////////////////////////////////
// ResourceModel
///////////////////////////////////////////////////////

ResourceModel::ResourceModel( const Filename& filename )
    : ResourceGeneric( filename )
    , mFlipAxis( true )
{
}

ResourceModel::~ResourceModel()
{
    for ( auto& pObject : mObjectList )
    {
        delete pObject;
    }

    for ( auto& pMaterial : mMaterialList )
    {
        delete pMaterial;
    }
}

void ResourceModel::Preload()
{
    struct TMFHEADER
    {
        uint8_t id[ 4 ]; // should be TMF
        uint16_t version; // version, multiplied by 100
        uint16_t nobj; // number of geometry objects
        uint16_t nhelpers; // number of helper objects
    } TMFHeader;
    memset( &TMFHeader, 0, sizeof( TMFHEADER ) );

    FILE* fp = 0;
#ifdef _WIN32
    fopen_s( &fp, GetFilename().GetFullPath().c_str(), "rb" );
#else
    fp = fopen( GetFilename().GetFullPath().c_str(), "rb" );
#endif
    if ( fp == nullptr )
    {
        return;
    }

    // Check if it is a TMF file
    fread( TMFHeader.id, sizeof( uint8_t ), 3, fp );
    if ( TMFHeader.id[ 0 ] != 'T' || TMFHeader.id[ 1 ] != 'M' || TMFHeader.id[ 2 ] != 'F' )
    {
        return;
    }

    fread( &TMFHeader.version, sizeof( uint16_t ), 1, fp );

    SDL_assert( TMFHeader.version == MODEL_VERSION );

    fread( &TMFHeader.nobj, sizeof( uint16_t ), 1, fp );
    fread( &TMFHeader.nhelpers, sizeof( uint16_t ), 1, fp );

    for ( int i = 0; i < TMFHeader.nhelpers; i++ )
    {
        AddTMFDummy( fp );
    }

    for ( int i = 0; i < TMFHeader.nobj; i++ )
    {
        AddTMFObject( fp );
    }

    fclose( fp );
}

bool ResourceModel::Load()
{
    LoadMaterialLibrary( GetFilename().GetFullPath() );

    for ( auto& pObject : mObjectList )
    {
        pObject->Load();
    }

    FrameWork::GetLogger()->LogInfo( "Model '%s': %d objects, %d materials", GetFilename().GetFullPath().c_str(), mObjectList.size(), mMaterialList.size() );
    m_State = ResourceState::Loaded;
    return true;
}

bool ResourceModel::GetDummy( const std::string& name, glm::vec3* pPosition ) const
{
    std::string nameLowerCase( name );
	std::transform( nameLowerCase.begin(), nameLowerCase.end(), nameLowerCase.begin(), []( char c ) -> char { return static_cast<char>(std::tolower(c)); } );

    DummyMap::const_iterator it = mDummyMap.find( nameLowerCase );
    if ( it == mDummyMap.end() )
	{
        return false;
	}
	else
	{
		*pPosition = it->second;
		return true;
	}
}

void ResourceModel::AddTMFDummy( FILE* fp )
{
    int len;
    fread( &len, sizeof( uint32_t ), 1, fp );
    char* pBuffer = new char[ len ];
    fread( pBuffer, sizeof( char ), len, fp );

    glm::vec3 pos;
    fread( &pos.x, sizeof( float ), 1, fp );
    fread( &pos.y, sizeof( float ), 1, fp );
    fread( &pos.z, sizeof( float ), 1, fp );

    std::string dummyName( pBuffer );
	std::transform( dummyName.begin(), dummyName.end(), dummyName.begin(), []( char c ) -> char { return static_cast<char>(std::tolower(c)); } );

    mDummyMap[ dummyName ] = pos;

    delete[] pBuffer;
}

void ResourceModel::AddTMFObject( FILE* fp )
{
    TMFObject* pObject = new TMFObject();
    pObject->Preload( fp );
    mObjectList.push_back( pObject );
}

// You can optionally pass an override material to be used instead of the normal materials this model would use. 
void ResourceModel::Render( const glm::mat4& modelTransform, Material* pOverrideMaterial /* = nullptr */ )
{
	if ( pOverrideMaterial == nullptr )
	{
		for ( auto& pObject : mObjectList )
		{
			pObject->Render( modelTransform, GetMaterials() );
		}
	}
	else
	{
		for ( auto& pObject : mObjectList )
		{
			pObject->Render( modelTransform, pOverrideMaterial );
		}
	}
}

void ResourceModel::LoadMaterialLibrary( const std::string& filename )
{
    Logger* pLogger = FrameWork::GetLogger();

    std::string materialFilename = filename;
    std::string path = filename.substr( 0, filename.find_last_of( "\\/" ) + 1 ); // We keep the final dash
    // Replace the last character of the extension - tmf becomes tml
    materialFilename[ filename.size() - 1 ] = 'l';

    std::ifstream fp;
    fp.open( materialFilename.c_str(), std::ios::in );

    if ( !fp.is_open() )
    {
        pLogger->LogError( "Couldn't load material library for %s", filename.c_str() );
        return;
    }

    Material* currentMaterial = nullptr;

    char buffer[ 256 ];
    char shaderName[ 64 ];
    char parameterType[ 16 ];
    char parameterName[ 64 ];
    char parameterValue[ 64 ];
    memset( shaderName, 0, sizeof( shaderName ) );
    memset( parameterType, 0, sizeof( parameterType ) );
    memset( parameterName, 0, sizeof( parameterName ) );
    memset( parameterValue, 0, sizeof( parameterValue ) );
    int numTextureMap = 0;
    while ( fp.getline( buffer, sizeof( buffer ) ) )
    {
#ifdef _WIN32
        sscanf_s( buffer, "  %s", parameterType, static_cast<uint32_t>(sizeof( parameterType ) ));
#else
        sscanf( buffer, "  %s", parameterType );
#endif
        std::string sparameterType( parameterType );

        if ( sparameterType == "INT" )
        {
            int v = 0;
#ifdef _WIN32
            sscanf_s( buffer, "%*s %s %d", parameterName, static_cast<uint32_t>(sizeof( parameterName )), &v );
#else
            sscanf( buffer, "%*s %s %d", parameterName, &v );
#endif

            ShaderUniform* pShaderUniform = currentMaterial->shader->RegisterUniform( parameterName, ShaderUniformType::Integer );
            if ( pShaderUniform == nullptr )
            {
                pLogger->LogWarning( "Model '%s', shader '%s', couldn't find uniform ShaderUniformType::Integer named '%s'", filename.c_str(), shaderName, parameterName );
            }
            else
            {
                ShaderUniformInstance instance( pShaderUniform );
                instance.Set( v );
                currentMaterial->uniforms.push_back( instance );
            }
        }
        else if ( sparameterType == "FLOAT" )
        {
            float v = 0.0f;
#ifdef _WIN32
            sscanf_s( buffer, "%*s %s %f", parameterName, static_cast<uint32_t>(sizeof( parameterName )), &v );
#else
            sscanf( buffer, "%*s %s %f", parameterName, &v );
#endif

            ShaderUniform* pShaderUniform = currentMaterial->shader->RegisterUniform( parameterName, ShaderUniformType::Float );
            if ( pShaderUniform == nullptr )
            {
                pLogger->LogWarning( "Model '%s', shader '%s', couldn't find uniform ShaderUniformType::Float named '%s'", filename.c_str(), shaderName, parameterName );
            }
            else
            {
                ShaderUniformInstance instance( pShaderUniform );
                instance.Set( v );
                currentMaterial->uniforms.push_back( instance );
            }
        }
        else if ( sparameterType == "FLOAT2" )
        {
            float v1, v2;
#ifdef _WIN32
            sscanf_s( buffer, "%*s %s %f %f", parameterName, static_cast<uint32_t>(sizeof( parameterName )), &v1, &v2 );
#else
            sscanf( buffer, "%*s %s %f %f", parameterName, &v1, &v2 );
#endif

            ShaderUniform* pShaderUniform = currentMaterial->shader->RegisterUniform( parameterName, ShaderUniformType::FloatVector2 );
            if ( pShaderUniform == nullptr )
            {
                pLogger->LogWarning( "Model '%s', shader '%s', couldn't find uniform ShaderUniformType::FloatVector2 named '%s'", filename.c_str(), shaderName, parameterName );
            }
            else
            {
                ShaderUniformInstance instance( pShaderUniform );
                instance.Set( glm::vec2( v1, v2 ) );
                currentMaterial->uniforms.push_back( instance );
            }
        }
        else if ( sparameterType == "FLOAT3" )
        {
            float v1, v2, v3;
#ifdef _WIN32
            sscanf_s( buffer, "%*s %s %f %f %f", parameterName, static_cast<uint32_t>(sizeof( parameterName )), &v1, &v2, &v3 );
#else
            sscanf( buffer, "%*s %s %f %f %f", parameterName, &v1, &v2, &v3 );
#endif

            ShaderUniform* pShaderUniform = currentMaterial->shader->RegisterUniform( parameterName, ShaderUniformType::FloatVector3 );
            if ( pShaderUniform == nullptr )
            {
                pLogger->LogWarning( "Model '%s', shader '%s', couldn't find uniform ShaderUniformType::Floatglm::vec3 named '%s'", filename.c_str(), shaderName, parameterName );
            }
            else
            {
                ShaderUniformInstance instance( pShaderUniform );
                instance.Set( glm::vec3( v1, v2, v3 ) );
                currentMaterial->uniforms.push_back( instance );
            }
        }
        else if ( sparameterType == "FLOAT4" )
        {
            float v1, v2, v3, v4;
#ifdef _WIN32
            sscanf_s( buffer, "%*s %s %f %f %f %f", parameterName, static_cast<uint32_t>(sizeof( parameterName )), &v1, &v2, &v3, &v4 );
#else
            sscanf( buffer, "%*s %s %f %f %f %f", parameterName, &v1, &v2, &v3, &v4 );
#endif

            ShaderUniform* pShaderUniform = currentMaterial->shader->RegisterUniform( parameterName, ShaderUniformType::FloatVector4 );
            if ( pShaderUniform == nullptr )
            {
                pLogger->LogWarning( "Model '%s', shader '%s', couldn't find uniform ShaderUniformType::FloatVector4 named '%s'", filename.c_str(), shaderName, parameterName );
            }
            else
            {
                ShaderUniformInstance instance( pShaderUniform );
                instance.Set( glm::vec4( v1, v2, v3, v4 ) );
                currentMaterial->uniforms.push_back( instance );
            }
        }
        else if ( sparameterType == "TEXTUREMAP" )
        {
#ifdef _WIN32
            sscanf_s( buffer, "%*s %s", parameterName, static_cast<uint32_t>(sizeof( parameterName )) );
#else
            sscanf( buffer, "%*s %s", parameterName );
#endif
            std::string textureFileName( path );
            textureFileName += parameterName;
            ResourceImage* pImage = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( textureFileName );

            std::stringstream uniformName;
            uniformName << "k_sampler" << numTextureMap;

            ShaderUniform* pShaderUniform = currentMaterial->shader->RegisterUniform( uniformName.str().c_str(), ShaderUniformType::Texture );
            if ( pShaderUniform == nullptr )
            {
                pLogger->LogWarning( "Model '%s', shader '%s', couldn't find uniform ShaderUniformType::Texture named '%s'", filename.c_str(), shaderName, uniformName.str().c_str() );
            }
            else
            {
                ShaderUniformInstance instance( pShaderUniform );
                instance.Set( pImage, GL_TEXTURE0 + numTextureMap );
                currentMaterial->uniforms.push_back( instance );
                currentMaterial->resources[ numTextureMap ] = pImage;
            }

            numTextureMap++;
        }
        else if ( sparameterType == "SHADER" )
        {
#ifdef _WIN32
            sscanf_s( buffer, "%*s %s", shaderName, static_cast<uint32_t>(sizeof( parameterName )) );
#else
            sscanf( buffer, "%*s %s", shaderName );
#endif

            numTextureMap = 0;

            std::string shaderNameNoExtension( shaderName );
            shaderNameNoExtension = shaderNameNoExtension.substr( 0, shaderNameNoExtension.find_last_of( '.' ) );

            currentMaterial = new Material();
            currentMaterial->shader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( shaderNameNoExtension );
            currentMaterial->name = shaderNameNoExtension;
            currentMaterial->resources.fill( nullptr );

            mMaterialList.push_back( currentMaterial );
        }
    }

    fp.close();
}
}