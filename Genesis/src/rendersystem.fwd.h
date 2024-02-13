// Copyright 2021 Pedro Nunes
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

#define NO_SDL_GLEXT
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

namespace Genesis
{

enum class BlendMode
{
    Disabled,
    Blend,
    Add,
    Multiply,
    Screen
};

enum class RenderTargetId
{
    None,
    Default,
    Glow,
    GlowBlurHorizontal,
    GlowBlurVertical,
    Radar,

	Count
};

enum class IntersectionResult
{
    Success,
    FailureParallel,
    FailureCoplanar
};

} // namespace Genesis 
