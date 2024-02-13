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

#include <configuration.h>
#include <genesis.h>
#include <rendersystem.h>
#include <resources/resourcesound.h>
#include <shadercache.h>

#include "hexterminate.h"
#include "menus/eva.h"
#include "menus/pointofinterest.h"
#include "menus/tutorialwindow.h"
#include "player.h"
#include "ship/ship.h"
#include "stringaux.h"

namespace Hexterminate
{

static const float sTutorialStepDelay = 0.3f;

///////////////////////////////////////////////////////////////////////////////
// TutorialStep
///////////////////////////////////////////////////////////////////////////////

TutorialStep::TutorialStep( const std::string& content )
    : m_Content( content )
    , m_HasPointOfInterest( false )
    , m_pPointOfInterest( nullptr )
{
}

const glm::vec2& TutorialStep::GetPointOfInterest() const
{
    if ( m_pPointOfInterest != nullptr )
    {
        return m_pPointOfInterest->GetPointOfInterestEnd();
    }
    else
    {
        return m_PointOfInterest;
    }
}

///////////////////////////////////////////////////////////////////////////////
// TutorialWindow
///////////////////////////////////////////////////////////////////////////////

TutorialWindow::TutorialWindow()
    : m_pMainPanel( nullptr )
    , m_pContent( nullptr )
    , m_pTitle( nullptr )
    , m_pIcon( nullptr )
    , m_pNextButton( nullptr )
    , m_LineLength( INT_MAX )
    , m_Dirty( false )
    , m_pSFX( nullptr )
    , m_Delay( 0.0f )
{
    using namespace Genesis;

    Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();

    float panelWidth = 635.0f;
    float panelHeight = 104.0f;

    m_pMainPanel = new Gui::Panel();
    m_pMainPanel->SetSize( panelWidth, panelHeight );
    m_pMainPanel->SetColour( EVA_COLOUR_BACKGROUND );
    m_pMainPanel->SetBorderColour( EVA_COLOUR_BORDER );
    m_pMainPanel->SetBorderMode( Gui::PANEL_BORDER_NONE );
    m_pMainPanel->SetPosition( floor( ( Configuration::GetScreenWidth() - panelWidth ) / 2.0f ), 26.0f );
    m_pMainPanel->SetDepth( 100 );
    pGuiManager->AddElement( m_pMainPanel );

    ResourceManager* pRm = FrameWork::GetResourceManager();
    m_pIcon = new Gui::Image();
    m_pIcon->SetPosition( 4.0f, 4.0f );
    m_pIcon->SetSize( 131.0f, 160.0f );
    m_pIcon->SetTexture( nullptr );
    m_pIcon->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
    m_pIcon->SetTexture( (ResourceImage*)pRm->GetResource( "data/ui/portraits/FleetIntelligenceOfficer.jpg" ) );
    m_pIcon->SetShader( FrameWork::GetRenderSystem()->GetShaderCache()->Load( "gui_portrait" ) );
    m_pMainPanel->AddElement( m_pIcon );

    m_pTitle = new Gui::Text();
    m_pTitle->SetSize( panelWidth, panelHeight );
    m_pTitle->SetPosition( 139.0f, 8.0f );
    m_pTitle->SetColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 1.0f ) );
    m_pTitle->SetFont( EVA_FONT_BOLD );
    m_pTitle->SetText( "Fleet Intelligence Officer" );
    m_pMainPanel->AddElement( m_pTitle );

    m_pContent = new Gui::Text();
    m_pContent->SetSize( panelWidth - 139.0f, panelHeight - 28.0f );
    m_pContent->SetPosition( 139.0f, 40.0f );
    m_pContent->SetColour( Genesis::Color( 0.85f, 0.85f, 0.85f, 1.0f ) );
    m_pContent->SetFont( EVA_FONT );
    m_pContent->SetText( "" );
    m_pMainPanel->AddElement( m_pContent );

    m_pNextButton = new TutorialNextButton( this );
    m_pNextButton->SetPosition( 0.0f, 0.0f );
    m_pNextButton->SetSize( panelWidth, 32.0f );
    m_pNextButton->SetColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 0.05f ) );
    m_pNextButton->SetHoverColour( EVA_BUTTON_COLOUR_HOVER );
    m_pNextButton->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
    m_pNextButton->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
    m_pNextButton->SetFont( EVA_FONT );
    m_pNextButton->SetText( "Click to continue" );
    m_pMainPanel->AddElement( m_pNextButton );

    m_pPointOfInterest = new PointOfInterest();
    m_pPointOfInterest->Show( false );
    m_pPointOfInterest->SetPosition( 100.0f, 100.0f );
    pGuiManager->AddElement( m_pPointOfInterest );

    m_LineLength = static_cast<int>( ( panelWidth - m_pContent->GetPosition().x - 8.0f ) / 8.0f );

    m_pSFX = Gui::LoadSFX( "data/sfx/intel.wav" );
}

TutorialWindow::~TutorialWindow()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();
    pGuiManager->RemoveElement( m_pMainPanel );
}

void TutorialWindow::Update( float delta )
{
    if ( m_Delay > 0.0f )
    {
        m_Delay -= delta;
        m_pMainPanel->Show( false );
    }
    else
    {
        m_pMainPanel->Show( !m_Steps.empty() );

        if ( !m_Steps.empty() )
        {
            TutorialStep& step = m_Steps.front();

            if ( m_Dirty )
            {
                SetContent( step.GetContent() );

                if ( step.HasPointOfInterest() )
                {
                    glm::vec2 start(
                        m_pMainPanel->GetPositionAbsolute().x + m_pMainPanel->GetSize().x / 2.0f,
                        m_pMainPanel->GetPositionAbsolute().y + m_pMainPanel->GetSize().y + 1.0f );

                    m_pPointOfInterest->SetStart( start );
                    m_pPointOfInterest->SetEnd( step.GetPointOfInterest() );
                }

                m_pPointOfInterest->Show( step.HasPointOfInterest() );

                Genesis::Gui::PlaySFX( m_pSFX );

                m_Dirty = false;
            }
        }
    }
}

void TutorialWindow::SetContent( const std::string& content )
{
    m_pContent->SetText( content );

    const int numLines = m_pContent->GetLineCount();
    const float lineHeight = m_pContent->GetFont()->GetLineHeight();
    const float contentHeight = std::max( m_pContent->GetPosition().y + numLines * lineHeight, 164.0f );

    m_pNextButton->SetPosition( 0.0f, contentHeight + 8.0f );

    m_pMainPanel->SetHeight( contentHeight + m_pNextButton->GetSize().y + 16.0f );
}

void TutorialWindow::AddStep( const TutorialStep& step )
{
    g_pGame->SetInputBlocked( true );
    m_Steps.push_back( step );
    m_Dirty = true;
}

void TutorialWindow::NextStep()
{
    if ( m_Steps.empty() == false )
    {
        m_Steps.pop_front();
        m_Dirty = true;
        m_Delay = sTutorialStepDelay;

        if ( m_Steps.empty() )
        {
            g_pGame->SetInputBlocked( false );
        }
    }
}

void TutorialWindow::Clear()
{
    m_Steps.clear();
    m_Dirty = true;
    m_Delay = sTutorialStepDelay;
}

///////////////////////////////////////////////////////////////////////////////
// TutorialNextButton
///////////////////////////////////////////////////////////////////////////////

TutorialNextButton::TutorialNextButton( TutorialWindow* pTutorialWindow )
    : m_pTutorialWindow( pTutorialWindow )
{
}

void TutorialNextButton::OnPress()
{
    m_pTutorialWindow->NextStep();
}

} // namespace Hexterminate