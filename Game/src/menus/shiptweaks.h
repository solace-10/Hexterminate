// Copyright 2018 Pedro Nunes
//
// This file is part of Hexterminate.
//
// Genesis is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Genesis is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Genesis. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <memory>

#include <gui/gui.h>
#include <logger.h>

#include "eva.h"

namespace Hexterminate
{

class ConsoleLogTarget;
class ShipTweaks;

using ShipTweaksUniquePtr = std::unique_ptr<ShipTweaks>;

class ShipTweaks
{
public:
    ShipTweaks();
    ~ShipTweaks();
    void Update( float delta );

    // Rigid body behaviour
    inline float GetLinearDamping() const { return m_LinearDamping; }
    inline float GetAngularDamping() const { return m_AngularDamping; }
    inline float GetEngineThrustMultiplier() const { return m_EngineThrustMultiplier; }
    inline float GetEngineTorqueMultiplier() const { return m_EngineTorqueMultiplier; }

    // Ship navigation: steering feelers, orbits, destinations etc
    inline bool GetDrawNavigation() { return m_DrawNavigation; }

    // Turret tracking logic, aim vectors, targets...
    inline bool GetDrawTurrets() { return m_DrawTurrets; }

    // Sector spawning positions and exclusive areas
    inline bool GetDrawFleetSpawnPositions() { return m_DrawFleetSpawnPositions; }

    // Draw trail geometry
    inline bool GetDrawTrails() { return m_DrawTrails; }

    // Draw debug information for the phase barrier (centre, orbit, dimensions)
    inline bool GetDrawPhaseBarrier() { return m_DrawPhaseBarrier; }

    // Engine disruptor radius and ray tests.
    inline bool GetDrawEngineDisruptor() { return m_DrawEngineDisruptor; }

    // How powerful is the dodge force when using the Evasion Protocols perk.
    inline float GetEvasionProtocolsForceMultiplier() { return m_EvasionProtocolsForceMultiplier; }

    // Global damage multiplier, affecting everyone.
    inline float GetGlobalDamageMultiplier() { return m_GlobalDamageMultiplier; }

    // Player fleet damage multiplier, to offset how powerful the player is against AI.
    inline float GetEmpireDamageMultiplier() { return m_EmpireDamageMultiplier; }

    // Outline
    inline bool GetOutlineUseStencil() { return m_OutlineUseStencil; }
    inline float GetOutlineThickness() { return m_OutlineThickness; }

private:
    bool m_Open;
    float m_LinearDamping;
    float m_AngularDamping;
    float m_EngineThrustMultiplier;
    float m_EngineTorqueMultiplier;
    bool m_DrawNavigation;
    bool m_DrawTurrets;
    bool m_DrawFleetSpawnPositions;
    bool m_DrawTrails;
    bool m_DrawPhaseBarrier;
    bool m_DrawEngineDisruptor;
    float m_EvasionProtocolsForceMultiplier;
    float m_GlobalDamageMultiplier;
    float m_EmpireDamageMultiplier;

    bool m_OutlineUseStencil;
    float m_OutlineThickness;
};

} // namespace Hexterminate
