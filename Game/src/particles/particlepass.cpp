// Copyright 2014 Pedro Nunes
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

#include <genesis.h>
#include <memory.h>
#include <rendersystem.h>
#include <resources/resourceimage.h>
#include <shader.h>
#include <shadercache.h>
#include <shaderuniform.h>

#include "particles/particlepass.h"

namespace Hexterminate
{

ParticlePass::ParticlePass( Genesis::BlendMode blendMode, const std::string& shader, bool glowEnabled )
    : m_GlowEnabled( glowEnabled )
    , m_BlendMode( blendMode )
    , m_pShader( nullptr )
    , m_pSamplerUniform( nullptr )
    , m_pVertexBuffer( nullptr )
{
    using namespace Genesis;

    RenderSystem* pRenderSystem = FrameWork::GetRenderSystem();
    m_pShader = pRenderSystem->GetShaderCache()->Load( shader );
    m_pSamplerUniform = m_pShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );
}

ParticlePass::~ParticlePass()
{
    delete m_pVertexBuffer;
}

} // namespace Hexterminate