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

#include <vector>

#include "SDL.h"
#include "logger.h"

namespace Genesis
{

class ProgramOptions;
class TaskManager;
class Timer;
class InputManager;
class RenderSystem;
class ResourceManager;
class Scene;
class VideoPlayer;
class Window;

namespace Gui
{
    class GuiManager;
}

namespace Render
{
    class DebugRender;
}

namespace Sound
{
    class SoundManager;
}

class FrameWork
{
public:
    static bool Initialize( int argc, char* argv[] );
    static void Shutdown();
    static bool CreateWindowGL( const std::string& name, uint32_t width, uint32_t height, uint32_t multiSampleSamples = 0 );

    static ProgramOptions* GetProgramOptions();
    static TaskManager* GetTaskManager();
    static Logger* GetLogger();
    static InputManager* GetInputManager();
    static Window* GetWindow();
    static RenderSystem* GetRenderSystem();
    static ResourceManager* GetResourceManager();
    static Scene* GetScene();
    static Gui::GuiManager* GetGuiManager();
    static Sound::SoundManager* GetSoundManager();
    static VideoPlayer* GetVideoPlayer();
    static Render::DebugRender* GetDebugRender();
};

} // namespace Genesis
