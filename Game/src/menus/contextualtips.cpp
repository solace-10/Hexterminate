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

#include <string>

#include "blackboard.h"
#include "hexterminate.h"
#include "menus/contextualtips.h"

namespace Hexterminate
{

////////////////////////////////////////////////////////////////////////////
// ContextualTipTag
////////////////////////////////////////////////////////////////////////////

std::string ContextualTipTag[ static_cast<std::size_t>( ContextualTipType::Count ) ] = {
    "#tip_enter_sector_1",
    "#tip_enter_sector_2",
    "#tip_armour_repairer",
    "#tip_enter_sector_particle_accelerator",
    "#tip_armour_repairer",
    "#tip_autocannon",
    "#tip_artillery",
    "#tip_missiles",
    "#tip_torpedoes",
    "#tip_rockets",
    "#tip_ion_cannon",
    "#tip_lance",
    "#tip_engines",
    "#tip_missile_interceptor",
    "#tip_reactor",
    "#tip_light_armour",
    "#tip_medium_armour",
    "#tip_heavy_armour",
    "#tip_bridge",
    "#tip_reactive_armour",
    "#tip_powered_armour",
    "#tip_regenerative_armour",
    "#tip_omni_armour",
    "#tip_shipyard",
    "#tip_deflector_shield",
    "#tip_null_point_generator",
    "#tip_shield_battery",
    "#tip_covariant_shield_array",
    "#tip_no_enemies",
    "#tip_engine_disruptor",
    "#tip_particle_accelerator",
    "#tip_antiproton",
    "#tip_quantum_alternator",
    "#tip_ramming_prow"
};

////////////////////////////////////////////////////////////////////////////
// ContextualTipText
////////////////////////////////////////////////////////////////////////////

std::string ContextualTipText[ static_cast<std::size_t>( ContextualTipType::Count ) ] = {
    // EnterSector1
    "Welcome to the battlefield, Captain. Your ship can be controlled through the keys W, A, S and D, as well as Q and E for strafing. Your turrets will aim at your crosshair and the left mouse button will open fire.",

    // EnterSector2
    "One of your modules is the crown of the Imperial achievements: the phase barrier, an extremely powerful directional shield. You can activate it by pressing '1' and you should make maximum use of it to block enemy fire. It recharges when not in use.",

    // EnterSectorWithArmourRepairer
    "Your ship is also fitted with a phased assembler module. It can repair any modules which haven't been fully destroyed, but it is twice as effective at fixing armour modules.",

    // EnterSectorWithParticleAccelerator
    "Captain, one of the ships we're facing has a Particle Accelerator. This is a very slow firing beam weapon but it must be blocked by our Phase Barrier or it will likely slice our ship in half.",

    // ArmourRepairer
    "This is a phased assembler module, which can repair any modules which haven't been fully destroyed. However, it is twice as effective at repairing armour modules.",

    // Autocannon
    "Autocannons are kinetic weapons which reward getting close and personal. Each individual bullet doesn't deal much damage but the sheer volume of fire makes up for that.",

    // Artillery
    "Artillery cannons have a very long range, making them ideal for skirmish tactics and to target individual modules - landing a few well placed shots on an engine module can be extremely effective. They require more energy to operate than autocannons.",

    // Missiles
    "Missile weapons deal kinetic damage at long range. They will home in on the enemy's bridge but be aware that they can be intercepted.",

    // Torpedoes
    "Unguided and slow but dealing massive kinetic damage at extreme ranges, few ships can deal with a sustained barrage of torpedoes. However, their low speed makes them very prone to interception. Being a fixed, forward mounted weapon, you can fire them by pressing the right mouse button.",

    // Rockets
    "Rockets are very short range but keep the same homing capabilities as the larger hunter missiles. However, being considerably faster and fired at a shorter range, missile interceptors aren't nearly as effective against them, making rockets deadly when massed.",

    // Ion cannon
    "Ion cannons are fixed, forward facing energy weapons which are very effective against unpowered armour. Particularly with the energy perk \"Overload\", ion cannons can make short work of shields. Being a fixed weapon, you can fire them by pressing the right mouse button.",

    // Lance
    "A lance's instantaneous, long range damage comes at a high energy cost. Its pinpoint damage application can be used to take out specific modules with ease and being an energy weapon, it fares quite well against the traditional reactive armours used by most ships in the galaxy.",

    // Engine
    "Engine modules are essential to keep your ship mobile in the battlefield. As your ship becomes larger, you should have more engine modules to compensate for the additional weight.",

    // Missile interceptors
    "Missile interceptors are effective at protecting your ship and those around you from rockets, missiles and torpedoes. These modules have a considerable energy drain when active, so you might have to toggle them on and off as needed.",

    // Reactor
    "Everything in a ship requires energy, from firing weapons to keeping the engines running. Some reactor manufacturers will provide higher capacity while others will enable faster recharge rates.",

    // Light armour
    "Light armour is intended for fast ships which want to maintain range. It provides some protection without slowing down the ship too much.",

    // Medium armour
    "Medium armour is a compromise between weight and effectiveness. A front line ship that wants to stay maneuverable will often have medium armour.",

    // Heavy armour
    "Heavy armour sacrifices mobility for the highest resistance to damage. This is a good option for an assault ship's bow, or even as overall armour if the trade-off is deemed acceptable.",

    // Bridge
    "The bridge is the heart of your ship: if it is destroyed, the ship is lost. This goes both ways, so attempt to take out the bridge of enemy ships as quickly as possible.",

    // Reactive armour
    "Reactive armour provides superior protection against projectiles, causing a considerably higher percentage to be safely deflected.",

    // Powered armour
    "Powered armour is electrically charged and made with rare materials which are capable of dissipating energy at a high rate. As such, it takes considerably less damage from ion cannons and lances than other kinds of armour.",

    // Regenerative armour
    "Regenerative armour contains a small nanobot factory which is capable of restoring the module's structural integrity after taking damage. It works particularly well when combined with a dedicated armour repair module.",

    // Omni armour
    "An omni armour module... these are extremely rare, a technology of the old Empire which can no longer be produced. It combines the properties of reactive, powered and regenerative armours, making it extremely effective against any kind of damage.",

    // Shipyard
    "This sector has a shipyard, at which you will be able to dock at if there are no enemies present. At the shipyard you'll be able to customise our ship with the modules you find throughout the galaxy.",

    // Deflector shields
    "Unlike shield batteries, deflector shields can regenerate on their own, allowing us to stay in a fight for longer. However, they do need energy to run.",

    // Null point generators
    "Null point generators are shields with low capacity but high regeneration rate. Mixed with a shield battery or deflector, the resulting shield can be quite difficult to break.",

    // Shield batteries
    "Shield batteries essentially work as a damage buffer, having considerable capacity but being unable to regenerate on their own. As such they work best when coupled with another kind of shield generator.",

    // Covariant shield arrays
    "Covariant shield arrays are a new technology which is still somewhat unstable. They have tremendous regeneration rates but if the shield is taken down, the resulting energy feedback usually destroys the module.",

    // No enemies present
    "With no hostiles in the sector, we can jump out immediately. Press the 'escape' key when you're ready for us to enter hyperspace.",

    // Engine disruptor
    "An engine disruptor module triggers a shock wave which greatly slows down any ships caught in the blast. It has a long range but also a considerable cooldown, so it needs to be used tactically for the best results.",

    // Particle accelerator
    "Particle accelerators take a few seconds to power up to fire and then twenty seconds to reload. It can cut through any known armour, being able to destroy even a dreadnought in a single shot if you disable the shields first.",

    // Antiproton cannon
    "Antiproton technology is extensively used by the Hegemon and highly devastating. A shield can block the slow firing projectile, but only if you have a Quantum State Alternator in your ship.",

    // Quantum alternator
    "The Quantum State Alternator allows you block antiproton projectiles, as long as they're both in the same state. For example, you can block a black projectile if your shield's state is also black. You can swap between these states by using space bar.",

    // Ramming prow
    "Ramming prows are armour modules specifically designed for ramming other ships. If this module hits another ship with Ramming Speed, it both deals increased damage to larger ships and takes very little damage from the ramming action."
};

////////////////////////////////////////////////////////////////////////////
// ContextualTips
////////////////////////////////////////////////////////////////////////////

ContextualTips::TagTypeMap ContextualTips::m_LookupTable;
bool ContextualTips::m_LookupTableInitialised = false;

bool ContextualTips::Present( ContextualTipType tip, bool overridePreference /* = false */ )
{
    if ( overridePreference == false && g_pGame->AreContextualTipsEnabled() == false )
    {
        return false;
    }

    const std::string& tag = ContextualTipTag[ static_cast<std::size_t>( tip ) ];
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    if ( pBlackboard != nullptr && pBlackboard->Exists( tag ) == false )
    {
        const std::string& text = ContextualTipText[ static_cast<std::size_t>( tip ) ];
        g_pGame->AddIntel( GameCharacter::FleetIntelligence, text );
        pBlackboard->Add( tag );
        return true;
    }
    else
    {
        return false;
    }
}

bool ContextualTips::Present( const std::string& tipTag, bool overridePreference /* = false */ )
{
    // Lazy initialisation of the lookup table
    if ( m_LookupTableInitialised == false )
    {
        for ( std::size_t type = 0; type < static_cast<std::size_t>( ContextualTipType::Count ); ++type )
        {
            m_LookupTable[ ContextualTipTag[ type ] ] = static_cast<ContextualTipType>( type );
        }
        m_LookupTableInitialised = true;
    }

    auto lookupIterator = m_LookupTable.find( tipTag );
    if ( lookupIterator == m_LookupTable.end() )
    {
        return false;
    }
    else
    {
        ContextualTipType type = lookupIterator->second;
        return Present( type, overridePreference );
    }
};

bool ContextualTips::HasBeenPresented( ContextualTipType tip )
{
    const std::string& tag = ContextualTipTag[ static_cast<std::size_t>( tip ) ];
    BlackboardSharedPtr pBlackboard = g_pGame->GetBlackboard();
    return ( pBlackboard != nullptr && pBlackboard->Exists( tag ) );
}

} // namespace Hexterminate
