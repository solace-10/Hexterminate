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

#include "ship/controller/controller.h"

// clang-format off
#include <beginexternalheaders.h>
#include <glm/vec2.hpp>
#include <endexternalheaders.h>
// clang-format on

namespace Hexterminate
{

class Ship;

///////////////////////////////////////////////////////////////////////////////
// ControllerAI
// Base class for AI controlled ships, implementing shared behaviours
///////////////////////////////////////////////////////////////////////////////

class ControllerAI : public Controller
{
public:
    ControllerAI( Ship* pShip );
    virtual ~ControllerAI(){};
    virtual void Update( float delta );

    inline Ship* GetTargetShip() const;

protected:
    inline float GetMinimumWeaponRange() const;
    inline bool HasWeapons() const;
    bool MoveToPosition( const glm::vec2& position, float goalRadius );
    bool MoveToPosition( const glm::vec2& position, float goalRadius, const glm::vec2& orientationAtGoal );

    virtual void HandleOrders( float delta );

private:
    void AcquireTarget( float delta );
    void FireControl();
    bool PredictTarget( const glm::vec2& src, glm::vec2& result, float projectileSpeed );
    void ManageAddons( float delta );
    int SolveQuadratic( float a, float b, float c, float& solA, float& solB ) const;
    bool IsCourseCorrectionRequired( const glm::vec2& shipForward, const glm::vec2& shipPosition, const glm::vec2& goalPosition, float distanceToGoal ) const;
    void GenerateNextPatrolPoint();
    void ResetAlternatorTimer();

    Ship* m_pTargetShip;
    float m_TargetTimer;
    float m_AccuracyTimer;
    float m_MinimumWeaponRange;
    bool m_PowerToAddons; // At times we need to stop fire to get enough energy to use addons
    bool m_HasWeapons;
    float m_PatrolTimer;
    glm::vec2 m_PatrolPosition;
    float m_AlternatorTimer;
};

inline Ship* ControllerAI::GetTargetShip() const
{
    return m_pTargetShip;
}

inline float ControllerAI::GetMinimumWeaponRange() const
{
    return m_MinimumWeaponRange;
}

inline bool ControllerAI::HasWeapons() const
{
    return m_HasWeapons;
}

} // namespace Hexterminate
