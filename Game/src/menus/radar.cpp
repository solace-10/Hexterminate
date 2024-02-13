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

#include <configuration.h>
#include <genesis.h>
#include <rendersystem.h>
#include <resources/resourceimage.h>
#include <shader.h>
#include <shadercache.h>
#include <shaderuniform.h>

#include "hexterminate.h"
#include "menus/radar.h"
#include "player.h"
#include "sector/sector.h"
#include "ship/ship.h"
#include "shipyard/shipyard.h"

namespace Hexterminate
{

static const float sRadarSize = 256.0f;
static const float sRadarHalfSize = sRadarSize / 2.0f;
static const float sRadarRange = 1500.0f;
static const float sRadarOutOfRangeScale = 0.75f;

Radar::Radar()
    : m_pTexture( nullptr )
    , m_BackgroundIdx( 0 )
    , m_BackgroundMaskIdx( 0 )
    , m_FriendlyShipIdx( 0 )
    , m_HostileShipIdx( 0 )
    , m_ShipyardIdx( 0 )
    , m_pVertexBuffer( nullptr )
    , m_pShader( nullptr )
    , m_QuadCount( 0 )
{
    using namespace Genesis;
    m_pTexture = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/ui/radar/radar.png" );
    m_Atlas.SetSource( m_pTexture );

    m_BackgroundIdx = m_Atlas.AddElement( 0.0f, 0.0f, 256.0f, 256.0f );
    m_BackgroundMaskIdx = m_Atlas.AddElement( 256.0f, 0.0f, 480.0f, 224.0f );
    m_FriendlyShipIdx = m_Atlas.AddElement( 480.0f, 224.0f, 512.0f, 256.0f );
    m_HostileShipIdx = m_Atlas.AddElement( 448.0f, 224.0f, 480.0f, 256.0f );
    m_ShipyardIdx = m_Atlas.AddElement( 416.0f, 224.0f, 448.0f, 256.0f );

    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV );
    m_pShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "radar" );
    ShaderUniform* pSampler = m_pShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    pSampler->Set( m_pTexture, GL_TEXTURE0 );

    SetPosition( Configuration::GetScreenWidth() - sRadarSize - 8.0f, Configuration::GetScreenHeight() - sRadarSize - 8.0f );
    SetSize( sRadarSize, sRadarSize );

    m_CachedPlayerShipPosition = glm::vec3( 0.0f, 0.0f, 0.0f );

    SetHiddenForCapture( true );
}

Radar::~Radar()
{
    delete m_pVertexBuffer;
}

void Radar::Update( float delta )
{
    Genesis::Gui::GuiElement::Update( delta );

    m_Icons.clear();
    m_PositionData.clear();
    m_UVData.clear();

    CachePlayerShipPosition();

    AddShipyard();
    AddShips();

    m_QuadCount = ( 1 + m_Icons.size() );

    // * 6 = vertices per quad
    m_PositionData.reserve( m_QuadCount * 6 );
    m_UVData.reserve( m_QuadCount * 6 );

    AddQuad( m_BackgroundIdx, sRadarHalfSize, sRadarHalfSize );
    for ( auto& pIcon : m_Icons )
    {
        AddQuad( pIcon.GetAtlasIndex(), pIcon.GetX(), pIcon.GetY(), pIcon.GetScale() );
    }

    m_pVertexBuffer->CopyPositions( m_PositionData );
    m_pVertexBuffer->CopyUVs( m_UVData );
}

void Radar::CachePlayerShipPosition()
{
    Ship* pPlayerShip = g_pGame->GetPlayer()->GetShip();
    if ( pPlayerShip == nullptr || pPlayerShip->IsModuleEditLocked() )
    {
        return;
    }

    if ( pPlayerShip != nullptr && pPlayerShip->GetTowerModule() != nullptr )
    {
        m_CachedPlayerShipPosition = pPlayerShip->GetTowerPosition();
    }
}

void Radar::Render()
{
    using namespace Genesis;

    Genesis::Gui::GuiElement::Render();

    if ( m_QuadCount > 0 )
    {
        FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Blend );

        m_pShader->Use();
        m_pVertexBuffer->Draw( static_cast<uint32_t>( m_QuadCount * 6 ) );

        FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Disabled );
    }
}

void Radar::AddQuad( int atlasIndex, float x, float y, float scale /* = 1.0f */ )
{
    const glm::vec2& pos = GetPositionAbsolute();
    x += pos.x;
    y += pos.y;

    const Genesis::Gui::AtlasElement& element = m_Atlas.GetElement( atlasIndex );
    const float hw = element.GetWidth() / 2.0f * scale;
    const float hh = element.GetHeight() / 2.0f * scale;

    m_PositionData.emplace_back( x - hw, y - hh, 0.0f );
    m_PositionData.emplace_back( x + hw, y - hh, 0.0f );
    m_PositionData.emplace_back( x - hw, y + hh, 0.0f );
    m_PositionData.emplace_back( x + hw, y - hh, 0.0f );
    m_PositionData.emplace_back( x + hw, y + hh, 0.0f );
    m_PositionData.emplace_back( x - hw, y + hh, 0.0f );

    m_UVData.emplace_back( element.GetU1(), element.GetV1() );
    m_UVData.emplace_back( element.GetU2(), element.GetV1() );
    m_UVData.emplace_back( element.GetU1(), element.GetV2() );
    m_UVData.emplace_back( element.GetU2(), element.GetV1() );
    m_UVData.emplace_back( element.GetU2(), element.GetV2() );
    m_UVData.emplace_back( element.GetU1(), element.GetV2() );
}

void Radar::AddShips()
{
    Ship* pPlayerShip = g_pGame->GetPlayer()->GetShip();
    if ( pPlayerShip == nullptr )
    {
        return;
    }

    const ShipList& ships = g_pGame->GetCurrentSector()->GetShipList();
    for ( ShipList::const_iterator it = ships.begin(), itEnd = ships.end(); it != itEnd; ++it )
    {
        Ship* pShip = *it;

        if ( pPlayerShip == pShip )
        {
            AddIcon( m_FriendlyShipIdx, m_CachedPlayerShipPosition );
        }
        else
        {
            if ( pShip->IsDestroyed() )
            {
                continue;
            }

            if ( Faction::sIsEnemyOf( pShip->GetFaction(), g_pGame->GetPlayerFaction() ) )
            {
                AddIcon( m_HostileShipIdx, pShip->GetTowerPosition() );
            }
            else
            {
                AddIcon( m_FriendlyShipIdx, pShip->GetTowerPosition() );
            }
        }
    }
}

void Radar::AddShipyard()
{
    Shipyard* pShipyard = g_pGame->GetCurrentSector()->GetShipyard();
    if ( pShipyard != nullptr )
    {
        // HACK: nudge the shipyard's icon position a bit to the right, since the shipyard's
        // model is (on purpose) not centered at the origin.
        glm::vec3 shipyardPositon = pShipyard->GetPosition() + glm::vec3( 100.0f, 0.0f, 0.0f );
        AddIcon( m_ShipyardIdx, shipyardPositon );
    }
}

void Radar::AddIcon( int atlasIndex, const glm::vec3& position )
{
    float distance = glm::distance( m_CachedPlayerShipPosition, position );
    float scale = 1.0f;
    if ( distance <= std::numeric_limits<float>::epsilon() ) // The player ship.
    {
        m_Icons.emplace_back( atlasIndex, sRadarHalfSize, sRadarHalfSize, scale );
    }
    else
    {
        if ( distance > sRadarRange )
        {
            distance = sRadarRange;
            scale = sRadarOutOfRangeScale;
        }

        glm::vec3 pos = glm::normalize( position - m_CachedPlayerShipPosition );
        pos *= distance / sRadarRange * ( sRadarHalfSize - 16.0f * scale );
        const float x = pos.x + sRadarHalfSize;
        const float y = -pos.y + sRadarHalfSize;

        m_Icons.emplace_back( atlasIndex, x, y, scale );
    }
}

} // namespace Hexterminate