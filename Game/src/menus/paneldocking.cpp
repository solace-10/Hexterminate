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
#include <gui/gui.h>
#include <resources/resourceimage.h>

#include "menus/eva.h"
#include "menus/paneldocking.h"

namespace Hexterminate
{

PanelDocking::PanelDocking()
{
    using namespace Genesis;

    ResourceImage* pBackgroundImage = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/ui/docking_background.png" );
    const int backgroundWidth = static_cast<int>( pBackgroundImage->GetWidth() );
    const int backgroundHeight = static_cast<int>( pBackgroundImage->GetHeight() );
    const int screenWidth = static_cast<int>( Configuration::GetScreenWidth() );
    const int screenHeight = static_cast<int>( Configuration::GetScreenHeight() );

    m_pBackground = new Gui::Image();
    m_pBackground->SetSize( backgroundWidth, backgroundHeight );
    m_pBackground->SetPosition( ( screenWidth - backgroundWidth ) / 2, screenHeight - 200 - backgroundHeight / 2 );
    m_pBackground->SetTexture( pBackgroundImage );
    m_pBackground->SetHiddenForCapture( true );
    m_pBackground->Show( false );
    FrameWork::GetGuiManager()->AddElement( m_pBackground );

    m_pDockingText = new Genesis::Gui::Text();
    m_pDockingText->SetFont( EVA_FONT );
    m_pDockingText->SetColour( EVA_TEXT_COLOUR );
    m_pDockingText->SetSize( 340, 64 );
    m_pDockingText->SetText( "Press 'F' to dock" );
    m_pDockingText->SetPosition( 197, 111 );
    m_pBackground->AddElement( m_pDockingText );
}

PanelDocking::~PanelDocking()
{
    using namespace Genesis;
    Gui::GuiManager* pGuiManager = FrameWork::GetGuiManager();
    if ( pGuiManager != nullptr )
    {
        pGuiManager->RemoveElement( m_pBackground );
    }
}

void PanelDocking::Show( bool state )
{
    m_pBackground->Show( state );
}

} // namespace Hexterminate
