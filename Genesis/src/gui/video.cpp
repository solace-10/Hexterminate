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

#include "video.h"
#include "../resources/resourcevideo.h"
#include "../shaderuniform.h"
#include "../vertexbuffer.h"
#include "../videoplayer.h"

#include "../genesis.h"
#include "../resources/resourceimage.h"

namespace Genesis
{

namespace Gui
{

    ///////////////////////////////////////////////////////////////////////////
    // Image
    ///////////////////////////////////////////////////////////////////////////

    Video::Video()
        : m_pImageVertexBuffer( nullptr )
    {
        m_pImageVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV );
    }

    Video::~Video()
    {
        delete m_pImageVertexBuffer;
    }

    void Video::Render()
    {
        VideoPlayer* pVideoPlayer = FrameWork::GetVideoPlayer();
        if ( pVideoPlayer != nullptr && pVideoPlayer->IsPlaying() )
        {
            GLuint texture = 0;
            pVideoPlayer->Render( texture );

            if ( texture > 0 )
            {
                const glm::vec2& pos = GetPositionAbsolute();
                m_pImageVertexBuffer->CreateTexturedQuad( pos.x, pos.y, mSize.x, mSize.y );

                GuiManager::GetTexturedShaderColourUniform()->Set( mColour.glm() );
                GuiManager::GetTexturedSamplerUniform()->Set( texture, GL_TEXTURE0 );
                GuiManager::GetTexturedShader()->Use();

                m_pImageVertexBuffer->Draw();
            }
        }

        DrawBorder();

        GuiElement::Render();
    }
}
}