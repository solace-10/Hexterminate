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

#pragma once

#include "hexterminate.h"

namespace Hexterminate
{

// This define must match the names set up in SteamWorks
enum AchievementID
{
    ACH_BOOT_STAMPING,
    ACH_IMPERIAL_DRM,
    ACH_NOT_TODAY,
    ACH_TIRED_OF_REPAINTING_IT,
    ACH_NOT_EVEN_BEACONS,
    ACH_TIS_BUT_A_SCRATCH,
    ACH_FAVOURED_BY_THE_RNG_GODS,
    ACH_WALK_SLOWLY,
    ACH_TACTICAL_RELOCATION,
    ACH_TO_GORGE_ON_LESSER_BEINGS,
    ACH_THIS_ISNT_EVEN_MY_FINAL_FORM,
    ACH_IMMORTAL_NO_MORE,
    ACH_MANAGED_FREEDOM,
    ACH_DEMIGOD,
    ACH_HEXTERMINATED,
    ACH_HEGEGONE,
    ACH_FINITE_WAR
};

struct Achievement_t
{
    AchievementID id;
    const char* idAsCString;
    char name[ 128 ];
    bool achieved;
};

#if USE_STEAM

class AchievementsManager
{
public:
    AchievementsManager();

    void Update();
    void UnlockAchievement( AchievementID achievementID );

    STEAM_CALLBACK( AchievementsManager, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived );

private:
#ifdef _DEBUG
    // Support for ImGui, appears under Game -> Achievements
    void UpdateDebugUI();
#endif

    // Steam User interface
    ISteamUser* m_pSteamUser;

    // Steam UserStats interface
    ISteamUserStats* m_pSteamUserStats;

    // Did we get the stats from Steam?
    bool m_RequestedStats;
    bool m_StatsValid;

    // Should we store stats this frame?
    bool m_StoreStats;

    bool m_DebugUIOpen;
};

#else

// Stubbed AchievementsManager
class AchievementsManager
{
public:
    void Update() {}
    void UnlockAchievement( AchievementID achievementID ) {}
};

#endif

} // namespace Hexterminate