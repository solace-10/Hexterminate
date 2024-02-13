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

#include <sstream>

#include "configuration.h"
#include "genesis.h"
#include "logger.h"
#include "rendersystem.h"
#include "resources/resourcevideo.h"
#include "shader.h"
#include "videoplayer.h"

#include "resourcemanager.h"
#include "resources/resourceimage.h"

namespace Genesis
{

VideoPlayer::VideoPlayer()
    : m_Skip( false )
{
    m_SkipKeyPressedToken = FrameWork::GetInputManager()->AddKeyboardCallback( std::bind( &VideoPlayer::Skip, this ), SDL_SCANCODE_ESCAPE, ButtonState::Pressed );
}

VideoPlayer::~VideoPlayer()
{
    if ( FrameWork::GetInputManager() != nullptr )
    {
        FrameWork::GetInputManager()->RemoveKeyboardCallback( m_SkipKeyPressedToken );
    }
}

TaskStatus VideoPlayer::Update( float delta )
{
    m_Queue.clear();
    return TaskStatus::Continue;
}

void VideoPlayer::Render( GLuint& outputTexture )
{
    outputTexture = 0;
}

void VideoPlayer::Play( ResourceVideo* pResourceVideo )
{
    if ( pResourceVideo != nullptr )
    {
        m_Queue.push_back( pResourceVideo );
    }
}

bool VideoPlayer::IsPlaying() const
{
    return ( m_Queue.empty() == false );
}

void VideoPlayer::Skip()
{
    if ( m_Queue.empty() == false && m_Queue.front()->IsSkippable() )
    {
        m_Skip = true;
    }
}

}