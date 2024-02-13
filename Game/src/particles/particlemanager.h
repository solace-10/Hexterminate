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

#pragma once

#include "particles/particleemitter.h"
#include <vector>

namespace Hexterminate
{

class ParticleEmitter;
typedef std::vector<ParticleEmitter> ParticleEmitterVector;

///////////////////////////////////////////////////////////////////////////////
// ParticleManager
///////////////////////////////////////////////////////////////////////////////

class ParticleManager
{
public:
    ParticleManager();
    ~ParticleManager();
    void Update( float delta );

    ParticleEmitter* GetAvailableEmitter();

    const ParticleEmitterVector& GetEmitters() const;

private:
    ParticleEmitterVector m_Emitters;
    size_t m_Idx;
};

inline const ParticleEmitterVector& ParticleManager::GetEmitters() const
{
    return m_Emitters;
}

} // namespace Hexterminate
