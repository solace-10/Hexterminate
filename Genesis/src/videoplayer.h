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

#include <list>

#include "rendersystem.fwd.h"
#include "inputmanager.h"
#include "taskmanager.h"

namespace Genesis
{

class ResourceVideo;
class VertexBuffer;
class Shader;
class ShaderUniform;

class VideoPlayer : public Task
{
public:
    VideoPlayer();
    virtual ~VideoPlayer() override;

    TaskStatus Update( float delta );
    void Render( GLuint& outputTexture );

    void Play( ResourceVideo* pResourceVideo );
    bool IsPlaying() const;
    void Skip();

private:
    typedef std::list<ResourceVideo*> ResourceVideoList;
    ResourceVideoList m_Queue;
    bool m_Skip;
    InputCallbackToken m_SkipKeyPressedToken;
};
}