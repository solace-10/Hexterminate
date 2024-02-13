// Copyright 2017 Pedro Nunes
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

#include <array>

#include <genesis.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl.h>

#include "achievements.h"

namespace Hexterminate
{

#define _ACH_ID( id, name ) Achievement_t( { id, #id, name, false } )

std::array<Achievement_t, 17> g_Achievements = {
    _ACH_ID( ACH_BOOT_STAMPING, "A boot stamping on a human face - forever" ),
    _ACH_ID( ACH_IMPERIAL_DRM, "Imperial DRM" ),
    _ACH_ID( ACH_NOT_TODAY, "Not today" ),
    _ACH_ID( ACH_TIRED_OF_REPAINTING_IT, "Tired of repainting it" ),
    _ACH_ID( ACH_NOT_EVEN_BEACONS, "Not even beacons" ),
    _ACH_ID( ACH_TIS_BUT_A_SCRATCH, "'Tis but a scratch!" ),
    _ACH_ID( ACH_FAVOURED_BY_THE_RNG_GODS, "Favoured by the RNG gods" ),
    _ACH_ID( ACH_WALK_SLOWLY, "Walk slowly and carry many, many guns" ),
    _ACH_ID( ACH_TACTICAL_RELOCATION, "Tactical relocation, not retreat" ),
    _ACH_ID( ACH_TO_GORGE_ON_LESSER_BEINGS, "To gorge on lesser on beings" ),
    _ACH_ID( ACH_THIS_ISNT_EVEN_MY_FINAL_FORM, "This isn't even my final form!" ),
    _ACH_ID( ACH_IMMORTAL_NO_MORE, "Immortal no more" ),
    _ACH_ID( ACH_MANAGED_FREEDOM, "Managed freedom" ),
    _ACH_ID( ACH_DEMIGOD, "Demigod" ),
    _ACH_ID( ACH_HEXTERMINATED, "HEXTERMINATED" ),
    _ACH_ID( ACH_HEGEGONE, "Hegegone" ),
    _ACH_ID( ACH_FINITE_WAR, "Finite war" )
};

/////////////////////////////////////////////////////////////////////
// AchievementsManager
/////////////////////////////////////////////////////////////////////

#if USE_STEAM

#if _MSC_VER
#pragma warning( push )
// warning C4355: 'this' : used in base member initializer list
// This is OK because it's warning on setting up the Steam callbacks, they won't use this until after construction is done
#pragma warning( disable : 4355 )
#endif // _MSC_VER
AchievementsManager::AchievementsManager()
    : m_pSteamUser( nullptr )
    , m_pSteamUserStats( nullptr )
    , m_CallbackUserStatsReceived( this, &AchievementsManager::OnUserStatsReceived )
{
    m_pSteamUser = SteamUser();
    m_pSteamUserStats = SteamUserStats();

    m_RequestedStats = false;
    m_StatsValid = false;
    m_StoreStats = false;
    m_DebugUIOpen = false;

    Genesis::ImGuiImpl::RegisterMenu( "Game", "Achievements", &m_DebugUIOpen );
}
#if _MSC_VER
#pragma warning( pop )
#endif

void AchievementsManager::Update()
{
    if ( !m_RequestedStats )
    {
        // Is Steam Loaded? If not we can't get stats.
        if ( m_pSteamUserStats == nullptr || m_pSteamUser == nullptr )
        {
            m_RequestedStats = true;
            return;
        }

        // If yes, request our stats.
        bool success = m_pSteamUserStats->RequestCurrentStats();

        // This function should only return false if we weren't logged in, and we already checked that.
        // But handle it being false again anyway, just ask again later.
        m_RequestedStats = success;
    }

    if ( !m_StatsValid )
    {
        return;
    }

#ifdef _DEBUG
    UpdateDebugUI();
#endif
}

void AchievementsManager::UnlockAchievement( AchievementID achievementID )
{
    if ( m_pSteamUserStats == nullptr )
    {
        return;
    }

    Achievement_t& achievement = g_Achievements[ achievementID ];
    if ( achievement.achieved )
    {
        return;
    }

    achievement.achieved = true;

    // mark it down
    m_pSteamUserStats->SetAchievement( achievement.idAsCString );

    // Send over the updated stats to the server.
    // Normally this would be deferred until a single point in the frame so
    // all stats would get aggregated, but Hexterminate doesn't have stat-based
    // achievements so this isn't necessary.
    m_pSteamUserStats->StoreStats();
}

// Callback for when we've received data from Steam containing information about stats and achievements.
// This information is authoritative and overrides any local data.
void AchievementsManager::OnUserStatsReceived( UserStatsReceived_t* pCallback )
{
    if ( m_pSteamUserStats == nullptr )
    {
        return;
    }

    // we may get callbacks for other games' stats arriving, ignore them
    if ( pCallback->m_nGameID == STEAM_APP_ID )
    {
        if ( k_EResultOK == pCallback->m_eResult )
        {
            Genesis::FrameWork::GetLogger()->LogInfo( "Received stats and achievements from Steam" );

            m_StatsValid = true;

            // Load achievements from Steam's response
            for ( Achievement_t& achievement : g_Achievements )
            {
                m_pSteamUserStats->GetAchievement( achievement.idAsCString, &achievement.achieved );
            }
        }
        else
        {
            Genesis::FrameWork::GetLogger()->LogWarning( "RequestStats - failed, %d", pCallback->m_eResult );
        }
    }
}

#ifdef _DEBUG
void AchievementsManager::UpdateDebugUI()
{
    if ( Genesis::ImGuiImpl::IsEnabled() && m_DebugUIOpen )
    {
        ImGui::SetNextWindowSize( ImVec2( 800.0f, 400.0f ) );
        ImGui::Begin( "Achievements", &m_DebugUIOpen );

        ImGui::Columns( 3 );
        ImGui::Separator();
        ImGui::Text( "ID" );
        ImGui::NextColumn();
        ImGui::Text( "Name" );
        ImGui::NextColumn();
        ImGui::Text( "Unlocked" );
        ImGui::NextColumn();
        ImGui::Separator();

        for ( const Achievement_t& achievement : g_Achievements )
        {
            ImGui::Text( achievement.idAsCString );
            ImGui::NextColumn();
            ImGui::Text( achievement.name );
            ImGui::NextColumn();

            if ( achievement.achieved )
            {
                ImGui::Text( "Yes" );
            }
            else
            {
                ImGui::PushID( achievement.id );
                if ( ImGui::Button( "Unlock" ) )
                {
                    UnlockAchievement( achievement.id );
                }
                ImGui::PopID();
            }
            ImGui::NextColumn();
        }

        if ( ImGui::Button( "Reset achievements" ) )
        {
            m_pSteamUserStats->ResetAllStats( true ); // true = reset achievements as well.

            for ( Achievement_t& achievement : g_Achievements )
            {
                achievement.achieved = false;
            }
        }

        ImGui::End();
    }
}
#endif // _DEBUG
#endif // USE_STEAM
} // namespace Hexterminate