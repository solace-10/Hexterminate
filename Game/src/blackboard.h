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

#include <map>
#include <memory>
#include <string>

// clang-format off
#include <beginexternalheaders.h>
#include <tinyxml2.h>
#include <endexternalheaders.h>
// clang-format on

#include "serialisable.h"

namespace Hexterminate
{

class Blackboard;
typedef std::shared_ptr<Blackboard> BlackboardSharedPtr;
typedef std::weak_ptr<Blackboard> BlackboardWeakPtr;

/////////////////////////////////////////////////////////////////////
// Blackboard
// String to Integer map, meant to hold facts about the game.
// Can be serialised into a save game.
/////////////////////////////////////////////////////////////////////

class Blackboard : public Serialisable
{
public:
    Blackboard();
    void Add( const std::string& text, int value = 1 );
    bool Exists( const std::string& text ) const;
    int Get( const std::string& text ) const;
    void Clear();
    void UpdateDebugUI();

    // Serialisable
    virtual bool Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement ) override;
    virtual bool Read( tinyxml2::XMLElement* pRootElement ) override;
    virtual int GetVersion() const override { return 1; }
    virtual void UpgradeFromVersion( int version ) override {}

private:
    typedef std::map<std::string, int> BlackboardMap;
    BlackboardMap m_Map;
    bool m_DebugUIOpen;
};

} // namespace Hexterminate
