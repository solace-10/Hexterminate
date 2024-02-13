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

#pragma once

#include <memory>

#include "hyperscape/hyperscapelocation.h"

#include "serialisable.h"

namespace Hexterminate
{

class HyperscapeRep;
class SilverThread;
class Starfield;

//-----------------------------------------------------------------------------
// Hyperscape
//-----------------------------------------------------------------------------

class Hyperscape : public Serialisable
{
public:
    Hyperscape();
    ~Hyperscape();

    void Update( float delta );
    void Show( bool state );
    HyperscapeRep* GetRepresentation() const;
    bool IsVisible() const;

    // Serialisable
    virtual bool Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement ) override;
    virtual bool Read( tinyxml2::XMLElement* pRootElement ) override;
    virtual int GetVersion() const override;
    virtual void UpgradeFromVersion( int version ) override;

private:
    void UpdateDebugUI();

    std::unique_ptr<HyperscapeRep> m_pRep;
    std::unique_ptr<SilverThread> m_pSilverThread;
    std::unique_ptr<Starfield> m_pStarfield;
    bool m_DebugWindowOpen;
};

inline HyperscapeRep* Hyperscape::GetRepresentation() const
{
    return m_pRep.get();
}

} // namespace Hexterminate
