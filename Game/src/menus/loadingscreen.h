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

#pragma once

#include <gui/gui.h>
#include <memory>

namespace Hexterminate
{

class LoadingScreen
{
public:
    LoadingScreen();
    ~LoadingScreen();
    void Show( bool state );
    void SetProgress( size_t current, size_t maximum );
    void Update();

private:
    Genesis::Gui::Image* m_pBackground;
    Genesis::Gui::Panel* m_pProgressBorderPanel;
    Genesis::Gui::Panel* m_pProgressPanel;
    size_t m_CurrentProgress;
    size_t m_MaximumProgress;
    int m_ProgressMaxWidth;
};

typedef std::unique_ptr<LoadingScreen> LoadingScreenUniquePtr;

} // namespace Hexterminate