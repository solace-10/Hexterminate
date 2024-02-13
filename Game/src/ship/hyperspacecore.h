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

namespace Hexterminate
{

static const float HyperspaceCoreChargeDuration = 8.0f;
static const float HyperspaceCoreJumpDuration = 3.0f;

class Ship;
class HyperspaceGate;

enum class HyperspaceJumpDirection
{
    JumpInvalid,
    JumpIn,
    JumpOut
};

class HyperspaceCore
{
public:
    HyperspaceCore( Ship* pOwner );
    ~HyperspaceCore();

    void Update( float delta );
    void EnterHyperspace( bool noChargeUp = false );
    void ExitHyperspace();
    bool IsCharging() const;
    float GetChargeTimer() const;
    float GetChargePercentage() const;
    bool IsJumping() const;
    HyperspaceJumpDirection GetJumpDirection() const;
    HyperspaceGate* GetHyperspaceGate() const;

private:
    void BeginJump();
    void EndJump();

    Ship* m_pOwner;
    bool m_Charging;
    bool m_Jumping;
    float m_ChargeTimer;
    HyperspaceJumpDirection m_JumpDirection;
    HyperspaceGate* m_pHyperspaceGate;
    bool m_ChargeupReduced;
};

inline bool HyperspaceCore::IsCharging() const
{
    return m_Charging;
}

inline float HyperspaceCore::GetChargeTimer() const
{
    return m_ChargeTimer;
}

inline bool HyperspaceCore::IsJumping() const
{
    return m_Jumping;
}

inline HyperspaceJumpDirection HyperspaceCore::GetJumpDirection() const
{
    return m_JumpDirection;
}

inline HyperspaceGate* HyperspaceCore::GetHyperspaceGate() const
{
    return m_pHyperspaceGate;
}

} // namespace Hexterminate