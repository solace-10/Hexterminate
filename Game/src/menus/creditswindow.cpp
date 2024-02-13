// Copyright 2021 Pedro Nunes
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

#include "menus/creditswindow.h"
#include "ui/panel.h"
#include "ui/text.h"

namespace Hexterminate
{

CreditsWindow::CreditsWindow()
    : UI::Window( "Credits window" )
    , m_Index( 0 )
    , m_Timer( 0.0f )
{
    m_pText = std::make_shared<UI::Text>( "Text" );
    GetContentPanel()->Add( m_pText );
}

void CreditsWindow::Update()
{
    static const std::string sText = "DEVELOPER ..................... Pedro Nunes / Wings of Steel\n"
                                     "\n"
                                     "SOUNDTRACK ...................................... Marc Chait\n"
                                     "CHARACTER PORTRAITS ....................... Trung Tin Shinji\n"
                                     "ICONS ......................................... Tyrone Swart\n"
                                     "\n"
                                     "SOUND EFFECTS\n"
                                     "................................................ Robert Fenn\n"
                                     "................................. Little Robot Sound Factory\n"
                                     "........................ Kantouth, klankbeeld & duckduckpony\n"
                                     "\n"
                                     "Particular thanks to my wife, Leslie Glass, for all her love\n"
                                     "and support even throughout debugging race conditions.\n"
                                     "\n"
                                     "And thanks to you, the player, for playing my game!";

    static const std::size_t sTextLength = sText.length();
    static const float sDelay = 0.02f;
    static const float sDelta = 0.033f;
    const bool finished = ( m_Index >= sTextLength );
    m_Timer += sDelta;
    if ( !finished && m_Timer > sDelay )
    {
        m_Index++;
        m_Timer -= sDelay;

        if ( m_Index > sTextLength )
        {
            m_Index = sTextLength;
        }
    }

    if ( finished )
    {
        m_pText->SetText( sText );
    }
    else
    {
        m_pText->SetText( sText.substr( 0, m_Index ) + "_" );
    }
}

void CreditsWindow::Reset()
{
    UI::Window::Reset();
    m_Index = 0;
    m_Timer = 0.0f;
}

} // namespace Hexterminate
