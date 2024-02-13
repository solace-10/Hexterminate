// Copyright 2018 Pedro Nunes
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

#include "menus/endgamewindow.h"

#include <configuration.h>
#include <genesis.h>

#include "hexterminate.h"
#include "menus/buttonevent.h"
#include "menus/eva.h"
#include "misc/gui.h"
#include "player.h"

namespace Hexterminate
{

EndGameWindow::EndGameWindow()
    : m_pMainPanel( nullptr )
    , m_pButtonClose( nullptr )
    , m_pTitle( nullptr )
{
}

EndGameWindow::~EndGameWindow()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();
    if ( pGuiManager != nullptr && m_pMainPanel != nullptr )
    {
        pGuiManager->RemoveElement( m_pMainPanel );
        m_pMainPanel = nullptr;
    }
}

void EndGameWindow::Init()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();

    const int panelWidth = 1365;
    const int panelHeight = 768;

    m_pMainPanel = new Genesis::Gui::Image();
    m_pMainPanel->SetSize( panelWidth, panelHeight );
    m_pMainPanel->SetBorderColour( EVA_COLOUR_BORDER );
    m_pMainPanel->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
    m_pMainPanel->SetPosition(
        (int)( ( Genesis::Configuration::GetScreenWidth() - panelWidth ) / 2.0f ),
        (int)( ( Genesis::Configuration::GetScreenHeight() - panelHeight ) / 2.0f ) );
    m_pMainPanel->Show( false );
    m_pMainPanel->SetTexture( (Genesis::ResourceImage*)Genesis::FrameWork::GetResourceManager()->GetResource( "data/backgrounds/Victory.jpg" ) );
    pGuiManager->AddElement( m_pMainPanel );

    m_pButtonClose = new ButtonEvent( this, Genesis::Gui::GuiEvent::Close );
    m_pButtonClose->SetPosition( panelWidth - 32, 0 );
    m_pButtonClose->SetSize( 32, 32 );
    m_pButtonClose->SetColour( 0.0f, 0.0f, 0.0f, 0.0f );
    m_pButtonClose->SetHoverColour( 0.0f, 0.0f, 0.0f, 0.0f );
    m_pButtonClose->SetIconColour( EVA_BUTTON_ICON_COLOUR );
    m_pButtonClose->SetIconHoverColour( EVA_BUTTON_ICON_COLOUR_HOVER );
    m_pButtonClose->SetIcon( "data/ui/icons/close.png" );
    m_pButtonClose->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
    m_pMainPanel->AddElement( m_pButtonClose );

    m_pButtonResume = new ButtonEvent( this, Genesis::Gui::GuiEvent::Close );
    m_pButtonResume->SetPosition( panelWidth / 2 + 32, panelHeight - 150 );
    m_pButtonResume->SetSize( 160, 32 );
    m_pButtonResume->SetColour( EVA_BUTTON_COLOUR_BACKGROUND );
    m_pButtonResume->SetHoverColour( EVA_BUTTON_COLOUR_HOVER );
    m_pButtonResume->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
    m_pButtonResume->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
    m_pButtonResume->SetFont( EVA_FONT );
    m_pButtonResume->SetText( "Carry on" );
    m_pMainPanel->AddElement( m_pButtonResume );

    m_pButtonReturnToMenu = new ButtonReturnToMenu();
    m_pButtonReturnToMenu->SetPosition( panelWidth / 2 - 192, panelHeight - 150 );
    m_pButtonReturnToMenu->SetSize( 160, 32 );
    m_pButtonReturnToMenu->SetColour( EVA_BUTTON_COLOUR_BACKGROUND );
    m_pButtonReturnToMenu->SetHoverColour( EVA_BUTTON_COLOUR_HOVER );
    m_pButtonReturnToMenu->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
    m_pButtonReturnToMenu->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
    m_pButtonReturnToMenu->SetFont( EVA_FONT );
    m_pButtonReturnToMenu->SetText( "Return to menu" );
    m_pMainPanel->AddElement( m_pButtonReturnToMenu );

    m_pTitle = GuiExtended::CreateText( 8, 8, 256, 32, "> The end", m_pMainPanel );
}

void EndGameWindow::Show( bool state )
{
    if ( m_pMainPanel != nullptr )
    {
        m_pMainPanel->Show( state );
        g_pGame->SetInputBlocked( state );
    }

    if ( state )
    {
        g_pGame->Pause();
    }
    else
    {
        g_pGame->Unpause();
    }
}

bool EndGameWindow::IsVisible() const
{
    return m_pMainPanel ? m_pMainPanel->IsVisible() : false;
}

bool EndGameWindow::HandleEvent( Genesis::Gui::GuiEvent event )
{
    using namespace Genesis::Gui;
    if ( event == GuiEvent::Close )
    {
        Show( false );
        return true;
    }
    else
    {
        return false;
    }
}

void ButtonReturnToMenu::OnPress()
{
    g_pGame->EndGame();
}

} // namespace Hexterminate
