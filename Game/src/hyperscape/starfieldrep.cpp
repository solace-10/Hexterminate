// Copyright 2021 Pedro Nunes
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

#include <configuration.h>
#include <genesis.h>
#include <rendersystem.h>
#include <scene/layer.h>
#include <scene/scene.h>
#include <shader.h>
#include <shadercache.h>
#include <shaderuniform.h>

#include "hyperscape/starfield.h"
#include "hyperscape/starfieldrep.h"
#include "sector/sector.h"

namespace Hexterminate
{

//-----------------------------------------------------------------------------
// StarfieldRep
//-----------------------------------------------------------------------------

StarfieldRep::StarfieldRep( Starfield* pStarfield )
    : m_pStarfield( pStarfield )
    , m_pStarShader( nullptr )
    , m_pStarfieldParallax( nullptr )
    , m_pStarfieldVB( nullptr )
    , m_pLayer( nullptr )
    , m_Parallax( 0.0f, 0.0f )
{
    using namespace Genesis;
    m_pStarShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "hyperscape_starfield" );
    m_pStarfieldParallax = m_pStarShader->RegisterUniform( "k_parallax", ShaderUniformType::FloatVector2 );
    m_pStarfieldVB = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );

    const size_t numVertices = pStarfield->GetEntries().size() * 6;
    m_PositionData.resize( numVertices );
    m_UVData.resize( numVertices );
    m_ColorData.resize( numVertices );
}

StarfieldRep::~StarfieldRep()
{
    delete m_pStarfieldVB;
}

void StarfieldRep::Initialise()
{
    Genesis::Scene* pScene = Genesis::FrameWork::GetScene();
    m_pLayer = pScene->AddLayer( LAYER_GALAXY, true );
    m_pLayer->AddSceneObject( this, false );
}

void StarfieldRep::RemoveFromScene()
{
    m_pLayer->RemoveSceneObject( this );
}

void StarfieldRep::Update( float delta )
{
    using namespace Genesis;
    Genesis::InputManager* pInputManager = Genesis::FrameWork::GetInputManager();

    glm::vec2 mousePosition = pInputManager->GetMousePosition();
    m_Parallax.x = mousePosition.x / static_cast<float>( Configuration::GetScreenWidth() );
    m_Parallax.y = mousePosition.y / static_cast<float>( Configuration::GetScreenHeight() );
}

void StarfieldRep::Render()
{
    using namespace Genesis;

    BuildStarfield();

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Add );
    m_pStarfieldParallax->Set( m_Parallax );
    m_pStarShader->Use();
    m_pStarfieldVB->Draw();
    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Disabled );
}

void StarfieldRep::BuildStarfield()
{
    size_t idx = 0u;
    for ( const StarfieldEntry& entry : m_pStarfield->GetEntries() )
    {
        const glm::vec3& pos = entry.GetPosition();
        const float halfSize = entry.GetSize() / 2.0f;

        m_PositionData[ idx ] = glm::vec3( pos.x - halfSize, pos.y - halfSize, pos.z ); // 0
        m_PositionData[ idx + 1 ] = glm::vec3( pos.x - halfSize, pos.y + halfSize, pos.z ); // 1
        m_PositionData[ idx + 2 ] = glm::vec3( pos.x + halfSize, pos.y + halfSize, pos.z ); // 2
        m_PositionData[ idx + 3 ] = glm::vec3( pos.x - halfSize, pos.y - halfSize, pos.z ); // 0
        m_PositionData[ idx + 4 ] = glm::vec3( pos.x + halfSize, pos.y + halfSize, pos.z ); // 2
        m_PositionData[ idx + 5 ] = glm::vec3( pos.x + halfSize, pos.y - halfSize, pos.z ); // 3

        m_UVData[ idx ] = glm::vec2( 0.0f, 1.0f ); // 0
        m_UVData[ idx + 1 ] = glm::vec2( 0.0f, 0.0f ); // 1
        m_UVData[ idx + 2 ] = glm::vec2( 1.0f, 0.0f ); // 2
        m_UVData[ idx + 3 ] = glm::vec2( 0.0f, 1.0f ); // 0
        m_UVData[ idx + 4 ] = glm::vec2( 1.0f, 0.0f ); // 2
        m_UVData[ idx + 5 ] = glm::vec2( 1.0f, 1.0f ); // 3

        const glm::vec4 color( 1.0f, 1.0f, 1.0f, entry.GetProgress() );
        m_ColorData[ idx ] = color;
        m_ColorData[ idx + 1 ] = color;
        m_ColorData[ idx + 2 ] = color;
        m_ColorData[ idx + 3 ] = color;
        m_ColorData[ idx + 4 ] = color;
        m_ColorData[ idx + 5 ] = color;

        idx += 6;
    }

    m_pStarfieldVB->CopyPositions( m_PositionData );
    m_pStarfieldVB->CopyUVs( m_UVData );
    m_pStarfieldVB->CopyColours( m_ColorData );
}

} // namespace Hexterminate
