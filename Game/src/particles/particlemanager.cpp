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

#ifdef _DEBUG
#include "hexterminate.h"
#include <render/debugrender.h>
#include <sstream>
#endif

#include "particles/particleemitter.h"
#include "particles/particlemanager.h"

namespace Hexterminate
{

static int sMaxEmitters = 2048;

ParticleManager::ParticleManager()
{
    m_Emitters.resize( sMaxEmitters );
    for ( int i = 0; i < sMaxEmitters; ++i )
    {
        m_Emitters[ i ].Reset();
    }
    m_Idx = 0;
}

ParticleManager::~ParticleManager()
{
}

void ParticleManager::Update( float delta )
{
    int activeEmitters = 0;
    for ( auto& emitter : m_Emitters )
    {
        if ( emitter.IsActive() )
        {
            emitter.Update( delta );
            activeEmitters++;
        }
    }

    //#ifdef _DEBUG
    //	glm::vec3 colour = ( activeEmitters < sMaxEmitters ) ? glm::vec3( 0.0f, 1.0f, 0.0f ) : glm::vec3( 1.0f, 0.0f, 0.0f );
    //	std::stringstream ss;
    //	ss << "Active emitters: " << activeEmitters << "/" << sMaxEmitters;
    //	g_pGame->GetDebugRender()->DrawText( 16.0f, 16.0f, ss.str(), colour );
    //#endif
}

ParticleEmitter* ParticleManager::GetAvailableEmitter()
{
    size_t numEmitters = m_Emitters.size();
    for ( size_t i = m_Idx; i < numEmitters; ++i )
    {
        if ( m_Emitters[ i ].IsActive() == false )
        {
            m_Idx = i;
            m_Emitters[ i ].Reset();
            return &m_Emitters[ i ];
        }
    }

    for ( size_t i = 0; i < m_Idx; ++i )
    {
        if ( m_Emitters[ i ].IsActive() == false )
        {
            m_Idx = i;
            m_Emitters[ i ].Reset();
            return &m_Emitters[ i ];
        }
    }

    // If all emitters have been used, override the first one
    m_Emitters[ 0 ].Reset();
    return &m_Emitters[ 0 ];
}

} // namespace Hexterminate