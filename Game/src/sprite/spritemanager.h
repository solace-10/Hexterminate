// Copyright 2016 Pedro Nunes
//
// This file is part of Hexterminate.
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

#include "sprite/sprite.h"
#include <genesis.h>
#include <rendersystem.h>
#include <resourcemanager.h>
#include <scene/sceneobject.h>
#include <vector>

namespace Genesis
{
class ResourceImage;
class VertexBuffer;
} // namespace Genesis

namespace Hexterminate
{

typedef std::vector<Sprite> SpriteVector;

///////////////////////////////////////////////////////////////////////////////
// SpriteManager
// Draws all the sprites as a single draw call
///////////////////////////////////////////////////////////////////////////////

class SpriteManager : public Genesis::SceneObject
{
public:
    SpriteManager();
    virtual ~SpriteManager() override;

    virtual void Update( float delta ) override;
    virtual void Render() override;

    void AddSprite( const Sprite& Sprite );

private:
    SpriteVector m_Sprites;

    Genesis::ResourceImage* m_pTexture;
    Genesis::Shader* m_pShader;
    Genesis::VertexBuffer* m_pVertexBuffer;
};

} // namespace Hexterminate