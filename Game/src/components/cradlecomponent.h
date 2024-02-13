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

#include <component.h>

namespace Hexterminate
{

class Ship;

/////////////////////////////////////////////////////////////////////
// A sector with this component will spawn a "special_cradle" ship
// if that ship hasn't been previously destroyed.
// This also triggers a number of campaign-specific dialogue lines.
/////////////////////////////////////////////////////////////////////

class CradleComponent : public Genesis::Component
{
    DECLARE_COMPONENT( CradleComponent )
public:
    CradleComponent();
    virtual bool Initialise() override;
    virtual void Update( float delta ) override;

private:
    Ship* m_pCradle;
};

} // namespace Hexterminate