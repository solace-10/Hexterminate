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

#include "hyperscape/silverthread.h"
#include "hyperscape/silverthreadrep.h"
#include "misc/mathaux.h"
#include "sector/sector.h"
#include "sector/starinfo.h"

namespace Hexterminate
{

//-----------------------------------------------------------------------------
// SilverThreadRep
//-----------------------------------------------------------------------------

SilverThreadRep::SilverThreadRep( SilverThread* pSilverThread )
    : m_pSilverThread( pSilverThread )
    , m_Show( false )
    , m_Parallax( 0.0f, 0.0f )
    , m_pStarShader( nullptr )
    , m_pStarParallax( nullptr )
    , m_pLocationVB( nullptr )
    , m_pLinkShader( nullptr )
    , m_pLinkParallax( nullptr )
    , m_pLayer( nullptr )
{
    using namespace Genesis;
    ShaderCache* pShaderCache = FrameWork::GetRenderSystem()->GetShaderCache();

    m_pStarShader = pShaderCache->Load( "hyperscape_star" );
    m_pStarParallax = m_pStarShader->RegisterUniform( "k_parallax", ShaderUniformType::FloatVector2 );
    m_pLocationVB = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );
    const size_t locationReserveSize = 128;
    m_LocationPosData.resize( locationReserveSize );
    m_LocationUVData.resize( locationReserveSize );
    m_LocationColorData.resize( locationReserveSize );

    m_pLinkShader = pShaderCache->Load( "hyperscape_link" );
    m_pLinkParallax = m_pLinkShader->RegisterUniform( "k_parallax", ShaderUniformType::FloatVector2 );
    m_pLinkVB = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );
    m_LinkPosData.resize( locationReserveSize );
    m_LinkUVData.resize( locationReserveSize );
    m_LinkColorData.resize( locationReserveSize );

    m_LeftMouseButtonDownToken = FrameWork::GetInputManager()->AddMouseCallback( std::bind( &SilverThreadRep::OnLeftMouseButtonDown, this ), MouseButton::Left, ButtonState::Pressed );
}

SilverThreadRep::~SilverThreadRep()
{
    Genesis::FrameWork::GetInputManager()->RemoveMouseCallback( m_LeftMouseButtonDownToken );

    delete m_pLocationVB;
}

void SilverThreadRep::Initialise()
{
    Genesis::Scene* pScene = Genesis::FrameWork::GetScene();
    m_pLayer = pScene->AddLayer( LAYER_GALAXY, true );
    m_pLayer->AddSceneObject( this, false );
    Show( true );
}

void SilverThreadRep::RemoveFromScene()
{
    m_pLayer->RemoveSceneObject( this );
}

void SilverThreadRep::Update( float delta )
{
    if ( m_Show == false )
    {
        return;
    }

    // m_pGalaxyWindow->Update();

    UpdateInput();

    // if ( m_ExitMenu )
    //{
    //	PopupState state = g_pGame->GetPopup()->GetState();
    //	if ( state == PopupState::Yes )
    //	{
    //		m_ExitMenu = false;
    //		g_pGame->EndGame();
    //	}
    //	else if ( state == PopupState::No )
    //	{
    //		m_pGalaxyWindow->Show( true );
    //		m_ExitMenu = false;
    //	}
    // }
}

void SilverThreadRep::UpdateInput()
{
    using namespace Genesis;
    Genesis::InputManager* pInputManager = Genesis::FrameWork::GetInputManager();

    glm::vec2 mousePosition = pInputManager->GetMousePosition();
    m_Parallax.x = mousePosition.x / static_cast<float>( Configuration::GetScreenWidth() );
    m_Parallax.y = mousePosition.y / static_cast<float>( Configuration::GetScreenHeight() );
}

void SilverThreadRep::Render()
{
    using namespace Genesis;

    if ( m_Show == false )
    {
        return;
    }

    RebuildLocationVB();
    RebuildLinkVB();

    DrawBackground();
    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Disabled );
}

void SilverThreadRep::DrawBackground()
{
    using namespace Genesis;

    m_pStarParallax->Set( m_Parallax );
    m_pLinkParallax->Set( m_Parallax );

    FrameWork::GetRenderSystem()->SetRenderTarget( RenderTargetId::Glow );
    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Blend );
    m_pStarShader->Use();
    m_pLocationVB->Draw();
    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Add );
    m_pLinkShader->Use();
    m_pLinkVB->Draw();
    FrameWork::GetRenderSystem()->SetRenderTarget( RenderTargetId::Default );
    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Blend );
    m_pStarShader->Use();
    m_pLocationVB->Draw();
    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Add );
    m_pLinkShader->Use();
    m_pLinkVB->Draw();
}

void SilverThreadRep::Show( bool state )
{
    if ( state != IsVisible() )
    {
        m_Show = state;
    }
}

void SilverThreadRep::OnLeftMouseButtonDown()
{
    // if ( m_InputPending == false )
    //{
    //	m_InputPending = true;
    //	m_InputTimer = SDL_GetTicks() + 100u;
    // }
}

void SilverThreadRep::RebuildLocationVB()
{
    m_LocationScreenData.clear();
    size_t slotsUsed = 0;
    auto AddLocation = [ this, &slotsUsed ]( float x, float y, float starSize, StarInfo* pStarInfo ) {
        const float halfStarSize = starSize / pStarInfo->GetDistance();
        const float x1 = x - halfStarSize;
        const float x2 = x + halfStarSize;
        const float y1 = y - halfStarSize;
        const float y2 = y + halfStarSize;

        m_LocationPosData[ slotsUsed ] = glm::vec3( x1, y2, 0.0f ); // 0
        m_LocationPosData[ slotsUsed + 1 ] = glm::vec3( x1, y1, 0.0f ); // 1
        m_LocationPosData[ slotsUsed + 2 ] = glm::vec3( x2, y1, 0.0f ); // 2
        m_LocationPosData[ slotsUsed + 3 ] = glm::vec3( x1, y2, 0.0f ); // 0
        m_LocationPosData[ slotsUsed + 4 ] = glm::vec3( x2, y1, 0.0f ); // 2
        m_LocationPosData[ slotsUsed + 5 ] = glm::vec3( x2, y2, 0.0f ); // 3

        m_LocationUVData[ slotsUsed ] = glm::vec2( 0.0f, 1.0f ); // 0
        m_LocationUVData[ slotsUsed + 1 ] = glm::vec2( 0.0f, 0.0f ); // 1
        m_LocationUVData[ slotsUsed + 2 ] = glm::vec2( 1.0f, 0.0f ); // 2
        m_LocationUVData[ slotsUsed + 3 ] = glm::vec2( 0.0f, 1.0f ); // 0
        m_LocationUVData[ slotsUsed + 4 ] = glm::vec2( 1.0f, 0.0f ); // 2
        m_LocationUVData[ slotsUsed + 5 ] = glm::vec2( 1.0f, 1.0f ); // 3

        const float coreSize = 0.01f + static_cast<float>( pStarInfo->GetSeed() % 1000 ) / 10000.0f;
        const glm::vec3& color = pStarInfo->GetCoreColour();
        for ( int i = 0; i < 6; ++i )
        {
            m_LocationColorData[ slotsUsed + i ] = glm::vec4( color.r, color.g, color.b, coreSize );
        }

        LocationScreenData lsd;
        lsd.position = glm::vec2( x, y );
        lsd.size = starSize / pStarInfo->GetDistance();
        m_LocationScreenData.push_back( lsd );

        slotsUsed += 6;
    };

    using namespace Genesis;
    const HyperscapeLocationVector& locations = m_pSilverThread->GetLocations();
    if ( locations.empty() )
    {
        return;
    }

    const size_t locationVertexCount = locations.size() * 6;
    if ( m_LocationPosData.size() < locationVertexCount )
    {
        m_LocationPosData.resize( locationVertexCount );
        m_LocationUVData.resize( locationVertexCount );
        m_LocationColorData.resize( locationVertexCount );
    }

    const float threadStartX = static_cast<float>( Configuration::GetScreenWidth() / 2 );
    const float threadStartY = static_cast<float>( Configuration::GetScreenHeight() / 2 );
    float threadOffsetX = 0.0f;
    float threadOffsetY = 0.0f;

    const float maxStarSize = 400.0f;
    const float gapX = 350.0f;
    const size_t startingIndex = locations.size() - m_pSilverThread->GetScannedLocationCount() - 1;
    const size_t endIndex = ( startingIndex > 5 ) ? ( startingIndex - 5 ) : 0;
    for ( int i = static_cast<int>( startingIndex ); i >= static_cast<int>( endIndex ); --i )
    {
        AddLocation( threadStartX + threadOffsetX, threadStartY + threadOffsetY, maxStarSize, locations[ i ].GetStarInfo() );
        threadOffsetX -= gapX;
    }

    const float gapY = 350.0f;
    threadOffsetY = threadStartY - gapY / 2.0f * ( m_pSilverThread->GetScannedLocationCount() - 1 );
    for ( size_t i = startingIndex + 1; i < locations.size(); ++i )
    {
        AddLocation( threadStartX + gapX, threadOffsetY, maxStarSize, locations[ i ].GetStarInfo() );
        threadOffsetY += gapY;
    }

    m_pLocationVB->CopyPositions( m_LocationPosData, slotsUsed );
    m_pLocationVB->CopyUVs( m_LocationUVData, slotsUsed );
    m_pLocationVB->CopyColours( m_LocationColorData, slotsUsed );
}

void SilverThreadRep::RebuildLinkVB()
{
    size_t slotsUsed = 0;
    auto AddLink = [ this, &slotsUsed ]( const LocationScreenData& lsdA, const LocationScreenData& lsdB ) {
        const float lineThickness = 6.0f;
        const float halfLineThickness = lineThickness / 2.0f;

        const glm::vec3 src( lsdA.position, 0.0f );
        const glm::vec3 dst( lsdB.position, 0.0f );
        const glm::vec3 dir = glm::normalize( dst - src );
        const glm::vec3 perp( -dir.y * halfLineThickness, dir.x * halfLineThickness, 0.0f );

        m_LinkPosData[ slotsUsed ] = glm::vec3( src.x + perp.x, src.y + perp.y, src.z ); // 0
        m_LinkPosData[ slotsUsed + 1 ] = glm::vec3( src.x - perp.x, src.y - perp.y, src.z ); // 1
        m_LinkPosData[ slotsUsed + 2 ] = glm::vec3( dst.x - perp.x, dst.y - perp.y, dst.z ); // 2
        m_LinkPosData[ slotsUsed + 3 ] = glm::vec3( src.x + perp.x, src.y + perp.y, src.z ); // 0
        m_LinkPosData[ slotsUsed + 4 ] = glm::vec3( dst.x - perp.x, dst.y - perp.y, dst.z ); // 2
        m_LinkPosData[ slotsUsed + 5 ] = glm::vec3( dst.x + perp.x, dst.y + perp.y, dst.z ); // 3

        m_LinkUVData[ slotsUsed ] = glm::vec2( 1.0f, 0.0f ); // 0
        m_LinkUVData[ slotsUsed + 1 ] = glm::vec2( 0.0f, 0.0f ); // 1
        m_LinkUVData[ slotsUsed + 2 ] = glm::vec2( 0.0f, 1.0f ); // 2
        m_LinkUVData[ slotsUsed + 3 ] = glm::vec2( 1.0f, 0.0f ); // 0
        m_LinkUVData[ slotsUsed + 4 ] = glm::vec2( 0.0f, 1.0f ); // 2
        m_LinkUVData[ slotsUsed + 5 ] = glm::vec2( 1.0f, 1.0f ); // 3

        for ( int i = 0; i < 6; ++i )
        {
            m_LinkColorData[ slotsUsed + i ] = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
        }

        slotsUsed += 6;
    };

    using namespace Genesis;
    if ( m_LocationScreenData.empty() )
    {
        return;
    }

    const size_t locationsCount = m_LocationScreenData.size();
    const size_t scannedLocations = m_pSilverThread->GetScannedLocationCount();

    for ( size_t i = 0; i < locationsCount - 1 - scannedLocations; ++i )
    {
        AddLink( m_LocationScreenData[ i + 1 ], m_LocationScreenData[ i ] );
    }

    for ( size_t i = 0; i < scannedLocations; ++i )
    {
        AddLink( m_LocationScreenData[ 0 ], m_LocationScreenData[ locationsCount - scannedLocations + i ] );
    }

    m_pLinkVB->CopyPositions( m_LinkPosData, slotsUsed );
    m_pLinkVB->CopyUVs( m_LinkUVData, slotsUsed );
    m_pLinkVB->CopyColours( m_LinkColorData, slotsUsed );
}

} // namespace Hexterminate
