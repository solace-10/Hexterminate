// Copyright 2014 Pedro Nunes
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

#include "loot/lootprobability.h"
#include <math/misc.h>
#include <numeric>

namespace Hexterminate
{

LootProbability::LootProbability()
{
    for ( int i = 0; i < (int)ModuleRarity::Count; ++i )
    {
        m_Value[ i ] = 0.0f;
    }
}

LootProbability::LootProbability( float commonChance, float uncommonChance, float rareChance, float artifactChance, float legendaryChance )
{
    m_Value[ (int)ModuleRarity::Trash ] = 0.0f;
    m_Value[ (int)ModuleRarity::Common ] = commonChance;
    m_Value[ (int)ModuleRarity::Uncommon ] = uncommonChance;
    m_Value[ (int)ModuleRarity::Rare ] = rareChance;
    m_Value[ (int)ModuleRarity::Artifact ] = artifactChance;
    m_Value[ (int)ModuleRarity::Legendary ] = legendaryChance;

    // Make sure the sum of all values is 0 or 1.
    // Also, it is possible that ships have no chance of dropping any loot, so a sum of 0 does make sense.
    float v = 0.0f;
    for ( int i = 0; i < (int)ModuleRarity::Count; ++i )
        v += m_Value[ i ];

    SDL_assert( gIsEqual( v, 0.0f ) || gIsEqual( v, 1.0f ) );
}

bool LootProbability::Get( ModuleRarity& rarity ) const
{
    float v = gRand();
    float p = 0.0f;
    for ( int i = 0; i < (int)ModuleRarity::Count; ++i )
    {
        p += m_Value[ i ];
        if ( v <= p )
        {
            rarity = static_cast<ModuleRarity>( i );
            return true;
        }
    }
    return false;
}

} // namespace Hexterminate