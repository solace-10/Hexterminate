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

#include <sstream>

#include "render/rendertarget.h"
#include "logger.h"
#include "genesis.h"
#include "rendersystem.h"
#include "window.h"

namespace Genesis
{

RenderTargetUniquePtr RenderTarget::Create( const std::string& name, GLuint width, GLuint height, bool hasDepth, bool hasStencil )
{
	// Make sure we are working on the correct context at this point.
	// The behaviour is divergent between Windows (NVIDIA drivers) and Linux:
	// on Windows the context is set, but on Linux we were seeing a null context at this point.
	Window* pWindow = FrameWork::GetWindow();
	if ( SDL_GL_GetCurrentContext() != pWindow->GetSDLRenderContext() )
	{
		SDL_GL_MakeCurrent( pWindow->GetSDLWindow(), pWindow->GetSDLRenderContext() );
	}

	GLuint fboId = 0;
	GLuint colorId = 0;
	GLuint depthId = 0;
	GLuint stencilId = 0;

	glGenTextures( 1, &colorId );
	glBindTexture( GL_TEXTURE_2D, colorId );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glBindTexture( GL_TEXTURE_2D, 0 );

	// create a framebuffer object, you need to delete them when program exits.
	glGenFramebuffers( 1, &fboId );
	glBindFramebuffer( GL_FRAMEBUFFER, fboId );

	if ( hasDepth || hasStencil )
	{
		GLuint depthStencilId = 0;
		glGenRenderbuffers( 1, &depthStencilId );
		glBindRenderbuffer( GL_RENDERBUFFER, depthStencilId );

		if ( hasDepth && hasStencil )
		{
			glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height );
			glBindRenderbuffer( GL_RENDERBUFFER, 0 );
			glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilId );
			depthId = stencilId = depthStencilId;
		}
		else if ( hasDepth )
		{
			glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height );
			glBindRenderbuffer( GL_RENDERBUFFER, 0 );
			glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthStencilId );
			depthId = depthStencilId;
		}
		else if ( hasStencil )
		{
			glRenderbufferStorage( GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height );
			glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilId );
			stencilId = depthStencilId;
		}
	}

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorId, 0 );

	RenderTargetUniquePtr pRenderTarget = std::make_unique<RenderTarget>( name, width, height, fboId, colorId, depthId, stencilId );

	SDL_GLContext pContext = SDL_GL_GetCurrentContext();
	if ( pContext == nullptr )
	{
		FrameWork::GetLogger()->LogError( "Checking for framebuffer status with no current context." );
		return nullptr;
	}

	GLenum status = (GLenum)glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if ( status != GL_FRAMEBUFFER_COMPLETE )
	{
		LogCreationError( name, status );
		return nullptr;
	}
	else
	{
		FrameWork::GetLogger()->LogInfo( "FBO info for render target %s:", name.c_str() );
		FrameWork::GetRenderSystem()->PrintFramebufferInfo( fboId );
	}

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	return pRenderTarget;
}

RenderTarget::RenderTarget( const std::string& name, GLuint width, GLuint height, GLuint fbo, GLuint color, GLuint depth, GLuint stencil ) :
m_Name( name ),
m_Width( width ),
m_Height( height ),
m_FBO( fbo ),
m_ColorAttachment( color ),
m_DepthAttachment( depth ),
m_StencilAttachment( stencil )
{

}

RenderTarget::~RenderTarget()
{

}

void RenderTarget::Clear()
{
	glBindFramebuffer( GL_FRAMEBUFFER, m_FBO );
	GLbitfield mask = GL_COLOR_BUFFER_BIT;
	if ( m_DepthAttachment > 0 )
	{
		mask |= GL_DEPTH_BUFFER_BIT;
	}
	if ( m_StencilAttachment > 0 )
	{
		mask |= GL_STENCIL_BUFFER_BIT;
	}
	glClear( mask );
}

void RenderTarget::LogCreationError( const std::string& name, GLenum status )
{
	std::string reason = "";
	switch ( status )
	{
	case GL_FRAMEBUFFER_COMPLETE:
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		reason = "Unsupported framebuffer format";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		reason = "Framebuffer incomplete, missing attachment";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		reason = "Framebuffer incomplete, missing draw buffer";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		reason = "Framebuffer incomplete, missing read buffer";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		reason = "Framebuffer incomplete attachment";
		break;
	default:
		reason = "Framebuffer unknown error";
		break;
	}

	std::stringstream error;
	error << "Couldn't create render target '" << name << "'. Status code 0x" << std::hex << status << ": " << reason << ".";
	FrameWork::GetLogger()->LogError( "%s", error.str().c_str() );
}

} // namespace Genesis
