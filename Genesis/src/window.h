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

#pragma once

#include <string>

struct SDL_Window;
typedef void* SDL_GLContext;

namespace Genesis
{

class Window
{
public:
    Window( const std::string& title, unsigned int width, unsigned int height, bool fullscreen );
    ~Window();
    void Present();

    unsigned int GetWidth() const;
    unsigned int GetHeight() const;
	unsigned int GetDrawableWidth() const;
	unsigned int GetDrawableHeight() const;
	bool HasFocus() const;

    SDL_Window* GetSDLWindow();
    SDL_GLContext GetSDLThreadGLContext();
    SDL_GLContext GetSDLRenderContext();

private:
    void SetupSwapInterval();

    SDL_Window* m_pWindow;
    SDL_GLContext m_ThreadContext;
    SDL_GLContext m_RenderContext;
    unsigned int m_Width;
    unsigned int m_Height;
};

inline unsigned int Window::GetWidth() const
{
    return m_Width;
}

inline unsigned int Window::GetHeight() const
{
    return m_Height;
}

inline SDL_Window* Window::GetSDLWindow()
{
    return m_pWindow;
}

inline SDL_GLContext Window::GetSDLThreadGLContext()
{
    return m_ThreadContext;
}

inline SDL_GLContext Window::GetSDLRenderContext()
{
    return m_RenderContext;
}

}