#pragma once

#include "eva.h"
#include <atomic>
#include <gui/gui.h>
#include <logger.h>

namespace Hexterminate
{

class ConsoleLogTarget;

class Console
{
public:
    Console();
    ~Console();
    void AddLine( const char* pText );
    void Update( float delta );
    void Show( bool state );
    bool IsVisible() const;

private:
    bool m_Open;
    ConsoleLogTarget* m_pLogTarget;
    std::string m_Text;
    SDL_mutex* m_pTextMutex;
};

class ConsoleLogTarget : public Genesis::LogTarget
{
public:
    ConsoleLogTarget( Console* pConsole );
    virtual ~ConsoleLogTarget(){};
    virtual void Log( const char* text, Genesis::LogMessageType type = Genesis::LOG_INFO );

private:
    Console* m_pConsole;
};

} // namespace Hexterminate
