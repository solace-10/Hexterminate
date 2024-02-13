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
#include <resources/resourcesound.h>
#include <shadercache.h>

#include "hexterminate.h"
#include "menus/eva.h"
#include "menus/intelwindow.h"
#include "player.h"
#include "ship/hyperspacecore.h"
#include "ship/moduleinfo.h"
#include "ship/ship.h"
#include "stringaux.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// IntelFragment
///////////////////////////////////////////////////////////////////////////////

IntelFragment::IntelFragment( GameCharacter icon, const std::string& title, const std::string& content, ModuleInfo* pModuleInfo, bool canBeQueued )
    : m_Icon( icon )
    , m_Title( title )
    , m_Content( content )
    , m_Duration( 5.0f )
    , m_pModuleInfo( pModuleInfo )
    , m_CanBeQueued( canBeQueued )
{
    m_Title.append( ":" );
}

///////////////////////////////////////////////////////////////////////////////
// IntelWindow
///////////////////////////////////////////////////////////////////////////////

IntelWindow::IntelWindow()
    : m_pMainPanel( nullptr )
    , m_pContent( nullptr )
    , m_pLoot( nullptr )
    , m_pTitle( nullptr )
    , m_pIcon( nullptr )
    , m_LineLength( INT_MAX )
    , m_Dirty( false )
    , m_HoldTimer( 0.0f )
    , m_pSFX( nullptr )
{
    using namespace Genesis;

    ResourceManager* pRm = FrameWork::GetResourceManager();
    for ( unsigned int characterIndex = 0; characterIndex < static_cast<unsigned int>( GameCharacter::Count ); ++characterIndex )
    {
        m_pIcons[ static_cast<unsigned int>( characterIndex ) ] = pRm->GetResource<ResourceImage*>( "data/ui/portraits/Placeholder.jpg" );
    }

    m_pIcons[ static_cast<unsigned int>( GameCharacter::FleetIntelligence ) ] = pRm->GetResource<ResourceImage*>( "data/ui/portraits/FleetIntelligenceOfficer.jpg" );
    m_pIcons[ static_cast<unsigned int>( GameCharacter::NavarreHexer ) ] = pRm->GetResource<ResourceImage*>( "data/ui/portraits/NavarreHexer.jpg" );
    m_pIcons[ static_cast<unsigned int>( GameCharacter::HarkonStormchaser ) ] = pRm->GetResource<ResourceImage*>( "data/ui/portraits/HarkonStormchaser.png" );
    m_pIcons[ static_cast<unsigned int>( GameCharacter::Chrysamere ) ] = pRm->GetResource<ResourceImage*>( "data/ui/portraits/Chrysamere.png" );
    m_pIcons[ static_cast<unsigned int>( GameCharacter::AeliseGloriam ) ] = pRm->GetResource<ResourceImage*>( "data/ui/portraits/AeliseGloriam.png" );

    Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();

    float panelWidth = 600.0f;
    float panelHeight = 168.0f;

    m_pMainPanel = new Gui::Panel();
    m_pMainPanel->SetSize( panelWidth, panelHeight );
    m_pMainPanel->SetColour( EVA_COLOUR_BACKGROUND );
    m_pMainPanel->SetBorderColour( EVA_COLOUR_BORDER );
    m_pMainPanel->SetBorderMode( Gui::PANEL_BORDER_NONE );
    m_pMainPanel->SetPosition( ( Configuration::GetScreenWidth() - panelWidth ) / 2.0f, 8.0f );
    m_pMainPanel->SetHiddenForCapture( true );
    m_pMainPanel->SetDepth( 100 );
    pGuiManager->AddElement( m_pMainPanel );

    m_pTitle = new Gui::Text();
    m_pTitle->SetSize( panelWidth, panelHeight );
    m_pTitle->SetPosition( 139.0f, 8.0f );
    m_pTitle->SetColour( EVA_TEXT_COLOUR );
    m_pTitle->SetFont( EVA_FONT_BOLD );
    m_pTitle->SetText( "" );
    m_pMainPanel->AddElement( m_pTitle );

    m_pContent = new Gui::Text();
    m_pContent->SetSize( panelWidth - 144.0f, panelHeight - 48.0f );
    m_pContent->SetPosition( 139.0f, 40.0f );
    m_pContent->SetColour( EVA_TEXT_COLOUR );
    m_pContent->SetFont( EVA_FONT );
    m_pContent->SetText( "" );
    m_pMainPanel->AddElement( m_pContent );

    m_pLoot = new Gui::Text();
    m_pLoot->SetSize( panelWidth, panelHeight );
    m_pLoot->SetPosition( panelWidth / 2.0f, panelHeight / 2.0f );
    m_pLoot->SetColour( EVA_TEXT_COLOUR );
    m_pLoot->SetFont( EVA_FONT );
    m_pLoot->SetText( "" );
    m_pMainPanel->AddElement( m_pLoot );

    m_pIcon = new Gui::Image();
    m_pIcon->SetPosition( 4.0f, 4.0f );
    m_pIcon->SetSize( 131.0f, 160.0f );
    m_pIcon->SetTexture( nullptr );
    m_pIcon->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
    m_pIcon->SetShader( FrameWork::GetRenderSystem()->GetShaderCache()->Load( "gui_portrait" ) );
    m_pMainPanel->AddElement( m_pIcon );

    m_LineLength = static_cast<int>( ( panelWidth - m_pContent->GetPosition().x - 8.0f ) / 8.0f );

    m_pSFX = Gui::LoadSFX( "data/sfx/intel.wav" );
}

IntelWindow::~IntelWindow()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();
    pGuiManager->RemoveElement( m_pMainPanel );
}

void IntelWindow::Update( float delta )
{
    // Adjust the vertical position of the window depending on whether we are in the galaxy view or in a sector.
    m_pMainPanel->SetPosition( ( Genesis::Configuration::GetScreenWidth() - m_pMainPanel->GetWidth() ) / 2.0f, g_pGame->GetCurrentSector() ? 8.0f : 26.0f );

    bool show = true;
    if ( m_Fragments.empty() || m_HoldTimer > 0.0f )
    {
        show = false;
        m_HoldTimer -= delta;
    }
    else if ( g_pGame->GetPlayer() != nullptr && g_pGame->GetPlayer()->GetShip() != nullptr )
    {
        HyperspaceCore* pCore = g_pGame->GetPlayer()->GetShip()->GetHyperspaceCore();
        SDL_assert( pCore != nullptr );
        if ( pCore->IsCharging() || ( pCore->IsJumping() && pCore->GetJumpDirection() == HyperspaceJumpDirection::JumpOut ) )
        {
            show = false;
        }
    }

    m_pMainPanel->Show( show );

    if ( !m_Fragments.empty() && m_HoldTimer <= 0.0f )
    {
        IntelFragment& fragment = m_Fragments.front();

        if ( m_Dirty )
        {
            m_pTitle->SetText( fragment.m_Title );
            m_pContent->SetText( fragment.m_Content );
            m_pIcon->SetTexture( m_pIcons[ (int)fragment.m_Icon ] );

            // Dynamic on-screen duration of the fragment is dependent on the amount of text in it.
            fragment.m_Duration = std::max( 5.0f, (float)m_pContent->GetLineCount() * 3.0f );

            ModuleInfo* pModuleInfo = fragment.m_pModuleInfo;
            if ( pModuleInfo != nullptr )
            {
                m_pLoot->SetText( pModuleInfo->GetFullName() );
                m_pLoot->SetColour( ModuleRarityToColour( pModuleInfo->GetRarity() ) );

                float halfTextWidth = m_pLoot->GetFont()->GetTextLength( m_pLoot->GetText() ) / 2.0f;
                float halfTextHeight = m_pLoot->GetFont()->GetLineHeight() / 2.0f;
                m_pLoot->SetPosition(
                    floorf( m_pContent->GetPosition().x + m_pContent->GetSize().x / 2.0f - halfTextWidth ),
                    floorf( m_pContent->GetPosition().y + m_pContent->GetSize().y / 2.0f - halfTextHeight ) );

                m_pLoot->Show( true );
            }
            else
            {
                m_pLoot->Show( false );
            }

            Genesis::Gui::PlaySFX( m_pSFX );

            m_Dirty = false;
        }

        if ( fragment.m_Duration <= 0.0f )
        {
            m_Fragments.pop_front();
            m_Dirty = true;
            m_HoldTimer = 0.4f;
        }
        else if ( g_pGame->IsPaused() == false )
        {
            fragment.m_Duration -= delta;
        }
    }
}

void IntelWindow::AddFragment( const IntelFragment& fragment )
{
    Player* pPlayer = g_pGame->GetPlayer();
    if ( pPlayer != nullptr && pPlayer->GetShip() != nullptr && pPlayer->GetShip()->IsDestroyed() )
    {
        return;
    }

    // If this fragment can't be queued and we're already displaying something, ignore it.
    if ( m_Fragments.empty() == false && fragment.m_CanBeQueued == false )
    {
        return;
    }

    // Avoid duplicates
    if ( m_Fragments.empty() == false )
    {
        if ( m_Fragments.back().m_Content == fragment.m_Content && m_Fragments.back().m_Title == fragment.m_Title )
        {
            return;
        }
    }

    m_Fragments.push_back( fragment );
    m_Dirty = true;
}

void IntelWindow::Clear()
{
    m_Fragments.clear();
    m_Dirty = true;
}

bool IntelWindow::IsVisible() const
{
    return m_pMainPanel->IsVisible();
}

const glm::vec2& IntelWindow::GetPosition() const
{
    return m_pMainPanel->GetPosition();
}

const glm::vec2& IntelWindow::GetSize() const
{
    return m_pMainPanel->GetSize();
}

} // namespace Hexterminate