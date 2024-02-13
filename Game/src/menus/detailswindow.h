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

#pragma once

#include "../globals.h"
#include "../ship/ship.h"
#include <gui/gui.h>

namespace Hexterminate
{

class DetailsWindow
{
public:
    DetailsWindow();
    virtual ~DetailsWindow();
    virtual void Update( float delta );
    virtual void Show( bool state );
    void SetContents( const std::string& contents );
    virtual void SetAnchor( float x, float y );

protected:
    virtual bool UsesDynamicLineLength() const { return false; }
    inline int GetNumLines() const { return m_NumLines; }

    Genesis::Gui::Panel* m_pMainPanel;
    Genesis::Gui::Text* m_pText;
    float m_AnchorX;
    float m_AnchorY;

private:
    int m_NumLines;
};

} // namespace Hexterminate