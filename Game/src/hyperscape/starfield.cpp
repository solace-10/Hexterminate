// Copyright 2021 Pedro Nunes
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

#include <random>

#include <glm/vec2.hpp>

#include <configuration.h>

#include "hyperscape/starfield.h"
#include "hyperscape/starfieldrep.h"
#include "misc/random.h"

namespace Hexterminate
{

//-----------------------------------------------------------------------------
// Starfield
//-----------------------------------------------------------------------------

Starfield::Starfield()
{
    m_pRep = std::make_unique<StarfieldRep>( this );
    m_pRep->Initialise();

    for ( StarfieldEntry& entry : m_Entries )
    {
        entry = CreateEntry( true );
    }
}

Starfield::~Starfield()
{
    m_pRep->RemoveFromScene();
}

void Starfield::Update( float delta )
{
    for ( StarfieldEntry& entry : m_Entries )
    {
        entry.Update( delta );

        if ( entry.IsFinished() )
        {
            entry = CreateEntry( false );
        }
    }
}

void Starfield::Show( bool state )
{
    // m_pRep->Show( state );
}

bool Starfield::IsVisible() const
{
    return true;
}

StarfieldEntry Starfield::CreateEntry( bool randomProgress )
{
    const glm::vec2 exclusionZone( 600.0f, 300.0f );
    const glm::vec2 resolution( static_cast<float>( Genesis::Configuration::GetScreenWidth() ), static_cast<float>( Genesis::Configuration::GetScreenHeight() ) );
    const glm::vec3 startPos( Random::Next( resolution.x ), Random::Next( resolution.y ), 0.0f );

    glm::vec3 endPos( startPos.x, startPos.y, 1.0f );
    float xCloseness = fabs( startPos.x / resolution.x / 2.0f );
    if ( startPos.x < resolution.x / 2.0f )
    {
        endPos.x -= exclusionZone.x / 2.0f * xCloseness;
    }
    else
    {
        endPos.x += exclusionZone.x / 2.0f * xCloseness;
    }

    float yCloseness = fabs( startPos.y / resolution.y / 2.0f );
    if ( startPos.y < resolution.y / 2.0f )
    {
        endPos.y -= exclusionZone.y / 2.0f * yCloseness;
    }
    else
    {
        endPos.y += exclusionZone.y / 2.0f * yCloseness;
    }

    return StarfieldEntry( startPos, endPos, randomProgress );
}

} // namespace Hexterminate
