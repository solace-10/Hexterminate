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

#include "menus/popup.h"
#include "hexterminate.h"
#include <configuration.h>
#include <genesis.h>
#include <sstream>

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// Popup
///////////////////////////////////////////////////////////////////////////////

Popup::Popup()
    : m_pWindow( nullptr )
    , m_pText( nullptr )
    , m_pBackground( nullptr )
    , m_State( PopupState::Undefined )
    , m_Mode( PopupMode::Undefined )
    , m_InputTimer( 0.0f )
    , m_PreviousCursorType( CursorType::Pointer )
{
    unsigned int menuWidth = Genesis::Configuration::GetScreenWidth();
    unsigned int menuHeight = 72;
    unsigned int menuX = 0;
    unsigned int menuY = Genesis::Configuration::GetScreenHeight() / 2 - menuHeight / 2;
    unsigned int centreX = Genesis::Configuration::GetScreenWidth() / 2;

    m_pBackground = new Genesis::Gui::Panel();
    m_pBackground->SetSize( static_cast<float>( Genesis::Configuration::GetScreenWidth() ), static_cast<float>( Genesis::Configuration::GetScreenHeight() ) );
    m_pBackground->SetPosition( 0.0f, 0.0f );
    m_pBackground->SetColour( 0.0f, 0.0f, 0.0f, 0.5f );
    m_pBackground->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
    m_pBackground->Show( false );
    m_pBackground->SetDepth( 9999 );
    Genesis::FrameWork::GetGuiManager()->AddElement( m_pBackground );

    m_pWindow = new EvaWindow( menuX, menuY, menuWidth, menuHeight, true, 10000 );
    m_pWindow->GetMainPanel()->Show( false );
    m_pWindow->GetMainPanel()->SetBorderMode( Genesis::Gui::PANEL_BORDER_BOTTOM | Genesis::Gui::PANEL_BORDER_TOP );

    m_pText = new Genesis::Gui::Text();
    m_pText->SetFont( EVA_FONT );
    m_pText->SetColour( EVA_TEXT_COLOUR );
    m_pText->SetSize( Genesis::Configuration::GetScreenWidth(), 64 );
    m_pText->SetPosition( 0, 8 );
    m_pWindow->GetMainPanel()->AddElement( m_pText );

    for ( int i = 0; i < 2; ++i )
    {
        m_pButton[ i ] = new ButtonPopup( this );
        m_pButton[ i ]->SetPosition( centreX - 128.0f - 4.0f, 32.0f );
        m_pButton[ i ]->SetSize( 128.0f, 32.0f );
        m_pButton[ i ]->SetColour( Genesis::Color( 1.0f, 1.0f, 1.0f, 0.05f ) );
        m_pButton[ i ]->SetHoverColour( EVA_BUTTON_COLOUR_HOVER );
        m_pButton[ i ]->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
        m_pButton[ i ]->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
        m_pButton[ i ]->SetFont( EVA_FONT );
        m_pButton[ i ]->SetCallbackState( PopupState::Undefined );
        m_pWindow->GetMainPanel()->AddElement( m_pButton[ i ] );
    }
}

Popup::~Popup()
{
    delete m_pWindow;
    Genesis::FrameWork::GetGuiManager()->RemoveElement( m_pBackground );
}

void Popup::Update( float delta )
{
    if ( m_InputTimer > 0.0f )
    {
        m_InputTimer -= delta;
    }
}

void Popup::Show( PopupMode mode, const std::string& text )
{
    m_Mode = mode;
    m_State = PopupState::Displayed;

    if ( mode == PopupMode::OkCancel )
    {
        m_pButton[ 0 ]->SetCallbackState( PopupState::Ok );
        m_pButton[ 1 ]->SetCallbackState( PopupState::Cancel );
    }
    else if ( mode == PopupMode::Ok )
    {
        m_pButton[ 0 ]->SetCallbackState( PopupState::Ok );
        m_pButton[ 1 ]->SetCallbackState( PopupState::Undefined );
    }
    else if ( mode == PopupMode::YesNo )
    {
        m_pButton[ 0 ]->SetCallbackState( PopupState::Yes );
        m_pButton[ 1 ]->SetCallbackState( PopupState::No );
    }

    m_pText->SetText( text );
    AlignToCentre( m_pText );

    RepositionButtons();

    m_pWindow->GetMainPanel()->Show( true );
    m_pBackground->Show( true );

    m_PreviousCursorType = g_pGame->GetCursorType();

    g_pGame->Pause();
}

void Popup::Close()
{
    m_pWindow->GetMainPanel()->Show( false );
    m_pBackground->Show( false );
    m_InputTimer = 0.2f;
    g_pGame->Unpause();
}

void Popup::AlignToCentre( Genesis::Gui::Text* pText )
{
    unsigned int centreX = Genesis::Configuration::GetScreenWidth() / 2;
    pText->SetPosition( floorf( centreX - pText->GetFont()->GetTextLength( pText->GetText() ) / 2.0f ), pText->GetPosition().y );
}

void Popup::SetState( PopupState state )
{
    m_State = state;
}

void Popup::RepositionButtons()
{
    float centreX = static_cast<float>( Genesis::Configuration::GetScreenWidth() / 2 );
    if ( m_Mode == PopupMode::OkCancel || m_Mode == PopupMode::YesNo )
    {
        m_pButton[ 0 ]->SetPosition( centreX - 128.0f - 4.0f, 32.0f );
        m_pButton[ 1 ]->SetPosition( centreX + 4.0f, 32.0f );

        m_pButton[ 0 ]->Show( true );
        m_pButton[ 1 ]->Show( true );
    }
    else if ( m_Mode == PopupMode::Ok )
    {
        m_pButton[ 0 ]->SetPosition( centreX - 64.0f, 32.0f );

        m_pButton[ 0 ]->Show( true );
        m_pButton[ 1 ]->Show( false );
    }
}

///////////////////////////////////////////////////////////////////////////////
// ButtonPopup
///////////////////////////////////////////////////////////////////////////////

ButtonPopup::ButtonPopup( Popup* pOwner )
    : m_pOwner( pOwner )
    , m_CallbackState( PopupState::Undefined )
{
}

void ButtonPopup::OnPress()
{
    m_pOwner->SetState( m_CallbackState );
    m_pOwner->Close();
}

void ButtonPopup::SetCallbackState( PopupState state )
{
    m_CallbackState = state;

    if ( m_CallbackState == PopupState::Ok )
    {
        SetText( "Ok" );
    }
    else if ( m_CallbackState == PopupState::Cancel )
    {
        SetText( "Cancel" );
    }
    else if ( m_CallbackState == PopupState::Yes )
    {
        SetText( "Yes" );
    }
    else if ( m_CallbackState == PopupState::No )
    {
        SetText( "No" );
    }
    else
    {
        SetText( "" );
    }
}

} // namespace Hexterminate