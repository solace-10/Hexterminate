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

#include <glm/gtc/matrix_access.hpp>

#include <genesis.h>
#include <scene/scene.h>

#include "hexterminate.h"
#include "player.h"
#include "ship/ship.h"
#include <configuration.h>
#include <rendersystem.h>
#include <shader.h>
#include <shadercache.h>
#include <shaderuniform.h>
#include <vertexbuffer.h>

#include "sector/boundary.h"

namespace Hexterminate
{

const float sPlayableArea = 10000.0f;
const float sBoundarySize = 3000.0f;
const float sTimeAllowedOutsideSector = 10.0f;

Boundary::Boundary()
    : m_DestructionTimer( sTimeAllowedOutsideSector )
    , m_WarningTimer( 0.0f )
{
    using namespace Genesis;

    m_pShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "boundary" );
    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV );
}

Boundary::~Boundary()
{
    delete m_pVertexBuffer;
}

void Boundary::Update( float delta )
{
    Genesis::SceneObject::Update( delta );

    Ship* pPlayerShip = g_pGame->GetPlayer()->GetShip();
    if ( pPlayerShip != nullptr && pPlayerShip->IsDestroyed() == false )
    {
        glm::vec3 position( glm::column( pPlayerShip->GetTransform(), 3 ) );
        const float halfPlayableArea = sPlayableArea / 2.0f;
        if ( position.x < -halfPlayableArea || position.x > halfPlayableArea || position.y < -halfPlayableArea || position.y > halfPlayableArea )
        {
            if ( m_WarningTimer <= 0.0f )
            {
                g_pGame->AddIntel( GameCharacter::FleetIntelligence, "WARNING: We must not go beyond the sector's designated area! The ship will self-destruct if we stay here for too long." );
                m_WarningTimer = 10.0f;
            }

            m_DestructionTimer -= delta;
            if ( m_DestructionTimer <= 0.0f )
            {
                pPlayerShip->GetTowerModule()->Destroy();
            }
        }
        else
        {
            m_DestructionTimer = sTimeAllowedOutsideSector;
            m_WarningTimer = std::max( 0.0f, m_WarningTimer - delta );
        }
    }
}

void Boundary::Render()
{
    using namespace Genesis;

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Add );

    const unsigned int numVertices = 24u;
    PositionData posData;
    UVData uvData;
    posData.reserve( numVertices );
    uvData.reserve( numVertices );

    struct Quad
    {
        Quad( float _x1, float _y1, float _x2, float _y2 )
            : x1( _x1 )
            , y1( _y1 )
            , x2( _x2 )
            , y2( _y2 )
        {
        }
        float x1, y1, x2, y2;
    };

    const float halfPlayableArea = sPlayableArea / 2.0f;

    std::array<Quad, 4> quads = {
        Quad( -halfPlayableArea - sBoundarySize, -halfPlayableArea, halfPlayableArea + sBoundarySize, -halfPlayableArea - sBoundarySize ), // Top
        Quad( -halfPlayableArea - sBoundarySize, halfPlayableArea, halfPlayableArea + sBoundarySize, halfPlayableArea + sBoundarySize ), // Bottom
        Quad( -halfPlayableArea - sBoundarySize, halfPlayableArea, -halfPlayableArea, -halfPlayableArea ), // Left
        Quad( halfPlayableArea, halfPlayableArea, halfPlayableArea + sBoundarySize, -halfPlayableArea ) // Right
    };

    for ( auto& quad : quads )
    {
        posData.emplace_back( quad.x1, quad.y1, 0.0f ); // 0
        posData.emplace_back( quad.x1, quad.y2, 0.0f ); // 1
        posData.emplace_back( quad.x2, quad.y2, 0.0f ); // 2
        posData.emplace_back( quad.x1, quad.y1, 0.0f ); // 0
        posData.emplace_back( quad.x2, quad.y2, 0.0f ); // 2
        posData.emplace_back( quad.x2, quad.y1, 0.0f ); // 3

        uvData.emplace_back( 0.0f, 0.0f ); // 0
        uvData.emplace_back( 0.0f, 1.0f ); // 1
        uvData.emplace_back( 1.0f, 1.0f ); // 2
        uvData.emplace_back( 0.0f, 0.0f ); // 0
        uvData.emplace_back( 1.0f, 1.0f ); // 2
        uvData.emplace_back( 1.0f, 0.0f ); // 3
    }

    m_pVertexBuffer->CopyPositions( posData );
    m_pVertexBuffer->CopyUVs( uvData );
    m_pShader->Use();
    m_pVertexBuffer->Draw();

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Disabled );
}

} // namespace Hexterminate