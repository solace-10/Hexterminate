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

#include <sstream>

#include <genesis.h>

#include "hexterminate.h"
#include "menus/perks/perktooltip.h"
#include "misc/gui.h"
#include "player.h"

namespace Hexterminate
{

PerkTooltip::PerkTooltip()
    : m_pMainPanel( nullptr )
    , m_pTitle( nullptr )
    , m_pState( nullptr )
    , m_pDescription( nullptr )
    , m_pCost( nullptr )
    , m_pCostIcon( nullptr )
    , m_Cost( 0 )
    , m_PerkState( PerkState::Locked )
{
}

PerkTooltip::~PerkTooltip()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();
    if ( pGuiManager != nullptr && m_pMainPanel != nullptr )
    {
        pGuiManager->RemoveElement( m_pMainPanel );
        m_pMainPanel = nullptr;
    }
}

void PerkTooltip::Init( const std::string& name, const std::string& description, unsigned int cost )
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();

    const int panelWidth = 256;
    const int panelHeight = 256;

    m_pMainPanel = new Genesis::Gui::Panel();
    m_pMainPanel->SetSize( panelWidth, panelHeight );
    m_pMainPanel->SetColour( Genesis::Color( 0.0f, 0.0f, 0.0f, 0.85f ) );
    m_pMainPanel->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
    m_pMainPanel->Show( false );
    pGuiManager->AddElement( m_pMainPanel );

    m_pTitle = GuiExtended::CreateText( 8, 8, 256, 32, name, m_pMainPanel, true );
    m_pState = GuiExtended::CreateText( 8, 32, 256, 32, "<state>", m_pMainPanel );
    m_pDescription = GuiExtended::CreateText( 8, 56, panelWidth - 16, panelHeight, description, m_pMainPanel, false, true );

    const int numLines = m_pDescription->GetLineCount();
    const float lineHeight = m_pDescription->GetFont()->GetLineHeight();
    const float contentHeight = m_pDescription->GetPosition().y + numLines * lineHeight + 8.0f;
    m_pMainPanel->SetHeight( contentHeight );

    m_Cost = cost;
    std::stringstream ss;
    ss << cost;
    m_pCost = GuiExtended::CreateText( panelWidth - 25 - 8 * static_cast<int>( ss.str().length() ), 8, 256, 32, ss.str(), m_pMainPanel, true );

    m_pCostIcon = new Genesis::Gui::Image();
    m_pCostIcon->SetSize( 16, 16 );
    m_pCostIcon->SetPosition( panelWidth - 21, 8 );
    m_pCostIcon->SetColour( 1.0f, 1.0f, 1.0f, 0.9f );
    m_pCostIcon->SetTexture( (Genesis::ResourceImage*)Genesis::FrameWork::GetResourceManager()->GetResource( "data/ui/icons/perks.png" ) );
    m_pMainPanel->AddElement( m_pCostIcon );
}

void PerkTooltip::SetState( PerkState state )
{
    m_PerkState = state;

    if ( state == PerkState::Enabled )
    {
        m_pCostIcon->Show( false );
        m_pCost->Show( false );
        m_pState->SetText( "Already purchased." );
        m_pState->SetColour( Genesis::Color( 0.0f, 1.0f, 0.0f, 0.8f ) );
    }
    else if ( state == PerkState::Locked )
    {
        m_pCostIcon->Show( true );
        m_pCost->Show( true );

        if ( g_pGame->GetPlayer() == nullptr || m_Cost > g_pGame->GetPlayer()->GetPerkPoints() )
        {
            m_pCost->SetColour( Genesis::Color( 1.0f, 0.0f, 0.0f, 0.8f ) );
            m_pState->SetColour( Genesis::Color( 1.0f, 0.0f, 0.0f, 0.8f ) );
            m_pState->SetText( "Insufficient perk points." );
        }
        else
        {
            m_pCost->SetColour( EVA_TEXT_COLOUR );
            m_pState->SetColour( Genesis::Color( 0.0f, 1.0f, 1.0f, 0.8f ) );
            m_pState->SetText( "Can purchase perk." );
        }
    }
    else if ( state == PerkState::Disabled )
    {
        m_pCostIcon->Show( true );
        m_pCost->Show( true );

        if ( g_pGame->GetPlayer() == nullptr || m_Cost > g_pGame->GetPlayer()->GetPerkPoints() )
        {
            m_pCost->SetColour( Genesis::Color( 1.0f, 0.0f, 0.0f, 0.8f ) );
        }
        else
        {
            m_pCost->SetColour( EVA_TEXT_COLOUR );
        }

        m_pState->SetColour( Genesis::Color( 1.0f, 0.0f, 0.0f, 0.8f ) );
        m_pState->SetText( "Previous perk required." );
    }
}

void PerkTooltip::Show( bool state )
{
    using namespace Genesis;

    if ( m_pMainPanel != nullptr )
    {
        m_pMainPanel->Show( state );

        if ( state )
        {
            SetState( m_PerkState );
            const glm::vec2& mousePos = FrameWork::GetInputManager()->GetMousePosition();
            m_pMainPanel->SetPosition( static_cast<int>( mousePos.x ) + 12, static_cast<int>( mousePos.y ) + 12 );
        }
    }
}

} // namespace Hexterminate
