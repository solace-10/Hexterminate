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

#include "imgui/imgui_impl.h"
#include "eventhandler.h"
#include "genesis.h"
#include "inputmanager.h"

namespace Genesis
{

TaskStatus EventHandler::Update( float delta )
{
    SDL_Event event;
    while ( SDL_PollEvent( &event ) )
    {
		if ( ImGuiImpl::HandleEvent( &event ) )
		{
			continue;
		}

        switch ( event.type )
        {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEMOTION:
        case SDL_MOUSEWHEEL:
        case SDL_TEXTEDITING:
        case SDL_TEXTINPUT:
            NotifyInputHandler( event );
            break;

        case SDL_QUIT:
            FrameWork::GetTaskManager()->Stop();
            break;

        default:
            break;
        }
    }

    // A new frame in ImGui should only be started after all the events are pumped.
    ImGuiImpl::NewFrame( delta );

    return TaskStatus::Continue;
}

void EventHandler::NotifyInputHandler( SDL_Event& event )
{
    InputManager* pInputManager = FrameWork::GetInputManager();
    if ( pInputManager != nullptr )
    {
        pInputManager->HandleEvent( event );
    }
}
}