// Copyright 2017 Pedro Nunes
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
#include <shadercache.h>

#include "hexterminate.h"
#include "menus/eva.h"
#include "menus/loadingscreen.h"

namespace Hexterminate
{

LoadingScreen::LoadingScreen()
    : m_pBackground( nullptr )
    , m_pProgressBorderPanel( nullptr )
    , m_pProgressPanel( nullptr )
    , m_CurrentProgress( 0 )
    , m_MaximumProgress( 0 )
    , m_ProgressMaxWidth( 395 )
{
    using namespace Genesis;

    Gui::GuiManager* pGuiManager = FrameWork::GetGuiManager();
    const int menuWidth = Configuration::GetScreenWidth();
    const int menuHeight = Configuration::GetScreenHeight();

    m_pBackground = new Gui::Image();
    m_pBackground->SetSize( menuWidth, menuHeight );
    m_pBackground->SetPosition( 0, 0 );
    m_pBackground->SetBorderMode( Gui::PANEL_BORDER_NONE );
    m_pBackground->SetTexture( nullptr );
    m_pBackground->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
    m_pBackground->SetShader( FrameWork::GetRenderSystem()->GetShaderCache()->Load( "gui_loading" ) );
    m_pBackground->SetDepth( 101 );
    m_pBackground->Show( false );
    pGuiManager->AddElement( m_pBackground );

    const int screenHalfWidth = Configuration::GetScreenWidth() / 2;
    const int screenHalfHeight = Configuration::GetScreenHeight() / 2;

    const int progressHeight = 16;
    m_pProgressBorderPanel = new Gui::Panel();
    m_pProgressBorderPanel->SetSize( 400, progressHeight );
    m_pProgressBorderPanel->SetPosition( screenHalfWidth - 200, screenHalfHeight + 200 );
    m_pProgressBorderPanel->SetColour( 0.0f, 0.0f, 0.0f, 1.0f );
    m_pProgressBorderPanel->SetBorderMode( Gui::PANEL_BORDER_ALL );
    m_pProgressBorderPanel->SetBorderColour( 0.22f, 0.22f, 0.22f, 1.0f );
    m_pBackground->AddElement( m_pProgressBorderPanel );

    m_pProgressPanel = new Gui::Panel();
    m_pProgressPanel->SetSize( 0, progressHeight - 4 );
    m_pProgressPanel->SetPosition( 3, 2 );
    m_pProgressPanel->SetColour( 0.309f, 1.0f, 1.0f, 1.0f );
    m_pProgressPanel->SetBorderMode( Gui::PANEL_BORDER_NONE );
    m_pProgressBorderPanel->AddElement( m_pProgressPanel );
}

LoadingScreen::~LoadingScreen()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();
    if ( pGuiManager != nullptr )
    {
        pGuiManager->RemoveElement( m_pBackground );
    }
}

void LoadingScreen::Show( bool state )
{
    m_pBackground->Show( state );
    g_pGame->SetInputBlocked( state );
    g_pGame->ShowCursor( !state );
}

void LoadingScreen::SetProgress( size_t current, size_t maximum )
{
    m_CurrentProgress = current;
    m_MaximumProgress = maximum;
}

void LoadingScreen::Update()
{
    if ( m_MaximumProgress == 0 )
    {
        return;
    }

    const float ratio = static_cast<float>( m_CurrentProgress ) / static_cast<float>( m_MaximumProgress );
    m_pProgressPanel->SetWidth( ratio * m_ProgressMaxWidth );
}

} // namespace Hexterminate
