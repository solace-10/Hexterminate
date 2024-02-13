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

#include "globals.h"
#include "hexterminate.h"
#include "ship/ship.h"
#include <gui/gui.h>
#include <list>

namespace Hexterminate
{

class ModuleInfo;

class IntelFragment
{
public:
    IntelFragment( GameCharacter character, const std::string& title, const std::string& content, ModuleInfo* pModuleInfo = nullptr, bool canBeQueued = true );
    ~IntelFragment(){};

    GameCharacter m_Icon;
    std::string m_Title;
    std::string m_Content;
    float m_Duration;
    ModuleInfo* m_pModuleInfo;
    bool m_CanBeQueued;
};

typedef std::list<IntelFragment> IntelFragmentList;

class IntelWindow
{
public:
    IntelWindow();
    ~IntelWindow();
    void Update( float delta );

    void AddFragment( const IntelFragment& fragment );
    void Clear();
    bool IsVisible() const;
    const glm::vec2& GetPosition() const;
    const glm::vec2& GetSize() const;

private:
    Genesis::Gui::Panel* m_pMainPanel;
    Genesis::Gui::Text* m_pContent;
    Genesis::Gui::Text* m_pLoot;
    Genesis::Gui::Text* m_pTitle;
    Genesis::Gui::Image* m_pIcon;

    Genesis::ResourceImage* m_pIcons[ (int)GameCharacter::Count ];
    IntelFragmentList m_Fragments;

    int m_LineLength;
    bool m_Dirty;
    float m_HoldTimer;

    Genesis::ResourceSound* m_pSFX;
};

} // namespace Hexterminate