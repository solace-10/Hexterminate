// Copyright 2020 Pedro Nunes
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
#include <vector>

#include "galaxycreationinfo.h"

namespace Hexterminate
{

class Faction;
class Galaxy;
class SectorInfo;

class GalaxyGenerator
{
public:
    GalaxyGenerator();
    void Run( Galaxy* pGalaxy, const GalaxyCreationInfo& creationInfo );

private:
    bool IsFinished() const;
    void GenerateHomeworlds( Galaxy* pGalaxy, const GalaxyCreationInfo& creationInfo );
    void GenerateSectors( Galaxy* pGalaxy, const GalaxyCreationInfo& creationInfo );
    void GenerateNames( Galaxy* pGalaxy, const GalaxyCreationInfo& creationInfo );
    std::vector<std::string> LoadNames() const;
    int CalculateNecessarySectors( FactionPresence presence ) const;

    struct Data
    {
        Faction* pFaction;
        SectorInfo* pHomeworldSector;
        int necessarySectors;
    };

    using DataVector = std::vector<Data>;
    DataVector m_Data;
};

} // namespace Hexterminate
