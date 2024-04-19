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

#include <SDL_image.h>
#include <imgui/imgui.h>

#include "configuration.h"
#include "core/programoptions.h"
#include "crashhandler/crashhandler.h"
#include "eventhandler.h"
#include "genesis.h"
#include "gui/gui.h"
#include "imgui/imgui_impl.h"
#include "inputmanager.h"
#include "memory.h"
#include "render/debugrender.h"
#include "rendersystem.h"
#include "resourcemanager.h"
#include "scene/scene.h"
#include "sound/soundmanager.h"
#include "taskmanager.h"
#include "timer.h"
#include "videoplayer.h"
#include "window.h"

namespace Genesis
{

//---------------------------------------------------------------
// Globals
//---------------------------------------------------------------

std::unique_ptr<CrashHandler> gCrashHandler;
std::unique_ptr<ProgramOptions> gProgramOptions;
TaskManager* gTaskManager = nullptr;
Logger* gLogger = nullptr;
InputManager* gInputManager = nullptr;
EventHandler* gEventHandler = nullptr;
Window* gWindow = nullptr;
RenderSystem* gRenderSystem = nullptr;
ResourceManager* gResourceManager = nullptr;
Scene* gScene = nullptr;
Gui::GuiManager* gGuiManager = nullptr;
Sound::SoundManager* gSoundManager = nullptr;
VideoPlayer* gVideoPlayer = nullptr;
Render::DebugRender* gDebugRender = nullptr;

CommandLineParameters* FrameWork::m_pCommandLineParameters = nullptr;

//-------------------------------------------------------------------
// FrameWork
//-------------------------------------------------------------------

bool FrameWork::Initialize( int argc, char* argv[] )
{
    // Initialize the Logger
    // We also create a FileLogger to start logging to "log.txt" and
    // a MessageBoxLogger that creates a message box if the log is
    // a warning or an error.
    gLogger = new Logger();

    gLogger->AddLogTarget( new FileLogger( "log.txt" ) );
    gLogger->AddLogTarget( new MessageBoxLogger() );
#ifdef _WIN32
#ifdef _DEBUG
    gLogger->AddLogTarget( new VisualStudioLogger() );
#endif
#endif

    gCrashHandler = std::make_unique<CrashHandler>();
    if ( gCrashHandler->Initialize() )
    {
        gLogger->LogInfo( "Crash handler initialized." );
    }
    else
    {
        gLogger->LogWarning( "Failed to initialize crash handler." );
    }

    gProgramOptions = std::make_unique<ProgramOptions>( argc, argv );

    // Tell the OS we'll do our own scaling. Without this, the OS will resize the window by the scaling factor,
    // resulting in some extremely poor upsclaing and broken UI.
    SDL_SetHint( SDL_HINT_WINDOWS_DPI_SCALING, "1" );

    // Initialize SDL
    // Needs to be done before InputManager() is created,
    // otherwise key repetition won't work.
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS ) < 0 )
    {
        gLogger->LogError( "%s", SDL_GetError() );
    }
    else
    {
        gLogger->LogInfo( "SDL initialized." );
    }

    Configuration::Load();

    int flags = IMG_INIT_JPG | IMG_INIT_PNG;
    int imgResult = IMG_Init( flags );
    if ( ( imgResult & flags ) != flags )
    {
        gLogger->LogError( "IMG_Init failed: %s", IMG_GetError() );
    }
    else
    {
        gLogger->LogInfo( "SDL_image initialized with JPG and PNG support." );
    }

    gInputManager = new InputManager();
    gEventHandler = new EventHandler();
    gResourceManager = new ResourceManager();

    // Initialize the task manager, as well as all the related tasks
    gTaskManager = new TaskManager( gLogger );

    gTaskManager->AddTask( "InputManager", gInputManager, (TaskFunc)&InputManager::Update, TaskPriority::System );
    gTaskManager->AddTask( "EventHandler", gEventHandler, (TaskFunc)&EventHandler::Update, TaskPriority::System );

    gRenderSystem = new RenderSystem();
    gTaskManager->AddTask( "Render", gRenderSystem, (TaskFunc)&RenderSystem::Update, TaskPriority::Rendering );

    gGuiManager = new Gui::GuiManager();
    gTaskManager->AddTask( "GUIManager", gGuiManager, (TaskFunc)&Gui::GuiManager::Update, TaskPriority::GameLogic );

    gScene = new Scene();
    gTaskManager->AddTask( "Scene", gScene, (TaskFunc)&Scene::Update, TaskPriority::GameLogic );

    gSoundManager = new Sound::SoundManager();
    gTaskManager->AddTask( "SoundManager", gSoundManager, (TaskFunc)&Sound::SoundManager::Update, TaskPriority::System );

    return true;
}

void FrameWork::Shutdown()
{
    delete gDebugRender;
    gDebugRender = nullptr;

    delete gGuiManager;
    gGuiManager = nullptr;

    delete gVideoPlayer;
    gVideoPlayer = nullptr;

    delete gInputManager;
    gInputManager = nullptr;

    delete gEventHandler;
    gEventHandler = nullptr;

    delete gTaskManager;
    gTaskManager = nullptr;

    delete gRenderSystem;
    gRenderSystem = nullptr;

    delete gResourceManager;
    gResourceManager = nullptr;

    delete gSoundManager;
    gSoundManager = nullptr;

    delete gScene;
    gScene = nullptr;

    delete gWindow;
    gWindow = nullptr;

    delete gLogger;
    gLogger = nullptr;
}

bool FrameWork::CreateWindowGL( const std::string& name, uint32_t width, uint32_t height, uint32_t multiSampleSamples /* = 0 */ )
{
    // Set OpenGL version to 3.3.
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE ); // OpenGL core profile
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 ); // OpenGL 3+
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 ); // OpenGL 3.3

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );

    if ( multiSampleSamples > 0 )
    {
        SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
        SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, multiSampleSamples );
    }

    gWindow = new Window( name, width, height, Configuration::IsFullscreen() );

    GetRenderSystem()->Initialize( width, height );

    GetGuiManager()->Initialize();
    ImGuiImpl::Initialise();

    gVideoPlayer = new VideoPlayer();
    gTaskManager->AddTask( "VideoPlayer", gVideoPlayer, (TaskFunc)&VideoPlayer::Update, TaskPriority::System );

    gDebugRender = new Render::DebugRender();

    return true;
}

CommandLineParameters* FrameWork::CreateCommandLineParameters( const char* parameterStr )
{
    m_pCommandLineParameters = new CommandLineParameters( parameterStr );
    return m_pCommandLineParameters;
}

CommandLineParameters* FrameWork::CreateCommandLineParameters( const char** parameters, uint32_t numParameters )
{
    m_pCommandLineParameters = new CommandLineParameters( parameters, numParameters );
    return m_pCommandLineParameters;
}

CommandLineParameters* FrameWork::GetCommandLineParameters()
{
    return m_pCommandLineParameters;
}

Logger* FrameWork::GetLogger()
{
    return gLogger;
}

ProgramOptions* FrameWork::GetProgramOptions()
{
    return gProgramOptions.get();
}

TaskManager* FrameWork::GetTaskManager()
{
    return gTaskManager;
}

InputManager* FrameWork::GetInputManager()
{
    return gInputManager;
}

Window* FrameWork::GetWindow()
{
    return gWindow;
}

RenderSystem* FrameWork::GetRenderSystem()
{
    return gRenderSystem;
}

ResourceManager* FrameWork::GetResourceManager()
{
    return gResourceManager;
}

Scene* FrameWork::GetScene()
{
    return gScene;
}

Gui::GuiManager* FrameWork::GetGuiManager()
{
    return gGuiManager;
}

Sound::SoundManager* FrameWork::GetSoundManager()
{
    return gSoundManager;
}

VideoPlayer* FrameWork::GetVideoPlayer()
{
    return gVideoPlayer;
}

Render::DebugRender* FrameWork::GetDebugRender()
{
    return gDebugRender;
}

//---------------------------------------------------------------
// CommandLineParameters
//---------------------------------------------------------------

CommandLineParameters::CommandLineParameters( const char* parameterStr )
{
    // Do we even have any parameters?
    if ( parameterStr != nullptr )
    {
        std::string tmpStr( parameterStr );

        size_t previousPos = 0;
        size_t currentPos = tmpStr.find_first_of( " ", 0 );
        // If there is no whitespace, then there's only one parameter
        if ( currentPos == std::string::npos )
        {
            mParameters.push_back( parameterStr );
        }
        // Otherwise, process every parameter
        else
        {
            do
            {
                mParameters.push_back( tmpStr.substr( previousPos, currentPos - previousPos ) );
                previousPos = currentPos + 1;
                currentPos = tmpStr.find_first_of( " ", previousPos );
            } while ( currentPos != std::string::npos );

            mParameters.push_back( tmpStr.substr( previousPos, tmpStr.size() - previousPos ) );
        }
    }
}

CommandLineParameters::CommandLineParameters( const char** parameters, uint32_t numParameters )
{
    for ( uint32_t i = 0; i < numParameters; i++ )
    {
        mParameters.push_back( parameters[ i ] );
    }
}

bool CommandLineParameters::HasParameter( const std::string& name ) const
{
    for ( auto& parameter : mParameters )
    {
        if ( parameter == name )
        {
            return true;
        }
    }

    return false;
}
} // namespace Genesis