// Copyright 2014 Pedro Nunes
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

namespace Hexterminate
{

class Ship;

///////////////////////////////////////////////////////////////////////////////
// Controller
// The Controller interface is used to control a Ship.
// Subclasses implement the actual behaviour.
///////////////////////////////////////////////////////////////////////////////

class Controller
{
public:
    Controller( Ship* pShip )
        : m_pShip( pShip )
        , m_Suspended( false ){};
    virtual ~Controller(){};
    virtual void Update( float delta ) = 0;

    inline Ship* GetShip() const;
    void Suspend( bool state );
    bool IsSuspended() const;

private:
    Ship* m_pShip;
    bool m_Suspended;
};

inline Ship* Controller::GetShip() const
{
    return m_pShip;
}

inline void Controller::Suspend( bool state )
{
    m_Suspended = state;
}

inline bool Controller::IsSuspended() const
{
    return m_Suspended;
}

} // namespace Hexterminate
