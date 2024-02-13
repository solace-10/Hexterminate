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

#include "eva.h"
#include <configuration.h>
#include <genesis.h>
#include <gui/gui.h>

namespace Hexterminate
{

EvaWindow::EvaWindow( int x, int y, unsigned int width, unsigned int height, bool showScanline, int depth /* = 0 */ )
    : mScanLine( nullptr )
{
    using namespace Genesis;
    Gui::GuiManager* guiManager = FrameWork::GetGuiManager();

    mMain = new Gui::Panel();
    mMain->SetSize( (float)width, (float)height );
    mMain->SetPosition( (float)x, (float)y );
    mMain->SetColour( EVA_COLOUR_BACKGROUND );
    mMain->SetBorderColour( EVA_COLOUR_BORDER );
    mMain->SetBorderMode( Gui::PANEL_BORDER_ALL );
    mMain->SetDepth( depth );
    guiManager->AddElement( mMain );

    if ( showScanline )
    {
        ResourceImage* scanLineImage = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/images/scanline.png" );
        mScanLine = new Gui::Image();
        mScanLine->SetPosition( 0.0f, -64.0f );
        mScanLine->SetSize( (float)width, 64.0f );
        mScanLine->SetTexture( scanLineImage );
        mScanLine->SetColour( 1.0f, 1.0f, 1.0f, 0.35f );
        mMain->AddElement( mScanLine );
    }
}

EvaWindow::~EvaWindow()
{
    Genesis::Gui::GuiManager* guiManager = Genesis::FrameWork::GetGuiManager();
    guiManager->RemoveElement( mMain );
}

void EvaWindow::Update( float delta )
{
    if ( mScanLine )
    {
        glm::vec2 scanLinePos = mScanLine->GetPosition();
        scanLinePos.y += 50.0f * delta;
        if ( scanLinePos.y > mMain->GetSize().y )
            scanLinePos.y = -mScanLine->GetSize().y;
        mScanLine->SetPosition( scanLinePos );
    }
}

} // namespace Hexterminate