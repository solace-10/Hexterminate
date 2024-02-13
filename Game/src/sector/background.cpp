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

#include <algorithm>
#include <sstream>

#include <configuration.h>
#include <genesis.h>
#include <math/misc.h>
#include <rendersystem.h>
#include <resources/resourceimage.h>
#include <resources/resourcemodel.h>
#include <shader.h>
#include <shadercache.h>
#include <shaderuniform.h>
#include <vertexbuffer.h>
#include <xml.h>

#include "background.h"
#include "backgroundinfo.h"
#include "hexterminate.h"
#include "player.h"
#include "sector/starinfo.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// Background
///////////////////////////////////////////////////////////////////////////////

Background::Background( const BackgroundInfo* pBackgroundInfo, StarInfo* pStarInfo )
    : m_pShader( nullptr )
    , m_pBackgroundInfo( pBackgroundInfo )
    , m_pVertexBuffer( nullptr )
    , m_pStarInfo( pStarInfo )
    , m_pStarOffset( nullptr )
    , m_AmbientColour( pBackgroundInfo->GetAmbientColour().glm() )
{
    using namespace Genesis;

    ResourceManager* pResourceManager = FrameWork::GetResourceManager();

    m_pShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "sectorbackground" );

    std::stringstream ss;
    ss << "data/backgrounds/" << pBackgroundInfo->GetFilename();
    ResourceImage* pBackground = (ResourceImage*)pResourceManager->GetResource( ss.str() );
    ShaderUniform* pBackgroundSampler = m_pShader->RegisterUniform( "k_backgroundSampler", ShaderUniformType::Texture );
    pBackgroundSampler->Set( pBackground, GL_TEXTURE0 );

    ResourceImage* pStar = (ResourceImage*)pResourceManager->GetResource( "data/backgrounds/star.jpg" );
    ShaderUniform* pStarSampler = m_pShader->RegisterUniform( "k_starSampler", ShaderUniformType::Texture );
    pStarSampler->Set( pStar, GL_TEXTURE1 );

    ShaderUniform* pHasStar = m_pShader->RegisterUniform( "k_hasStar", ShaderUniformType::Boolean );
    pHasStar->Set( pStarInfo != nullptr );

    if ( pStarInfo != nullptr )
    {
        ShaderUniform* pCoreColour = m_pShader->RegisterUniform( "k_coreColour", ShaderUniformType::FloatVector3 );
        pCoreColour->Set( pStarInfo->GetCoreColour() );

        ShaderUniform* pCoronaColour = m_pShader->RegisterUniform( "k_coronaColour", ShaderUniformType::FloatVector3 );
        pCoronaColour->Set( pStarInfo->GetCoronaColour() );

        ShaderUniform* pDistance = m_pShader->RegisterUniform( "k_distance", ShaderUniformType::Float );
        pDistance->Set( pStarInfo->GetDistance() );

        m_pStarOffset = m_pShader->RegisterUniform( "k_offset", ShaderUniformType::FloatVector2 );
        m_pStarOffset->Set( pStarInfo->GetOffset() );

        const glm::vec2 imageSize( static_cast<float>( pBackground->GetWidth() ), static_cast<float>( pBackground->GetHeight() ) );
        const glm::vec2 screenSize( static_cast<float>( Configuration::GetScreenWidth() ), static_cast<float>( Configuration::GetScreenHeight() ) );
        glm::vec2 starUvScale = imageSize / screenSize;

        if ( starUvScale.x > 1.0f )
        {
            starUvScale.x = 1.0f;
            starUvScale.y *= starUvScale.x;
        }

        ShaderUniform* pStarUvScale = m_pShader->RegisterUniform( "k_starUvScale", ShaderUniformType::FloatVector2 );
        pStarUvScale->Set( starUvScale );

        m_AmbientColour = glm::mix( m_pBackgroundInfo->GetAmbientColour().glm(), glm::vec4( pStarInfo->GetCoreColour(), 1.0f ), 0.2f );

        // If we are attempting to take a ship capture screenshot, move the star well out of the way!
        if ( g_pGame->IsShipCaptureModeActive() )
        {
            m_pStarOffset->Set( glm::vec2( -3.0f, 0.0f ) );
        }
    }

    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV );
    CreateGeometry();
}

Background::~Background()
{
    delete m_pVertexBuffer;
    delete m_pStarInfo;
}

void Background::CreateGeometry()
{
    using namespace Genesis;

    ResourceManager* pResourceManager = FrameWork::GetResourceManager();
    std::stringstream ss;
    ss << "data/backgrounds/" << m_pBackgroundInfo->GetFilename();
    ResourceImage* pBackground = (ResourceImage*)pResourceManager->GetResource( ss.str() );

    const glm::vec2 imageSize( static_cast<float>( pBackground->GetWidth() ), static_cast<float>( pBackground->GetHeight() ) );
    const glm::vec2 screenSize( static_cast<float>( Configuration::GetScreenWidth() ), static_cast<float>( Configuration::GetScreenHeight() ) );
    glm::vec2 uvScale = screenSize / imageSize;

    if ( uvScale.x > 1.0f )
    {
        uvScale.x = 1.0f;
        uvScale.y *= uvScale.x;
    }

    m_pVertexBuffer->CreateUntexturedQuad( 0.0f, 0.0f, screenSize.x, screenSize.y );

    const float uvs[] = {
        0.0f, uvScale.y, // 0
        0.0f, 0.0f, // 1
        uvScale.x, 0.0f, // 2
        0.0f, uvScale.y, // 0
        uvScale.x, 0.0f, // 2
        uvScale.x, uvScale.y // 3
    };

    m_pVertexBuffer->CopyData( uvs, 12, Genesis::VBO_UV );
}

void Background::Update( float delta )
{
    Genesis::SceneObject::Update( delta );

    if ( m_pStarInfo != nullptr )
    {
        const glm::vec3& shipPositionHk = g_pGame->GetPlayer()->GetShip()->GetTowerPosition();
        glm::vec2 shipOffset( shipPositionHk.x, -shipPositionHk.y );
        static float sParalaxModifier = 30000.0f;
        shipOffset /= sParalaxModifier;
        m_pStarOffset->Set( m_pStarInfo->GetOffset() + shipOffset );
    }
}

void Background::Render()
{
    m_pShader->Use();
    m_pVertexBuffer->Draw();
}

} // namespace Hexterminate