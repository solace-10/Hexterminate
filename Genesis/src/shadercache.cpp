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

#include "shadercache.h"
#include "genesis.h"
#include "rendersystem.h"
#include "shader.h"

namespace Genesis
{

///////////////////////////////////////////////////////////////////////////////
// ShaderCache
///////////////////////////////////////////////////////////////////////////////

ShaderCache::ShaderCache()
{
}

ShaderCache::~ShaderCache()
{
    for ( auto& shaderPair : m_ProgramCache )
    {
        delete shaderPair.second;
    }
}

Shader* ShaderCache::Load( const std::string& programName )
{
    Logger* pLog = FrameWork::GetLogger();

    ShaderMap::const_iterator it = m_ProgramCache.find( programName );
    if ( it != m_ProgramCache.cend() )
    {
        return it->second;
    }

    // Create the shaders
    GLuint vertexShaderID = glCreateShader( GL_VERTEX_SHADER );
    GLuint fragmentShaderID = glCreateShader( GL_FRAGMENT_SHADER );

    // Read the Vertex Shader code from the file
    std::string vertexFilename = "data/shaders/" + programName + ".vert";
    std::string vertexShaderCode;
    std::ifstream vertexShaderStream( vertexFilename, std::ios::in );

    if ( vertexShaderStream.is_open() )
    {
        std::string line = "";
        while ( getline( vertexShaderStream, line ) )
        {
            vertexShaderCode += "\n" + line;
        }

        vertexShaderStream.close();
    }
    else
    {
        pLog->LogError( "Couldn't open '%s'.", vertexFilename.c_str() );
        return nullptr;
    }

    // Read the Fragment Shader code from the file
    std::string fragmentFilename = "data/shaders/" + programName + ".frag";
    std::string fragmentShaderCode;
    std::ifstream fragmentShaderStream( fragmentFilename, std::ios::in );

    if ( fragmentShaderStream.is_open() )
    {
        std::string line = "";
        while ( getline( fragmentShaderStream, line ) )
        {
            fragmentShaderCode += "\n" + line;
        }

        fragmentShaderStream.close();
    }

    pLog->LogInfo( "Compiling shader program: %s", programName.c_str() );

    GLint compilationSuccessful = GL_FALSE;
    int infoLogLength = 0;

    // Compile Vertex Shader
    const char* vertexSourcePointer = vertexShaderCode.c_str();
    glShaderSource( vertexShaderID, 1, &vertexSourcePointer, nullptr );
    glCompileShader( vertexShaderID );

    // Check Vertex Shader
    glGetShaderiv( vertexShaderID, GL_COMPILE_STATUS, &compilationSuccessful );
    if ( compilationSuccessful == GL_FALSE )
    {
        glGetShaderiv( vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength );
        std::vector<char> vertexShaderErrorMessage( infoLogLength + 1 );
        glGetShaderInfoLog( vertexShaderID, infoLogLength, nullptr, &vertexShaderErrorMessage[ 0 ] );
        pLog->LogError( "Compiling vertex shader '%s':\n%s", programName.c_str(), &vertexShaderErrorMessage[ 0 ] );
    }

    // Compile Fragment Shader
    const char* fragmentSourcePointer = fragmentShaderCode.c_str();
    glShaderSource( fragmentShaderID, 1, &fragmentSourcePointer, nullptr );
    glCompileShader( fragmentShaderID );

    // Check Fragment Shader
    glGetShaderiv( fragmentShaderID, GL_COMPILE_STATUS, &compilationSuccessful );
    if ( compilationSuccessful == GL_FALSE )
    {
        glGetShaderiv( fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength );
        std::vector<char> fragmentShaderErrorMessage( infoLogLength + 1 );
        glGetShaderInfoLog( fragmentShaderID, infoLogLength, nullptr, &fragmentShaderErrorMessage[ 0 ] );
        pLog->LogError( "Compiling fragment shader '%s':\n%s", programName.c_str(), &fragmentShaderErrorMessage[ 0 ] );
    }

    // Link the program
    GLuint programHandle = glCreateProgram();
    glAttachShader( programHandle, vertexShaderID );
    glAttachShader( programHandle, fragmentShaderID );
    glLinkProgram( programHandle );

    // Check the program
    GLint linkSuccessful = GL_FALSE;
    glGetProgramiv( programHandle, GL_LINK_STATUS, &linkSuccessful );
    if ( linkSuccessful == GL_FALSE )
    {
        glGetProgramiv( programHandle, GL_INFO_LOG_LENGTH, &infoLogLength );
        std::vector<char> programErrorMessage( infoLogLength + 1 );
        glGetProgramInfoLog( programHandle, infoLogLength, nullptr, &programErrorMessage[ 0 ] );
        pLog->LogError( "%s", &programErrorMessage[ 0 ] );
    }

    glDetachShader( programHandle, vertexShaderID );
    glDetachShader( programHandle, fragmentShaderID );

    glDeleteShader( vertexShaderID );
    glDeleteShader( fragmentShaderID );

    Shader* pShader = new Shader( programName, programHandle );
    m_ProgramCache[ programName ] = pShader;

    pLog->LogInfo( "Cached shader program '%s'", programName.c_str() );

    return pShader;
}

}
