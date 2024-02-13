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

#pragma once

#include "muzzleflash/muzzleflashdata.h"
#include <vector>

namespace Hexterminate
{

using MuzzleflashDataVector = std::vector<MuzzleflashData>;

class MuzzleflashManager
{
public:
    MuzzleflashManager();
    ~MuzzleflashManager();

    void Update( float delta );
    const MuzzleflashDataVector& GetMuzzleflashes() const;
    void Add( const MuzzleflashData& muzzleflash );

private:
    MuzzleflashDataVector m_Muzzleflashes;
};

inline const MuzzleflashDataVector& MuzzleflashManager::GetMuzzleflashes() const
{
    return m_Muzzleflashes;
}

inline void MuzzleflashManager::Add( const MuzzleflashData& muzzleflash )
{
    m_Muzzleflashes.push_back( muzzleflash );
}

} // namespace Hexterminate
