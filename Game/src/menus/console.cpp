#include <configuration.h>
#include <genesis.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl.h>
#include <logger.h>
#include <memory.h>
#include <timer.h>

#include "hexterminate.h"
#include "menus/console.h"

namespace Hexterminate
{

Console::Console()
    : m_Open( false )
    , m_pLogTarget( nullptr )
    , m_pTextMutex( nullptr )
{
    m_Text = "";

    m_pTextMutex = SDL_CreateMutex();

    m_pLogTarget = new ConsoleLogTarget( this );
    Genesis::FrameWork::GetLogger()->AddLogTarget( m_pLogTarget );

    Genesis::ImGuiImpl::RegisterMenu( "Tools", "Console", &m_Open );
}

Console::~Console()
{
    Genesis::FrameWork::GetLogger()->RemoveLogTarget( m_pLogTarget );
    SDL_DestroyMutex( m_pTextMutex );
}

bool Console::IsVisible() const
{
    return Genesis::ImGuiImpl::IsEnabled() && m_Open;
}

void Console::AddLine( const char* text )
{
    SDL_LockMutex( m_pTextMutex );

    m_Text += text;

    size_t chars = m_Text.size();
    int lines = 0;
    int maxLines = 25;
    for ( size_t i = 0; i < chars; ++i )
    {
        if ( m_Text[ i ] == '\n' )
            ++lines;
    }

    if ( lines > maxLines )
    {
        int linesToDelete = lines - maxLines;
        size_t charPos = 0;
        for ( ; charPos < chars; ++charPos )
        {
            if ( m_Text[ charPos ] == '\n' && --linesToDelete == 0 )
                break;
        }
        m_Text = m_Text.substr( charPos + 1, chars );
    }

    SDL_UnlockMutex( m_pTextMutex );
}

void Console::Update( float delta )
{
    if ( m_Open )
    {
        SDL_LockMutex( m_pTextMutex );

        ImGui::SetNextWindowSize( ImVec2( 500.0f, 500.0f ) );
        ImGui::Begin( "Console", &m_Open );
        ImGui::TextWrapped( "%s", m_Text.c_str() );
        ImGui::End();

        SDL_UnlockMutex( m_pTextMutex );
    }
}

void Console::Show( bool state )
{
}

ConsoleLogTarget::ConsoleLogTarget( Console* pConsole )
    : m_pConsole( pConsole )
{
}

void ConsoleLogTarget::Log( const char* text, Genesis::LogMessageType type /* = Genesis::LOG_INFO */ )
{
    m_pConsole->AddLine( text );
}

} // namespace Hexterminate