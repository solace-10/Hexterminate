// Copyright 2022 Pedro Nunes
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

#include "sound/window.h"

#include "beginexternalheaders.h"

#include "endexternalheaders.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl.h"
#include "resources/resourceplaylist.h"
#include "resources/resourcesound.h"
#include "sound/soundinstance.h"
#include "sound/soundmanager.h"
#include "genesis.h"

namespace Genesis::Sound
{

Window::Window(SoundManager* pSoundManager)
	: m_pSoundManager(pSoundManager)
	, m_Open(false)
{
	Genesis::ImGuiImpl::RegisterMenu( "Genesis", "Sound manager", &m_Open );
}

Window::~Window()
{
	Genesis::ImGuiImpl::UnregisterMenu( "Genesis", "Sound manager" );
}

void Window::Update( float delta )
{
	if ( m_Open )
	{
		ImGui::Begin( "Sound manager", &m_Open );

		if (ImGui::CollapsingHeader("Overview", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Active voices: %u / %u", m_pSoundManager->GetActiveSoundCount(), m_pSoundManager->GetMaximumSoundCount());
			ImGui::Text("Virtual voices: %u", m_pSoundManager->GetVirtualSoundCount());
		}

		if (ImGui::CollapsingHeader("Playlist", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ResourcePlaylist* pPlaylist = m_pSoundManager->GetPlaylist();
			if ( pPlaylist )
			{
				ImGui::Text( "Playlist: %s", pPlaylist->GetFilename().GetFullPath().c_str() );
				const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
				if ( ImGui::BeginTable( "PlaylistTable", 2, flags ) )
				{
					ImGui::TableSetupColumn( "Track" );
					ImGui::TableSetupColumn( "State" );
					ImGui::TableHeadersRow();

					SoundInstanceSharedPtr pCurrentTrack = m_pSoundManager->GetCurrentTrack();
					for ( auto& pTrack : pPlaylist->GetTracks() )
					{
						ImGui::TableNextColumn();
						ImGui::Text( "%s", pTrack->GetFilename().GetName().c_str() );
						ImGui::TableNextColumn();
						if ( pCurrentTrack != nullptr && pCurrentTrack->IsValid() && pCurrentTrack->GetResource() == pTrack )
						{
							ImGui::Text( "%s", "Playing" );
						}
						else
						{
							ImGui::Text( "%s", " " );
						}
					}

					ImGui::EndTable();
				}
			}
			else
			{
				ImGui::TextDisabled( "%s", "No playlist set." );
			}
		}

		if (ImGui::CollapsingHeader("SFX", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
			if ( ImGui::BeginTable( "SFXTable", 3, flags ) )
			{
				ImGui::TableSetupColumn( "Resource" );
				ImGui::TableSetupColumn("Paused");
				ImGui::TableSetupColumn( "Volume" );
				ImGui::TableHeadersRow();

				for (auto& pInstance : m_pSoundManager->GetSoundInstances())
				{
					ImGui::TableNextColumn();
					ImGui::Text("%s", pInstance->GetResource()->GetFilename().GetName().c_str());
					ImGui::TableNextColumn();
					ImGui::Text("%s", pInstance->IsPaused() ? "True" : "False");
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", pInstance->GetVolume());
				}

				ImGui::EndTable();
			}
		}

		ImGui::End();
	}
}

} // namespace Genesis::Sound
