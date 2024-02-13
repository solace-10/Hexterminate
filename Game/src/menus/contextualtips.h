// Copyright 2016 Pedro Nunes
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

#include <map>
#include <string>

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// ContextualTipType
// List of all contextual tips that can be displayed. If this list is changed
// both ContextualTipTag and ContextualTipText in contextualtips.cpp have to be
// updated to match.
///////////////////////////////////////////////////////////////////////////////

enum class ContextualTipType
{
    EnterSector1,
    EnterSector2,
    EnterSectorWithArmourRepairer,
    EnterSectorWithParticleAccelerator,
    ArmourRepairer,
    Autocannon,
    Artillery,
    Missiles,
    Torpedoes,
    Rockets,
    IonCannon,
    Lance,
    Engines,
    MissileInterceptor,
    Reactor,
    LightArmour,
    MediumArmour,
    HeavyArmour,
    Bridge,
    ReactiveArmour,
    PoweredArmour,
    RegenerativeArmour,
    OmniArmour,
    Shipyard,
    DeflectorShield,
    NullPointGenerator,
    ShieldBattery,
    CovariantShieldArray,
    NoEnemies,
    EngineDisruptor,
    ParticleAccelerator,
    Antiproton,
    QuantumAlternator,
    RammingProw,

    Count
};

///////////////////////////////////////////////////////////////////////////////
// ContextualTips
// Displays a contextual tip from a previously set ContextualTipType.
// These tips are presented through the Fleet Intelligence Officer and an entry
// is added to the blackboard to make sure a given tip is only presented once
// per playthrough.
///////////////////////////////////////////////////////////////////////////////

class ContextualTips
{
public:
    static bool Present( ContextualTipType tip, bool overridePreference = false );
    static bool Present( const std::string& tipTag, bool overridePreference = false );
    static bool HasBeenPresented( ContextualTipType tip );

private:
    typedef std::map<std::string, ContextualTipType> TagTypeMap;
    static TagTypeMap m_LookupTable;
    static bool m_LookupTableInitialised;
};

} // namespace Hexterminate
