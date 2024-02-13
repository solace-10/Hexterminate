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

#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <list>
#include <stdarg.h>

#include <SDL.h>

#define GENESIS_LOG( x ) Genesis::FrameWork::GetLogger()->LogInfo( x )

namespace Genesis
{

#define LOG_BUFFER_SIZE 20480

class LogTarget;

enum LogMessageType
{
    LOG_INFO = 0,
    LOG_WARNING,
    LOG_ERROR,
    LOG_MAX, // unused, for translation table only
};

//////////////////////////////////////////////////////////////////////////
// Logger
// A Logger contains any number of LogTargets. When the Log***() functions
// are called, all the LogTargets are called as well.
//////////////////////////////////////////////////////////////////////////

typedef std::list<LogTarget*> LogTargetList;

class Logger
{
public:
    Logger();
    ~Logger();

    // Thread safe
    void LogInfo( const char* pFormat, ... );
    void LogWarning( const char* pFormat, ... );
    void LogError( const char* pFormat, ... );

    // Not thread safe. Call only from the main thread.
    void AddLogTarget( LogTarget* pLogTarget );
    void RemoveLogTarget( LogTarget* pLogTarget );

private:
    void Log( const char* pText, LogMessageType type = LOG_INFO );

    SDL_mutex* m_pMutex;
    LogTargetList m_Targets;
    std::array<char, LOG_BUFFER_SIZE> m_Buffer;
    std::array<char, LOG_BUFFER_SIZE> m_VABuffer;
};

//////////////////////////////////////////////////////////////////////////
// LogTarget interface. Any LogTarget must implement Log()
//////////////////////////////////////////////////////////////////////////

class LogTarget
{
public:
	virtual ~LogTarget() {}
    virtual void Log( const char* pText, LogMessageType type = LOG_INFO ) = 0;
};

//////////////////////////////////////////////////////////////////////////
// FileLogger
// Dumps the logging into file given in "filename". It is flushed
// after every entry.
//////////////////////////////////////////////////////////////////////////

class FileLogger : public LogTarget
{
public:
    FileLogger( const char* pFilename );
    ~FileLogger();
    virtual void Log( const char* pText, LogMessageType type );

private:
    std::ofstream m_File;
};

//////////////////////////////////////////////////////////////////////////
// MessageBoxLogger
// Creates a message box whenever the log message is above
// LOG_INFO
//////////////////////////////////////////////////////////////////////////

class MessageBoxLogger : public LogTarget
{
public:
    virtual void Log( const char* pText, LogMessageType type );
};

//////////////////////////////////////////////////////////////////////////
// VisualStudioLogger
// Windows only - all the output from the logger goes to the
// Visual Studio output window.
//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
class VisualStudioLogger : public LogTarget
{
public:
    virtual void Log( const char* pText, LogMessageType type );
};
#endif
}
