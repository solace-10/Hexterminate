// Copyright 2021 Pedro Nunes
//
// This file is part of Genesis.
//
// Hexterminate is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Hexterminate is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hexterminate. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <string>

#include "rendersystem.fwd.h"

namespace Genesis
{

class RenderTarget;
using RenderTargetUniquePtr = std::unique_ptr<RenderTarget>;

class RenderTarget
{
public:
	static RenderTargetUniquePtr Create( const std::string& name, GLuint, GLuint height, bool hasDepth, bool hasStencil );

	RenderTarget( const std::string& name, GLuint width, GLuint height, GLuint fbo, GLuint color, GLuint depth, GLuint stencil );
	~RenderTarget();

	void Clear();

	const std::string& GetName() const;
	GLuint GetWidth() const;
	GLuint GetHeight() const;
	GLuint GetFBO() const;
	GLuint GetColor() const;
	GLuint GetDepth() const;
	GLuint GetStencil() const;

private:
	static void LogCreationError( const std::string& name, GLenum status );

	std::string m_Name;
	GLuint m_Width;
	GLuint m_Height;
	GLuint m_FBO;
	GLuint m_ColorAttachment;
	GLuint m_DepthAttachment;
	GLuint m_StencilAttachment;
};

inline const std::string& RenderTarget::GetName() const
{
	return m_Name;
}

inline GLuint RenderTarget::GetWidth() const
{
	return m_Width;
}

inline GLuint RenderTarget::GetHeight() const
{
	return m_Height;
}

inline GLuint RenderTarget::GetFBO() const
{
	return m_FBO;
}

inline GLuint RenderTarget::GetColor() const
{
	return m_ColorAttachment;
}

inline GLuint RenderTarget::GetDepth() const
{
	SDL_assert( m_DepthAttachment != 0 );
	return m_DepthAttachment;
}

inline GLuint RenderTarget::GetStencil() const
{
	SDL_assert( m_StencilAttachment != 0 );
	return m_StencilAttachment;
}

} // namespace Genesis
