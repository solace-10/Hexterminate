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

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace Hexterminate
{

enum class StarType
{
    MainSequenceClassA, // Blue
    MainSequenceClassF, // White to blue
    MainSequenceClassG, // Yellow to white
    MainSequenceClassK, // Red to yellow
    MainSequenceClassM // Red
};

///////////////////////////////////////////////////////////////////////////////
// StarInfo
///////////////////////////////////////////////////////////////////////////////

class StarInfo
{
public:
    StarInfo( unsigned int seed );

    unsigned int GetSeed() const;
    const glm::vec3& GetCoreColour() const;
    const glm::vec3& GetCoronaColour() const;
    const glm::vec2& GetOffset() const;
    float GetDistance() const;

    void PrintOut() const;

private:
    unsigned int m_Seed;
    unsigned int m_SurfaceTemperature;
    glm::vec3 m_CoreColour;
    glm::vec3 m_CoronaColour;
    glm::vec2 m_Offset;
    float m_Distance;
    StarType m_StarType;
};

inline unsigned int StarInfo::GetSeed() const
{
    return m_Seed;
}

inline const glm::vec3& StarInfo::GetCoreColour() const
{
    return m_CoreColour;
}

inline const glm::vec3& StarInfo::GetCoronaColour() const
{
    return m_CoronaColour;
}

inline const glm::vec2& StarInfo::GetOffset() const
{
    return m_Offset;
}

inline float StarInfo::GetDistance() const
{
    return m_Distance;
}

///////////////////////////////////////////////////////////////////////////////
// StarSurfaceTemperatureInfo
///////////////////////////////////////////////////////////////////////////////

class StarSurfaceTemperatureInfo
{
public:
    StarSurfaceTemperatureInfo( StarType starType, unsigned int surfaceTemperature, const glm::vec3& coreColour, const glm::vec3& coronaColour );

    StarType GetStarType() const;
    unsigned int GetSurfaceTemperature() const;
    const glm::vec3& GetCoreColour() const;
    const glm::vec3& GetCoronaColour() const;

private:
    StarType m_StarType;
    unsigned int m_SurfaceTemperature;
    glm::vec3 m_CoreColour;
    glm::vec3 m_CoronaColour;
};

inline StarType StarSurfaceTemperatureInfo::GetStarType() const
{
    return m_StarType;
}

inline unsigned int StarSurfaceTemperatureInfo::GetSurfaceTemperature() const
{
    return m_SurfaceTemperature;
}

inline const glm::vec3& StarSurfaceTemperatureInfo::GetCoreColour() const
{
    return m_CoreColour;
}

inline const glm::vec3& StarSurfaceTemperatureInfo::GetCoronaColour() const
{
    return m_CoronaColour;
}

} // namespace Hexterminate
