// Copyright 2018 Pedro Nunes
//
// This file is part of Genesis.
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

#include "beginexternalheaders.h"
#include <btBulletDynamicsCommon.h>
#include "endexternalheaders.h"

#include "genesis.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl.h"
#include "physics/debugrender.h"
#include "physics/shape.h"
#include "physics/simulation.h"
#include "window.h"

namespace Genesis
{
namespace Physics
{

Window::Window( Simulation* pSimulation ) :
m_pSimulation( pSimulation ),
m_Open( false ),
m_ModeWireframe( false ),
m_ModeAABB( false ),
m_ModeTransforms( false ),
m_ModeRayTests( false ),
m_ModeContactPoints( false )
{
	Genesis::ImGuiImpl::RegisterMenu( "Tools", "Physics", &m_Open );
}

Window::~Window()
{
	Genesis::ImGuiImpl::UnregisterMenu( "Tools", "Physics" );
}

void Window::Update( float delta )
{
	if ( m_Open )
	{
		ImGui::Begin( "Window", &m_Open );

		if ( ImGui::CollapsingHeader( "Simulation statistics" ) )
		{
			ImGui::Columns( 2 );

			ImGui::Text( "btCollisionObject" ); ImGui::NextColumn();
			ImGui::Text( "%d", m_pSimulation->m_pWorld->getCollisionWorld()->getNumCollisionObjects() ); ImGui::NextColumn();

			ImGui::Text( "btCollisionShape" ); ImGui::NextColumn();
			ImGui::Text( "%d", sActiveShapes ); ImGui::NextColumn();

			ImGui::Text( "RigidBody" ); ImGui::NextColumn();
			ImGui::Text( "%zu", m_pSimulation->m_RigidBodies.size() ); ImGui::NextColumn();

			ImGui::Text( "Ghost" ); ImGui::NextColumn();
			ImGui::Text( "%zu", m_pSimulation->m_Ghosts.size() ); ImGui::NextColumn();

			ImGui::Columns( 1 );
		}

		if ( ImGui::CollapsingHeader( "Debug render" ) )
		{
			ImGui::Checkbox( "AABB", &m_ModeAABB );
			ImGui::Checkbox( "Contact points", &m_ModeContactPoints );
			ImGui::Checkbox( "Ray tests", &m_ModeRayTests );
			ImGui::Checkbox( "Transforms", &m_ModeTransforms );
			ImGui::Checkbox( "Wireframe", &m_ModeWireframe );
		}
		ImGui::End();
	}

	DebugRender* pDebugRender = m_pSimulation->m_pDebugRender;
	pDebugRender->SetEnabled( DebugRender::Mode::Wireframe, m_ModeWireframe );
	pDebugRender->SetEnabled( DebugRender::Mode::AABB, m_ModeAABB );
	pDebugRender->SetEnabled( DebugRender::Mode::Transforms, m_ModeTransforms );
	pDebugRender->SetEnabled( DebugRender::Mode::RayTests, m_ModeRayTests );
	pDebugRender->SetEnabled( DebugRender::Mode::ContactPoints, m_ModeContactPoints );
}

} // namespace Physics
} // namespace Genesis