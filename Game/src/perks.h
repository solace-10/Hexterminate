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

#include <bitset>

#include "serialisable.h"

namespace Hexterminate
{

/////////////////////////////////////////////////////////////////////
// The perks are serialised into a bitfield which is build from this
// enum, so do not change the order.
/////////////////////////////////////////////////////////////////////

enum class Perk
{
    EngineRemapping,
    EvasionProtocols,
    ThrustVectoring,
    RammingSpeed,
    GunshipConstruction,
    BattlecruiserConstruction,
    BattleshipConstruction,
    DreadnaughtConstruction,
    SupportRequest,
    PrimaryFleet,
    PriorityRequisition,
    PrototypeAccess,
    MagneticLoaders,
    AdvancedElectrocoils,
    PlasmaWarheads,
    Siegebreaker,
    AdvancedHeatsinks,
    PhaseSynchronisation,
    Disruption,
    Overload,
    FrequencyCycling,
    KineticHardening,
    Superconductors,
    EmergencyCapacitors,
    ReclaimedSectors,
    SharedGlory,
    SwordOfTheEmpire,
    UnityIsStrength,
    LighterMaterials,
    Nanobots,
    ReinforcedBulkheads,
    Unbroken,

    Count
};

/////////////////////////////////////////////////////////////////////
// Perks
/////////////////////////////////////////////////////////////////////

static const unsigned int sMaxPerks = 64;

class Perks : public Serialisable
{
public:
    Perks();

    bool IsEnabled( Perk perk ) const;
    void Enable( Perk perk );
    void Reset();

    // Serialisable
    virtual bool Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement ) override;
    virtual bool Read( tinyxml2::XMLElement* pRootElement ) override;
    virtual int GetVersion() const override { return 1; }
    virtual void UpgradeFromVersion( int version ) override {}

private:
    std::bitset<sMaxPerks> m_Bitset;
};

inline bool Perks::IsEnabled( Perk perk ) const
{
    return m_Bitset.test( static_cast<std::size_t>( perk ) );
}

inline void Perks::Enable( Perk perk )
{
    m_Bitset.set( static_cast<std::size_t>( perk ) );
}

inline void Perks::Reset()
{
    m_Bitset.reset();

    // The "Gunship construction" perk should always be enabled
    Enable( Perk::GunshipConstruction );
}

} // namespace Hexterminate
