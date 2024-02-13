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

#pragma once

#include <gui/gui.h>

namespace Hexterminate
{

#define EVA_COLOUR_BACKGROUND Genesis::Color( 0.00f, 0.00f, 0.00f, 0.50f )
#define EVA_COLOUR_BORDER Genesis::Color( 1.00f, 1.00f, 1.00f, 0.50f )
#define EVA_BUTTON_COLOUR_BACKGROUND Genesis::Color( 0.00f, 0.00f, 0.00f, 0.30f )
#define EVA_BUTTON_COLOUR_HOVER Genesis::Color( 1.00f, 1.00f, 1.00f, 0.10f )
#define EVA_BUTTON_COLOUR_BORDER Genesis::Color( 0.68f, 0.72f, 0.82f, 1.00f )
#define EVA_BUTTON_ICON_COLOUR Genesis::Color( 1.00f, 1.00f, 1.00f, 0.80f )
#define EVA_BUTTON_ICON_COLOUR_HOVER Genesis::Color( 1.00f, 1.00f, 1.00f, 1.00f )
#define EVA_COLOUR_TITLEBAR_BACKGROUND Genesis::Color( 0.68f, 0.72f, 0.82f, 0.60f )
#define EVA_TEXT_COLOUR Genesis::Color( 1.00f, 1.00f, 1.00f, 0.80f )
#define EVA_TEXT_COLOUR_DISABLED Genesis::Color( 0.40f, 0.40f, 0.40f, 0.40f )
#define EVA_CHECKBOX_BULLET_COLOUR Genesis::Color( 0.00f, 1.00f, 1.00f, 0.80f )

#define EVA_FONT (Genesis::ResourceFont*)Genesis::FrameWork::GetResourceManager()->GetResource( "data/fonts/fixedsys16.fnt" )
#define EVA_FONT_BOLD (Genesis::ResourceFont*)Genesis::FrameWork::GetResourceManager()->GetResource( "data/fonts/fixedsys16bold.fnt" )

class EvaWindow
{
public:
    EvaWindow( int x, int y, unsigned int width, unsigned int height, bool showScanline, int depth = 0 );
    ~EvaWindow();
    void Update( float delta );

    Genesis::Gui::Panel* GetMainPanel() const;

private:
    Genesis::Gui::Panel* mMain;
    Genesis::Gui::Image* mScanLine;
};

inline Genesis::Gui::Panel* EvaWindow::GetMainPanel() const
{
    return mMain;
}

} // namespace Hexterminate
