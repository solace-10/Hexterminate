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

#include "shadertweaks.h"
#include <imgui/imgui.h>

namespace Hexterminate
{

/////////////////////////////////////////////////////////////////////
// ShaderTweaksDebugWindow
/////////////////////////////////////////////////////////////////////

bool ShaderTweaksDebugWindow::m_Open = false;

float ShaderTweaksDebugWindow::m_QuantumShieldGeometryScale = 1.05f;
float ShaderTweaksDebugWindow::m_QuantumShieldPatternScale = 5.0f;
float ShaderTweaksDebugWindow::m_QuantumShieldTriangleGap = 0.01f;
float ShaderTweaksDebugWindow::m_QuantumShieldIntensity = 2.5f;
bool ShaderTweaksDebugWindow::m_QuantumShieldGlowPass = false;
float ShaderTweaksDebugWindow::m_AntiprotonGeometrySize = 32.0f;
float ShaderTweaksDebugWindow::m_AntiprotonInternalRadius = 0.05f;
float ShaderTweaksDebugWindow::m_AntiprotonExternalRadius = 1.0f;

void ShaderTweaksDebugWindow::Register()
{
    Genesis::ImGuiImpl::RegisterMenu( "Tools", "Shader tweaks", &m_Open );
}

void ShaderTweaksDebugWindow::Unregister()
{
    Genesis::ImGuiImpl::UnregisterMenu( "Tools", "Shader tweaks" );
}

void ShaderTweaksDebugWindow::Update()
{
    using namespace Genesis;
    if ( m_Open )
    {
        ImGui::SetNextWindowSize( ImVec2( 400.0f, 400.0f ) );
        ImGui::Begin( "Shader tweaks", &m_Open );

        if ( ImGui::CollapsingHeader( "Quantum shield" ) )
        {
            ImGui::SliderFloat( "Geometry scale", &m_QuantumShieldGeometryScale, 0.5f, 2.0f );
            ImGui::SliderFloat( "Pattern scale", &m_QuantumShieldPatternScale, 1.0f, 25.0f );
            ImGui::SliderFloat( "Triangle gap", &m_QuantumShieldTriangleGap, -1.0f, 1.0f );
            ImGui::SliderFloat( "Intensity", &m_QuantumShieldIntensity, 0.0f, 8.0f );
            ImGui::Checkbox( "Glow pass", &m_QuantumShieldGlowPass );
        }

        if ( ImGui::CollapsingHeader( "Antiproton" ) )
        {
            ImGui::SliderFloat( "Geometry size", &m_AntiprotonGeometrySize, 8.0f, 128.0f );
            ImGui::SliderFloat( "Internal radius", &m_AntiprotonInternalRadius, 0.0f, 1.0f );
            ImGui::SliderFloat( "External radius", &m_AntiprotonExternalRadius, 0.0f, 2.0f );
        }

        ImGui::End();
    }
}

float ShaderTweaksDebugWindow::GetQuantumShieldGeometryScale()
{
    return m_QuantumShieldGeometryScale;
}

float ShaderTweaksDebugWindow::GetQuantumShieldPatternScale()
{
    return m_QuantumShieldPatternScale;
}

float ShaderTweaksDebugWindow::GetQuantumShieldTriangleGap()
{
    return m_QuantumShieldTriangleGap;
}

float ShaderTweaksDebugWindow::GetQuantumShieldIntensity()
{
    return m_QuantumShieldIntensity;
}

bool ShaderTweaksDebugWindow::GetQuantumShieldGlowPass()
{
    return m_QuantumShieldGlowPass;
}

float ShaderTweaksDebugWindow::GetAntiprotonGeometrySize()
{
    return m_AntiprotonGeometrySize;
}

float ShaderTweaksDebugWindow::GetAntiprotonInternalRadius()
{
    return m_AntiprotonInternalRadius;
}

float ShaderTweaksDebugWindow::GetAntiprotonExternalRadius()
{
    return m_AntiprotonExternalRadius;
}

} // namespace Hexterminate