// Copyright 2024 Pedro Nunes
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

namespace Hexterminate
{

class NavigationStats
{
public:
    NavigationStats()
        : m_Mass( 0 )
        , m_LinearThrust( 0.0f )
        , m_Torque( 0.0f )
        , m_MaximumSpeed( 0.0f )
        , m_MaximumAngularSpeed( 0.0f )
    {
    }

    NavigationStats( float mass, float linearThrust, float torque, float maximumSpeed, float maximumAngularSpeed )
        : m_Mass( mass )
        , m_LinearThrust( linearThrust )
        , m_Torque( torque )
        , m_MaximumSpeed( maximumSpeed )
        , m_MaximumAngularSpeed( maximumAngularSpeed )
    {
    }

    ~NavigationStats() {}

    inline float GetMass() const { return m_Mass; }
    inline float GetLinearThrust() const { return m_LinearThrust; }
    inline float GetTorque() const { return m_Torque; }
    inline float GetMaximumSpeed() const { return m_MaximumSpeed; }
    inline float GetMaximumAngularSpeed() const { return m_MaximumAngularSpeed; }

private:
    float m_Mass;
    float m_LinearThrust;
    float m_Torque;
    float m_MaximumSpeed;
    float m_MaximumAngularSpeed;
};

} // namespace Hexterminate