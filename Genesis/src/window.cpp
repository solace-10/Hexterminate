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

#include "SDL.h"

#include "genesis.h"
#include "logger.h"
#include "window.h"

namespace Genesis
{

Window::Window( const std::string& title, unsigned int width, unsigned int height, bool fullscreen )
{
    Uint32 flags = SDL_WINDOW_OPENGL;

    if ( fullscreen )
    {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    else
    {
        flags |= SDL_WINDOW_BORDERLESS;
    }

    m_pWindow = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        flags );

    m_Width = width;
    m_Height = height;

    if ( m_pWindow == nullptr )
    {
        Genesis::FrameWork::GetLogger()->LogError( "Failed to create window: %s", SDL_GetError() );
    }

    SDL_GL_SetAttribute( SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1 );
    m_ThreadContext = SDL_GL_CreateContext( m_pWindow );
    m_RenderContext = SDL_GL_CreateContext( m_pWindow );

    SetupSwapInterval();

    SDL_SetWindowGrab( m_pWindow, SDL_TRUE );
}

Window::~Window()
{
    if ( m_pWindow != nullptr )
    {
        SDL_DestroyWindow( m_pWindow );
        SDL_GL_DeleteContext( m_ThreadContext );
        SDL_GL_DeleteContext( m_RenderContext );
    }
}

void Window::Present()
{
    SDL_GL_SwapWindow( m_pWindow );
}

void Window::SetupSwapInterval()
{
    // Some systems allow specifying -1 for the interval, to enable late swap tearing.
    // Late swap tearing works the same as vsync, but if you've already missed the vertical retrace for a given frame,
    // it swaps buffers immediately, which might be less jarring for the user during occasional framerate drops.
    // If application requests late swap tearing and the system does not support it, this function will fail and return -1.
    const bool lateSwapTearing = ( SDL_GL_SetSwapInterval( -1 ) == 0 );
    if ( lateSwapTearing == false )
    {
        FrameWork::GetLogger()->LogInfo( "Couldn't enable late swap tearing: %s", SDL_GetError() );
        const bool vsync = ( SDL_GL_SetSwapInterval( 1 ) == 0 );
        if ( vsync )
        {
            FrameWork::GetLogger()->LogInfo( "Falling back to vsync: success" );
        }
        else
        {
            FrameWork::GetLogger()->LogInfo( "Failed to enable vsync: %s", SDL_GetError() );
        }
    }
}

unsigned int Window::GetDrawableWidth() const
{
	int w, h;
	SDL_GL_GetDrawableSize( m_pWindow, &w, &h );
	return static_cast<unsigned int>( w );
}

unsigned int Window::GetDrawableHeight() const
{
	int w, h;
	SDL_GL_GetDrawableSize( m_pWindow, &w, &h );
	return static_cast<unsigned int>( h );
}

bool Window::HasFocus() const
{
	return ( ( SDL_GetWindowFlags( m_pWindow ) & SDL_WINDOW_MOUSE_FOCUS ) == 1 );
}

}
