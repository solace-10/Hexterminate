// Copyright 2017 Pedro Nunes
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

#ifdef _WIN32
#include "windows.h"
#endif

#include "logger.h"
#include "memory.h"

namespace Genesis
{

static const char* LogMessageText[ LOG_MAX ] = {
    "INFO: ",
    "WARNING: ",
    "ERROR: "
};

//---------------------------------------------------------------
// Logger
//---------------------------------------------------------------

Logger::Logger()
{
    m_pMutex = SDL_CreateMutex();
}

Logger::~Logger()
{
    SDL_DestroyMutex( m_pMutex );
    m_pMutex = nullptr;

    for ( auto& pTarget : m_Targets )
    {
        delete pTarget;
    }
}

void Logger::AddLogTarget( LogTarget* pLogTarget )
{
    if ( pLogTarget != nullptr )
    {
        m_Targets.push_back( pLogTarget );
    }
}

void Logger::RemoveLogTarget( LogTarget* pLogTarget )
{
    if ( pLogTarget == nullptr )
    {
        return;
    }

    LogTargetList::iterator it = m_Targets.begin();
    LogTargetList::iterator itEnd = m_Targets.end();
    while ( it != itEnd )
    {
        if ( *it == pLogTarget )
        {
            delete *it;
            m_Targets.erase( it );
            break;
        }

        it++;
    }
}

// Internal logging function. Should be called by one of the public functions (LogInfo / LogWarning / LogError).
// Assumes that m_pMutex is locked at this stage.
void Logger::Log( const char* text, LogMessageType type /* = LOG_INFO */ )
{
#ifdef _WIN32
    sprintf_s( m_Buffer.data(), LOG_BUFFER_SIZE, "%s%s\n", LogMessageText[ type ], text );
#else
    snprintf( m_Buffer.data(), LOG_BUFFER_SIZE, "%s%s\n", LogMessageText[ type ], text );
#endif

    for ( auto& pTarget : m_Targets )
    {
		pTarget->Log( m_Buffer.data(), type );
    }

    if ( type == LOG_ERROR )
    {
#ifdef _WIN32
        __debugbreak();
#else
        asm( "int $3" );
#endif
        exit( -1 );
    }
}

void Logger::LogInfo( const char* format, ... )
{
    SDL_LockMutex( m_pMutex );

    va_list args;
    va_start( args, format );
#ifdef _WIN32
	vsprintf_s( m_VABuffer.data(), LOG_BUFFER_SIZE, format, args );
#else
    vsnprintf( m_VABuffer.data(), LOG_BUFFER_SIZE, format, args );
#endif
    Log( m_VABuffer.data(), LOG_INFO );
    va_end( args );

    SDL_UnlockMutex( m_pMutex );
}

void Logger::LogWarning( const char* format, ... )
{
    SDL_LockMutex( m_pMutex );

    va_list args;
    va_start( args, format );
#ifdef _WIN32
    vsprintf_s( m_VABuffer.data(), LOG_BUFFER_SIZE, format, args );
#else
    vsnprintf( m_VABuffer.data(), LOG_BUFFER_SIZE, format, args );
#endif
    Log( m_VABuffer.data(), LOG_WARNING );
    va_end( args );

    SDL_UnlockMutex( m_pMutex );
}

void Logger::LogError( const char* format, ... )
{
    SDL_LockMutex( m_pMutex );

    va_list args;
    va_start( args, format );
#ifdef _WIN32
    vsprintf_s( m_VABuffer.data(), LOG_BUFFER_SIZE, format, args );
#else
    vsnprintf( m_VABuffer.data(), LOG_BUFFER_SIZE, format, args );
#endif
    Log( m_VABuffer.data(), LOG_ERROR );
    va_end( args );

    SDL_UnlockMutex( m_pMutex );
}

//---------------------------------------------------------------
// FileLogger
//---------------------------------------------------------------

FileLogger::FileLogger( const char* pFilename )
{
    m_File.open( pFilename, std::fstream::out | std::fstream::trunc );
}

FileLogger::~FileLogger()
{
    if ( m_File.is_open() )
    {
        m_File.close();
    }
}

void FileLogger::Log( const char* pText, LogMessageType type )
{
    if ( !m_File.is_open() )
    {
        return;
    }

    m_File.write( pText, strlen( pText ) );
    m_File.flush();
}

//---------------------------------------------------------------
// MessageBoxLogger
//---------------------------------------------------------------

void MessageBoxLogger::Log( const char* pText, LogMessageType type )
{
    if ( type == LOG_WARNING )
    {
        SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_WARNING, "Warning", pText, nullptr );
    }
    else if ( type == LOG_ERROR )
    {
        SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Error", pText, nullptr );
    }
}

//---------------------------------------------------------------
// VisualStudioLogger
//---------------------------------------------------------------

#ifdef _WIN32
void VisualStudioLogger::Log( const char* pText, LogMessageType type )
{
    OutputDebugStringA( pText );
}
#endif
}
