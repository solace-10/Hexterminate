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

#include <genesis.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl.h>

#include "hexterminate.h"
#include "ship/shipoutline.h"
#include "shiptweaks.h"

namespace Hexterminate
{

ShipTweaks::ShipTweaks()
    : m_Open( false )
    , m_LinearDamping( 0.6f )
    , m_AngularDamping( 0.6f )
    , m_EngineThrustMultiplier( 40.0f )
    , m_EngineTorqueMultiplier( 28.0f )
    , m_DrawNavigation( false )
    , m_DrawTurrets( false )
    , m_DrawFleetSpawnPositions( false )
    , m_DrawTrails( false )
    , m_DrawPhaseBarrier( false )
    , m_DrawEngineDisruptor( false )
    , m_EvasionProtocolsForceMultiplier( 2.0f )
    , m_GlobalDamageMultiplier( 1.6f )
    , m_EmpireDamageMultiplier( 1.75f )
    , m_OutlineThickness( 2.0f )
{
    Genesis::ImGuiImpl::RegisterMenu( "Sector", "Ship tweaks", &m_Open );
}

ShipTweaks::~ShipTweaks()
{
    Genesis::ImGuiImpl::UnregisterMenu( "Sector", "Ship tweaks" );
}

void ShipTweaks::Update( float delta )
{
#ifdef _DEBUG
    if ( m_Open )
    {
        ImGui::SetNextWindowSize( ImVec2( 600.0f, 500.0f ) );
        ImGui::Begin( "Ship tweaks", &m_Open );

        if ( ImGui::CollapsingHeader( "Ship damage" ) )
        {
            ImGui::SliderFloat( "Global damage multiplier", &m_GlobalDamageMultiplier, 0.5f, 5.0f );
            ImGui::SliderFloat( "Empire damage multiplier", &m_EmpireDamageMultiplier, 0.5f, 5.0f );
        }

        if ( ImGui::CollapsingHeader( "Ship movement" ) )
        {
            ImGui::SliderFloat( "Linear damping", &m_LinearDamping, 0.0f, 1.0f );
            ImGui::SliderFloat( "Angular damping", &m_AngularDamping, 0.0f, 1.0f );
            ImGui::SliderFloat( "Engine thrust multiplier", &m_EngineThrustMultiplier, 1.0f, 100.0f );
            ImGui::SliderFloat( "Engine torque multiplier", &m_EngineTorqueMultiplier, 1.0f, 100.0f );
            ImGui::SliderFloat( "Evasion protocols multiplier", &m_EvasionProtocolsForceMultiplier, 1.0f, 10.0f );
        }

        if ( ImGui::CollapsingHeader( "Debug render" ) )
        {
            ImGui::Checkbox( "Fleet spawn positions", &m_DrawFleetSpawnPositions );
            ImGui::Checkbox( "Navigation", &m_DrawNavigation );
            ImGui::Checkbox( "Phase barrier", &m_DrawPhaseBarrier );
            ImGui::Checkbox( "Trails", &m_DrawTrails );
            ImGui::Checkbox( "Turrets", &m_DrawTurrets );
            ImGui::Checkbox( "Engine disruptor", &m_DrawEngineDisruptor );
        }

        g_pGame->GetShipOutline()->DrawShipOutlineTweaks();

        ImGui::End();
    }
#endif
}

} // namespace Hexterminate