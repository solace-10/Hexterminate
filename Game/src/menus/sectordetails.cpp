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

#include "menus/sectordetails.h"

#include <sstream>

#include <configuration.h>
#include <genesis.h>

#include "faction/faction.h"
#include "fleet/fleet.h"
#include "globals.h"
#include "hexterminate.h"
#include "menus/eva.h"
#include "menus/pointofinterest.h"
#include "player.h"
#include "requests/imperialrequest.h"
#include "requests/requestmanager.h"
#include "sector/galaxy.h"
#include "sector/galaxyrep.h"
#include "ship/inventory.h"
#include "ship/moduleinfo.h"
#include "ship/shipinfo.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// SectorDetails
///////////////////////////////////////////////////////////////////////////////

SectorDetails::SectorDetails()
    : m_pSectorInfo( nullptr )
    , m_pTitle( nullptr )
    , m_pDeployProbeButton( nullptr )
    , m_pDeployStarfortButton( nullptr )
    , m_pEnterSectorButton( nullptr )
    , m_pThreatBackground( nullptr )
    , m_pPointOfInterest( nullptr )
{
    m_pThreatBackground = new Genesis::Gui::Panel();
    m_pThreatBackground->SetSize( 260.0f, 20.0f );
    m_pThreatBackground->SetPosition( 0.0f, 0.0f );
    m_pThreatBackground->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
    m_pThreatBackground->SetBorderColour( 1.0f, 1.0f, 1.0f, 0.5f );
    m_pThreatBackground->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
    m_pMainPanel->AddElement( m_pThreatBackground );

    m_pTitle = new Genesis::Gui::Text();
    m_pTitle->SetSize( 256.0f, 16.0f );
    m_pTitle->SetPosition( 2.0f, 2.0f );
    m_pTitle->SetColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 1.0f ) );
    m_pTitle->SetFont( EVA_FONT );
    m_pTitle->SetText( "Threat rating: ???" );
    m_pThreatBackground->AddElement( m_pTitle );

    if ( g_pGame->GetGameMode() == GameMode::InfiniteWar )
    {
        m_pDeployProbeButton = new DeployProbeButton();
        m_pDeployProbeButton->SetPosition( 8.0f, 0.0f );
        m_pDeployProbeButton->SetSize( 244.0f, 32.0f );
        m_pDeployProbeButton->SetColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 0.05f ) );
        m_pDeployProbeButton->SetHoverColour( EVA_BUTTON_COLOUR_HOVER );
        m_pDeployProbeButton->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
        m_pDeployProbeButton->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
        m_pDeployProbeButton->SetFont( EVA_FONT );
        m_pDeployProbeButton->SetText( "Deploy probe (500 RU)" );
        m_pMainPanel->AddElement( m_pDeployProbeButton );

        m_pDeployStarfortButton = new DeployStarfortButton();
        m_pDeployStarfortButton->SetPosition( 8.0f, 0.0f );
        m_pDeployStarfortButton->SetSize( 244.0f, 32.0f );
        m_pDeployStarfortButton->SetColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 0.05f ) );
        m_pDeployStarfortButton->SetHoverColour( EVA_BUTTON_COLOUR_HOVER );
        m_pDeployStarfortButton->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
        m_pDeployStarfortButton->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
        m_pDeployStarfortButton->SetFont( EVA_FONT );
        m_pDeployStarfortButton->SetText( "Deploy starfort (4000 RU)" );
        m_pMainPanel->AddElement( m_pDeployStarfortButton );
    }

    m_pEnterSectorButton = new EnterSectorButton();
    m_pEnterSectorButton->SetPosition( 8.0f, 0.0f );
    m_pEnterSectorButton->SetSize( 244.0f, 32.0f );
    m_pEnterSectorButton->SetColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 0.05f ) );
    m_pEnterSectorButton->SetHoverColour( EVA_BUTTON_COLOUR_HOVER );
    m_pEnterSectorButton->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
    m_pEnterSectorButton->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
    m_pEnterSectorButton->SetFont( EVA_FONT );
    m_pEnterSectorButton->SetText( "Enter sector" );
    m_pMainPanel->AddElement( m_pEnterSectorButton );

    m_pPointOfInterest = new PointOfInterest();
    m_pPointOfInterest->SetPosition( 0.0f, 0.0f );
    Genesis::FrameWork::GetGuiManager()->AddElement( m_pPointOfInterest );
}

SectorDetails::~SectorDetails()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();
    if ( pGuiManager != nullptr )
    {
        pGuiManager->RemoveElement( m_pPointOfInterest );
        m_pPointOfInterest = nullptr;
    }
}

void SectorDetails::SetAnchor( float x, float y )
{
    const glm::vec2& galaxySize = g_pGame->GetGalaxy()->GetRepresentation()->GetSize();

    const float sectorWidth = galaxySize.x / NumSectorsX;
    const float sectorHeight = galaxySize.y / NumSectorsY;

    bool horizontalFlip = false;
    const float lineLength = sectorWidth * 1.25f - 8.0f;
    glm::vec2 start( x + sectorWidth * 0.5f, y + sectorHeight * 0.25f );

    // If the menu would be drawn beyond the screen's width, we draw it to the left instead
    if ( start.x + lineLength + m_pMainPanel->GetWidth() > Genesis::Configuration::GetScreenWidth() )
    {
        horizontalFlip = true;
    }

    glm::vec2 end( horizontalFlip ? ( x - lineLength ) : ( x + lineLength ), y + sectorHeight * 0.25f );

    m_pPointOfInterest->SetStart( start );
    m_pPointOfInterest->SetEnd( end );

    glm::vec2 detailsPosition( end.x, end.y - 8.0f );
    if ( horizontalFlip )
    {
        detailsPosition.x = end.x - m_pMainPanel->GetWidth();
    }

    if ( detailsPosition.y + m_pMainPanel->GetHeight() > Genesis::Configuration::GetScreenHeight() )
    {
        detailsPosition.y = Genesis::Configuration::GetScreenHeight() - m_pMainPanel->GetHeight() - 8.0f;
    }

    DetailsWindow::SetAnchor( detailsPosition.x, detailsPosition.y );
}

void SectorDetails::Show( bool state )
{
    DetailsWindow::Show( state );
    m_pPointOfInterest->Show( state );
}

void SectorDetails::SetSectorInfo( SectorInfo* pSectorInfo )
{
    m_pSectorInfo = pSectorInfo;

    std::stringstream contents;
    if ( pSectorInfo != nullptr )
    {
        contents << "\n"
                 << pSectorInfo->GetName() << ( pSectorInfo->IsContested() ? " [CONTESTED]\n" : "\n" );

        contents << pSectorInfo->GetFaction()->GetName() << "\n";

        if ( pSectorInfo->HasHyperspaceInhibitor() )
        {
            contents << "\nHYPERSPACE INHIBITOR PRESENT\n\n";
        }

        const int conquestReward = pSectorInfo->GetConquestReward();
        if ( conquestReward > 0 )
        {
            contents << "Reward: " << conquestReward << " influence\n";
        }

        if ( pSectorInfo->HasProbe() )
        {
            contents << "- Hyperspace probe\n";
        }

        if ( pSectorInfo->HasShipyard() )
        {
            contents << "- Shipyard\n";
        }

        if ( pSectorInfo->HasStarfort() )
        {
            contents << "- Starfort (" << pSectorInfo->GetStarfortHealth() << "%)\n";
        }

        contents << "Forces present:";

        for ( int i = 0; i < (int)FactionId::Count; ++i )
        {
            Faction* pFaction = g_pGame->GetFaction( (FactionId)i );
            const FleetList& factionFleets = pFaction->GetFleets();
            for ( auto& pFleet : factionFleets )
            {
                if ( pFleet->GetCurrentSector() == pSectorInfo )
                {
                    if ( pFaction == g_pGame->GetPlayerFaction() )
                    {
                        contents << "\n- "
                                 << "Player's fleet";
                    }
                    else
                    {
                        contents << "\n- " << pFaction->GetName() << ( pFleet->HasFlagship() ? " main fleet" : " fleet" );
                    }
                }
            }
        }

        if ( pSectorInfo->GetRegionalFleetPoints() > 0 )
        {
            contents << "\n- Regional fleet";
        }

        SetContents( contents.str() );

        ThreatRating threatRating = pSectorInfo->GetThreatRating();
        const Genesis::Color& threatRatingColour = GetThreatRatingColour( threatRating );
        m_pThreatBackground->SetColour( threatRatingColour );
        m_pPointOfInterest->SetColour( threatRatingColour );
        m_pTitle->SetText( GetThreatRatingText( threatRating ) );

        // Position the "Enter sector" button below the contents.
        // The main panel needs to be expanded to be able to fit this.
        // The +0.5f is there to create a vertical border between the button and the content / bottom of the panel.
        Genesis::ResourceFont* pFont = EVA_FONT;
        float offsetY = floor( ( GetNumLines() + 0.5f ) * pFont->GetLineHeight() );

        if ( g_pGame->GetGameMode() == GameMode::InfiniteWar )
        {
            m_pDeployProbeButton->SetSectorInfo( pSectorInfo );
            m_pDeployStarfortButton->SetSectorInfo( pSectorInfo );

            m_pDeployProbeButton->SetPosition( 8.0f, offsetY );
            offsetY += m_pDeployProbeButton->GetSize().y + 8.0f;

            m_pDeployStarfortButton->SetPosition( 8.0f, offsetY );
            offsetY += m_pDeployStarfortButton->GetSize().y + 8.0f;
        }

        m_pEnterSectorButton->SetPosition( 8.0f, offsetY );
        m_pMainPanel->SetSize( m_pMainPanel->GetSize().x, offsetY + m_pEnterSectorButton->GetSize().y + 8.0f );

        if ( pSectorInfo->HasHyperspaceInhibitor() )
        {
            m_pEnterSectorButton->SetColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 0.05f ) );
            m_pEnterSectorButton->SetHoverColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 0.05f ) );
            m_pEnterSectorButton->SetTextColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 0.2f ) );
        }
        else
        {
            m_pEnterSectorButton->SetColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 0.05f ) );
            m_pEnterSectorButton->SetHoverColour( EVA_BUTTON_COLOUR_HOVER );
            m_pEnterSectorButton->SetTextColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 1.0f ) );
        }
    }

    m_pEnterSectorButton->SetSectorInfo( pSectorInfo );
    m_pPointOfInterest->Show( pSectorInfo != nullptr );
}

Genesis::Color SectorDetails::GetThreatRatingColour( ThreatRating rating ) const
{
    using namespace Genesis;

    const float alpha = 0.8f;
    if ( rating == ThreatRating::Trivial )
        return Color( 0.5f, 0.5f, 0.5f, alpha );
    else if ( rating == ThreatRating::Easy )
        return Color( 0.0f, 0.8f, 0.0f, alpha );
    else if ( rating == ThreatRating::Fair )
        return Color( 0.8f, 0.8f, 0.0f, alpha );
    else if ( rating == ThreatRating::Challenging )
        return Color( 1.0f, 0.5f, 0.0f, alpha );
    else if ( rating == ThreatRating::Overpowering )
        return Color( 1.0f, 0.0f, 0.0f, alpha );
    else
        return Color( 0.0f, 0.0f, 0.8f, alpha );
}

std::string SectorDetails::GetThreatRatingText( ThreatRating rating ) const
{
    std::string text = "Threat rating: ";
    if ( rating == ThreatRating::Trivial )
        text += "Trivial";
    else if ( rating == ThreatRating::Easy )
        text += "Easy";
    else if ( rating == ThreatRating::Fair )
        text += "Considerable";
    else if ( rating == ThreatRating::Challenging )
        text += "Challenging";
    else if ( rating == ThreatRating::Overpowering )
        text += "Overpowering";
    else
        text = "Allied sector";

    return text;
}

SectorInfo* SectorDetails::GetSectorInfo() const
{
    return m_pSectorInfo;
}

///////////////////////////////////////////////////////////////////////////////
// GenericSectorButton
///////////////////////////////////////////////////////////////////////////////

GenericSectorButton::GenericSectorButton()
    : m_pSectorInfo( nullptr )
{
}

void GenericSectorButton::SetSectorInfo( SectorInfo* pSectorInfo )
{
    m_pSectorInfo = pSectorInfo;
}

///////////////////////////////////////////////////////////////////////////////
// DeployProbeButton
///////////////////////////////////////////////////////////////////////////////

DeployProbeButton::DeployProbeButton()
{
    m_CanDeploy = false;
    m_Cost = 750;
}

void DeployProbeButton::Update( float delta )
{
    GenericSectorButton::Update( delta );

    if ( m_pSectorInfo->HasProbe() )
    {
        SetTextColour( Genesis::Color( 1.0f, 0.0f, 0.0f, 1.0f ) );
        m_CanDeploy = false;
    }
    else
    {
        FactionId sectorFactionId = m_pSectorInfo->GetFaction()->GetFactionId();
        bool isValidSector = ( sectorFactionId == FactionId::Neutral || sectorFactionId == FactionId::Empire );
        if ( g_pGame->GetPlayer()->GetInfluence() < m_Cost || isValidSector == false )
        {
            SetTextColour( Genesis::Color( 1.0f, 0.0f, 0.0f, 1.0f ) );
            m_CanDeploy = false;
        }
        else
        {
            SetTextColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 1.0f ) );
            m_CanDeploy = true;
        }
    }
}

void DeployProbeButton::OnPress()
{
    g_pGame->SetInputBlocked( true );
    if ( m_CanDeploy )
    {
        m_pSectorInfo->SetProbe( true );
        g_pGame->GetPlayer()->SetInfluence( g_pGame->GetPlayer()->GetInfluence() - m_Cost );
        g_pGame->AddFleetCommandIntel( "Hyperspace probe deployed." );
    }
    else
    {
        FactionId sectorFactionId = m_pSectorInfo->GetFaction()->GetFactionId();
        bool isValidSector = ( sectorFactionId == FactionId::Neutral || sectorFactionId == FactionId::Empire );
        if ( isValidSector == false )
        {
            g_pGame->AddFleetCommandIntel( "Probes can only be deployed in Imperial or neutral space." );
        }
    }
    g_pGame->SetInputBlocked( false );
}

///////////////////////////////////////////////////////////////////////////////
// DeployStarfortButton
///////////////////////////////////////////////////////////////////////////////

DeployStarfortButton::DeployStarfortButton()
{
    m_CanDeploy = false;
    m_Cost = 4000;
}

void DeployStarfortButton::Update( float delta )
{
    GenericSectorButton::Update( delta );

    if ( m_pSectorInfo->HasStarfort() )
    {
        SetTextColour( Genesis::Color( 1.0f, 0.0f, 0.0f, 1.0f ) );
        m_CanDeploy = false;
    }
    else
    {
        FactionId sectorFactionId = m_pSectorInfo->GetFaction()->GetFactionId();
        bool isValidSector = ( sectorFactionId == FactionId::Empire );
        if ( g_pGame->GetPlayer()->GetInfluence() < m_Cost || isValidSector == false )
        {
            SetTextColour( Genesis::Color( 1.0f, 0.0f, 0.0f, 1.0f ) );
            m_CanDeploy = false;
        }
        else
        {
            SetTextColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 1.0f ) );
            m_CanDeploy = true;
        }
    }
}

void DeployStarfortButton::OnPress()
{
    g_pGame->SetInputBlocked( true );
    if ( m_CanDeploy )
    {
        m_pSectorInfo->SetStarfort( true );
        g_pGame->GetPlayer()->SetInfluence( g_pGame->GetPlayer()->GetInfluence() - m_Cost );
        g_pGame->AddFleetCommandIntel( "Starfort deployed." );
    }
    else
    {
        FactionId sectorFactionId = m_pSectorInfo->GetFaction()->GetFactionId();
        bool isValidSector = ( sectorFactionId == FactionId::Empire );
        if ( isValidSector == false )
        {
            g_pGame->AddFleetCommandIntel( "Probes can only be deployed in Imperial space." );
        }
    }
    g_pGame->SetInputBlocked( false );
}

///////////////////////////////////////////////////////////////////////////////
// EnterSectorButton
///////////////////////////////////////////////////////////////////////////////

EnterSectorButton::EnterSectorButton()
{
}

void EnterSectorButton::OnPress()
{
    SDL_assert( m_pSectorInfo != nullptr );
    if ( g_pGame->GetCurrentSector() == nullptr && m_pSectorInfo->HasHyperspaceInhibitor() == false )
    {
        FleetSharedPtr pPlayerFleet = g_pGame->GetPlayerFleet().lock();
        if ( pPlayerFleet != nullptr )
        {
            // If we were fighting and just exited a sector then our fleet is likely immune.
            // Remove the immunity flag so we can join the fray.
            pPlayerFleet->SetImmunity( false );

            m_pSectorInfo->Contest();
        }
    }
}

} // namespace Hexterminate