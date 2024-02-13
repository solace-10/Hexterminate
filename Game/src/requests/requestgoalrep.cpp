// Copyright 2015 Pedro Nunes
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

#include "requests/requestgoalrep.h"

#include <configuration.h>
#include <genesis.h>
#include <rendersystem.h>
#include <resources/resourceimage.h>
#include <shader.h>
#include <shadercache.h>
#include <shaderuniform.h>

#include "drawutils.h"
#include "fleet/fleet.h"
#include "fleet/fleetrep.h"
#include "globals.h"
#include "hexterminate.h"
#include "menus/eva.h"
#include "menus/pointofinterest.h"
#include "requests/requestgoal.h"
#include "sector/fogofwar.h"
#include "sector/galaxy.h"
#include "sector/galaxyrep.h"
#include "sector/sectorinfo.h"

namespace Hexterminate
{

RequestGoalRep::RequestGoalRep( RequestGoal* pOwner, const Color& colour )
    : m_pDescription( nullptr )
    , m_pBackground( nullptr )
    , m_pPointOfInterest( nullptr )
    , m_pOwner( pOwner )
    , m_Show( true )
    , m_pShader( nullptr )
    , m_pVertexBuffer( nullptr )
    , m_pGoalTargetImage( nullptr )
{
    using namespace Genesis;

    Gui::GuiManager* pGuiManager = FrameWork::GetGuiManager();

    m_pBackground = new Genesis::Gui::Panel();
    m_pBackground->SetSize( 260.0f, 20.0f );
    m_pBackground->SetPosition( 0.0f, 0.0f );
    m_pBackground->SetColour( colour );
    m_pBackground->SetBorderColour( 1.0f, 1.0f, 1.0f, 0.25f );
    m_pBackground->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
    pGuiManager->AddElement( m_pBackground );

    m_pDescription = new Genesis::Gui::Text();
    m_pDescription->SetSize( 256.0f, 16.0f );
    m_pDescription->SetPosition( 2.0f, 2.0f );
    m_pDescription->SetColour( Color( 1.0f, 1.0f, 1.0f, 1.0f ) );
    m_pDescription->SetFont( EVA_FONT );
    m_pDescription->SetText( "???" );
    m_pDescription->SetMultiLine( false );
    m_pBackground->AddElement( m_pDescription );

    m_pPointOfInterest = new PointOfInterest();
    m_pPointOfInterest->SetPosition( 0.0f, 0.0f );
    m_pPointOfInterest->SetColour( Genesis::Color( 0.0f, 0.6f, 0.6f, 0.6f ) );
    pGuiManager->AddElement( m_pPointOfInterest );

    Show( g_pGame->GetGalaxy()->GetRepresentation()->IsVisible() );

    m_pGoalTargetImage = (Genesis::ResourceImage*)Genesis::FrameWork::GetResourceManager()->GetResource( "data/ui/sector/target.png" );
    m_pShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "target" );
    Genesis::ShaderUniform* pSampler = m_pShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    pSampler->Set( m_pGoalTargetImage, GL_TEXTURE0 );

    glBindTexture( GL_TEXTURE_2D, m_pGoalTargetImage->GetTexture() );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV );
}

RequestGoalRep::~RequestGoalRep()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();
    if ( pGuiManager != nullptr )
    {
        pGuiManager->RemoveElement( m_pBackground );
        pGuiManager->RemoveElement( m_pPointOfInterest );
    }

    delete m_pVertexBuffer;
}

void RequestGoalRep::Update( GalaxyRep* pGalaxyRep )
{
    Show( IsVisible() );

    const glm::vec2& galaxySize = g_pGame->GetGalaxy()->GetRepresentation()->GetSize();
    const float sectorSize = galaxySize.x / NumSectorsX;
    const float halfSectorSize = sectorSize / 2.0f;

    // Goals can be either on sectors or fleets
    if ( m_pOwner->GetSectorInfo() != nullptr )
    {
        const glm::ivec2& sectorCoordinates = m_pOwner->GetSectorInfo()->GetCoordinates();
        const Math::FPoint2 offset = pGalaxyRep->GetOffset();

        m_Position.x = offset.x + sectorSize * sectorCoordinates.x + halfSectorSize;
        m_Position.y = offset.y + sectorSize * sectorCoordinates.y + halfSectorSize;
    }
    else
    {
        FleetSharedPtr pFleet = m_pOwner->GetFleet().lock();
        if ( pFleet != nullptr )
        {
            const glm::vec2& fleetPos = pFleet->GetPosition();
            const Math::FPoint2 offset = pGalaxyRep->GetOffset();
            m_Position.x = fleetPos.x * GalaxyMinSize + offset.x;
            m_Position.y = fleetPos.y * GalaxyMinSize + offset.y;
        }
    }

    // Update where the text overlay is.
    // Make sure the overlay isn't being drawn beyond the edge of the screen.
    const std::string& description = m_pOwner->GetDescription();
    m_pDescription->SetText( description );
    m_pBackground->SetWidth( description.length() * 8.0f + 4.0f );

    const float lineLength = sectorSize * 0.75f - 8.0f;
    bool alignedRight = ( m_Position.x + lineLength + m_pBackground->GetWidth() ) < Genesis::Configuration::GetScreenWidth();

    glm::vec2 end( m_Position.x + ( alignedRight ? lineLength : -lineLength ), m_Position.y );
    m_pPointOfInterest->SetStart( m_Position );
    m_pPointOfInterest->SetEnd( end );

    m_pBackground->SetPosition(
        alignedRight ? (int)end.x : ( (int)end.x - m_pBackground->GetWidth() ),
        (int)( end.y - m_pBackground->GetHeight() / 2.0f ) );
}

void RequestGoalRep::Render()
{
    if ( IsVisible() )
    {
        const float targetWidth = (float)m_pGoalTargetImage->GetWidth();
        const float targetHeight = (float)m_pGoalTargetImage->GetHeight();
        const float targetHalfWidth = targetWidth / 2.0f;
        const float targetHalfHeight = targetHeight / 2.0f;

        m_pVertexBuffer->CreateTexturedQuad(
            m_Position.x - targetHalfWidth,
            m_Position.y - targetHalfHeight,
            targetWidth,
            targetHeight );
        m_pShader->Use();
        m_pVertexBuffer->Draw();
    }
}

void RequestGoalRep::Show( bool state )
{
    if ( state != m_Show )
    {
        m_Show = state;
        m_pBackground->Show( state );
        m_pPointOfInterest->Show( state );
    }
}

bool RequestGoalRep::IsVisible() const
{
    FogOfWar* pFogOfWar = g_pGame->GetGalaxy()->GetFogOfWar();
    if ( pFogOfWar != nullptr && pFogOfWar->IsVisible( m_pOwner->GetSectorInfo() ) == false )
    {
        return false;
    }

    return m_Show;
}

} // namespace Hexterminate
