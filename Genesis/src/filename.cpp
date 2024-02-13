#include <algorithm>

#include "filename.h"
#include "genesis.h"
#include "logger.h"

namespace Genesis
{

Filename::Filename( const char* pFilename )
{
    Initialise( pFilename );
}

Filename::Filename( const std::string& filename )
{
    Initialise( filename );
}

void Filename::Initialise( const std::string& filename )
{
    m_FullPath = ResolvePath( filename );

    const std::size_t l = m_FullPath.length();
    const std::size_t c = m_FullPath.find_last_of( "\\/" );
    m_Name = m_FullPath.substr( c + 1, l - 1 );
    m_Directory = m_FullPath.substr( 0, c + 1 );

    const std::size_t e = m_FullPath.find_last_of( "." );
    if ( e == std::string::npos )
    {
        Genesis::FrameWork::GetLogger()->LogError( "Don't know how to initialise filename '%s', it has no extension.", filename.c_str() );
    }
    else
    {
        m_Extension = filename.substr( filename.find_last_of( "." ) + 1, filename.length() );
        std::transform( m_Extension.begin(), m_Extension.end(), m_Extension.begin(), []( char c ) -> char { return static_cast<char>(std::tolower(c)); } );
    }
}

#ifdef _WIN32

std::string Filename::ResolvePath( const std::string& filename ) const
{
    std::string resolvedPath = filename;
    const size_t length = resolvedPath.length();
    for ( size_t i = 0; i < length; ++i )
    {
        if ( resolvedPath[ i ] == '/' )
            resolvedPath[ i ] = '\\';
    }

#if _MSC_VER
#pragma warning( suppress : 4127 ) // conditional expression is constant
#endif
    while ( 1 )
    {
        std::size_t up = resolvedPath.find( "..\\" );
        if ( up == std::string::npos )
        {
            break;
        }
        else if ( up == 0 ) // Edge case when someone is trying to escape out of the root
        {
            // Return the path without the ..
            resolvedPath = resolvedPath.substr( 3 );
        }
        else
        {
            std::size_t previousFolder = resolvedPath.find_last_of( "\\", up - 2 );
            if ( previousFolder == std::string::npos )
            {
                resolvedPath = resolvedPath.substr( up + 2 );
            }
            else
            {
                std::string bp = resolvedPath.substr( 0, previousFolder );
                std::string ap = resolvedPath.substr( up + 2 );
                resolvedPath = bp + ap;
            }
        }
    }

    return resolvedPath;
}

#else

std::string Filename::ResolvePath( const std::string& filename ) const
{
    std::string resolvedPath = filename;
    const unsigned int length = resolvedPath.length();
    for ( unsigned int i = 0; i < length; ++i )
    {
        if ( resolvedPath[ i ] == '\\' )
            resolvedPath[ i ] = '/';
    }

    while ( 1 )
    {
        std::size_t up = resolvedPath.find( "../" );
        if ( up == std::string::npos )
        {
            break;
        }
        else if ( up == 0 ) // Edge case when someone is trying to escape out of the root
        {
            // Return the path without the ..
            resolvedPath = resolvedPath.substr( 3 );
        }
        else
        {
            std::size_t previousFolder = resolvedPath.find_last_of( "/", up - 2 );
            if ( previousFolder == std::string::npos )
            {
                resolvedPath = resolvedPath.substr( up + 2 );
            }
            else
            {
                std::string bp = resolvedPath.substr( 0, previousFolder );
                std::string ap = resolvedPath.substr( up + 2 );
                resolvedPath = bp + ap;
            }
        }
    }

    return resolvedPath;
}

#endif // _WIN32

}