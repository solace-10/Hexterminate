// Copyright 2020 Pedro Nunes
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

#include <imgui/imgui_impl.h>

namespace Hexterminate
{

/////////////////////////////////////////////////////////////////////
// ShaderTweaksDebugWindow
/////////////////////////////////////////////////////////////////////

class ShaderTweaksDebugWindow
{
public:
    static void Register();
    static void Unregister();
    static void Update();

    static float GetQuantumShieldGeometryScale();
    static float GetQuantumShieldPatternScale();
    static float GetQuantumShieldTriangleGap();
    static float GetQuantumShieldIntensity();
    static bool GetQuantumShieldGlowPass();

    static float GetAntiprotonGeometrySize();
    static float GetAntiprotonInternalRadius();
    static float GetAntiprotonExternalRadius();

private:
    static bool m_Open;

    // Shield tweakables
    static float m_QuantumShieldGeometryScale;
    static float m_QuantumShieldPatternScale;
    static float m_QuantumShieldTriangleGap;
    static float m_QuantumShieldIntensity;
    static bool m_QuantumShieldGlowPass;

    // Antiproton
    static float m_AntiprotonGeometrySize;
    static float m_AntiprotonInternalRadius;
    static float m_AntiprotonExternalRadius;
};

} // namespace Hexterminate