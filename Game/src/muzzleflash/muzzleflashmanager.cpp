// Copyright 2019 Pedro Nunes
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

#include <memory.h>

#include "hexterminate.h"
#include "muzzleflash/muzzleflashmanager.h"

namespace Hexterminate
{

MuzzleflashManager::MuzzleflashManager()
{
    m_Muzzleflashes.reserve( 32 );
}

MuzzleflashManager::~MuzzleflashManager()
{
}

void MuzzleflashManager::Update( float delta )
{
    if ( g_pGame->IsPaused() == false )
    {
        for ( auto& data : m_Muzzleflashes )
        {
            data.SetLifetime( data.GetLifetime() - delta );
        }

        m_Muzzleflashes.erase( std::remove_if( m_Muzzleflashes.begin(), m_Muzzleflashes.end(),
                                   []( const MuzzleflashData& data ) { return data.GetLifetime() <= 0.0f; } ),
            m_Muzzleflashes.end() );
    }
}

} // namespace Hexterminate