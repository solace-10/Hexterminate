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

#include <string>

namespace Hexterminate
{

// Expansion arc
static const std::string sFirstExpansionEvent( "#first_expansion_event" );
static const std::string sFirstExpansionEventCompleted( "#first_expansion_event_completed" );
static const std::string sSecondExpansionEvent( "#second_expansion_event" );
static const std::string sSecondExpansionEventCompleted( "#second_expansion_event_completed" );
static const std::string sExpansionArcFinished( "#expansion_arc_finished" );

// Pirate arc
static const std::string sFirstPirateEvent( "#first_pirate_event" );
static const std::string sFirstPirateEventCompleted( "#first_pirate_event_completed" );
static const std::string sPirateShipyardsCaptured( "#pirate_shipyards_captured" );
static const std::string sKillPirateFlagship( "#kill_pirate_flagship" );
static const std::string sKillPirateFlagshipCompleted( "#kill_pirate_flagship_completed" );
static const std::string sPirateArcFinished( "#pirate_arc_finished" );

// Marauder arc
static const std::string sMarauderIntro( "#marauder_intro" );
static const std::string sConquerMuspell( "#conquer_muspell" );
static const std::string sConquerMuspellCompleted( "#conquer_muspell_completed" );
static const std::string sConquerSurtr( "#conquer_surtr" );
static const std::string sConquerSurtrCompleted( "#conquer_surtr_completed" );
static const std::string sKillMarauderFlagship( "#kill_marauder_flagship" );
static const std::string sKillMarauderFlagshipCompleted( "#kill_marauder_flagship_completed" );
static const std::string sConquerValhalla( "#conquer_valhalla" );
static const std::string sConquerValhallaCompleted( "#conquer_valhalla_completed" );
static const std::string sMarauderArcFinished( "#marauder_arc_finished" );

// Ascent arc
static const std::string sAscentIntro( "#ascent_intro" );
static const std::string sConquerSolarisSecundus( "#conquer_solaris_secundus" );
static const std::string sConquerSolarisSecundusCompleted( "conquer_solaris_secundus_completed" );
static const std::string sKillAscentFlagship( "#kill_ascent_flagship" );
static const std::string sKillAscentFlagshipCompleted( "#kill_ascent_flagship_completed" );
static const std::string sAnchorDestroyed( "#anchor_destroyed" );
static const std::string sAscentArcFinished( "#ascent_arc_finished" );

// Chrysamere arc
static const std::string sChrysamereIntro( "#chrysamere_intro" );
static const std::string sEnterCradle( "#enter_cradle" );
static const std::string sEnterCradleCompleted( "#enter_cradle_completed" );
static const std::string sPlayerHasOrionsSword( "#player_has_orions_sword" );
static const std::string sChrysamereArcFinished( "#chrysamere_arc_finished" );
static const std::string sFinalChrysamereDestroyed( "#final_chrysamere_destroyed" );

// Iriani arc
static const std::string sIrianiIntro( "#iriani_intro" );
static const std::string sArbitersIntro( "#arbiters_intro" );
static const std::string sArbitersDestroyed( "#arbiters_destroyed" );
static const std::string sConquerIrianiPrime( "#conquer_iriani_prime" );
static const std::string sConquerIrianiPrimeCompleted( "#conquer_iriani_prime_completed" );
static const std::string sIrianiArcFinished( "iriani_arc_finished" );

// Game end
static const std::string sGameEnd( "#game_end" );

} // namespace Hexterminate