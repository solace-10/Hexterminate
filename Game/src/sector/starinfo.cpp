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

#include <array>

#include <glm/common.hpp>

#include <genesis.h>
#include <logger.h>
#include <math/constants.h>
#include <math/misc.h>

#include "sector/starinfo.h"

namespace Hexterminate
{

// Based on information from http://www.enchantedlearning.com/subjects/astronomy/stars/startypes.shtml
static std::array<StarSurfaceTemperatureInfo, 6> gStarSurfaceTemperatureInfoTable = {
    StarSurfaceTemperatureInfo( StarType::MainSequenceClassM, 2500, glm::vec3( 1.00f, 0.30f, 0.20f ), glm::vec3( 0.80f, 0.20f, 0.10f ) ),
    StarSurfaceTemperatureInfo( StarType::MainSequenceClassK, 3500, glm::vec3( 0.80f, 0.65f, 0.30f ), glm::vec3( 0.80f, 0.35f, 0.10f ) ),
    StarSurfaceTemperatureInfo( StarType::MainSequenceClassG, 5000, glm::vec3( 0.80f, 0.85f, 0.50f ), glm::vec3( 0.80f, 0.55f, 0.30f ) ),
    StarSurfaceTemperatureInfo( StarType::MainSequenceClassF, 6000, glm::vec3( 0.50f, 0.55f, 0.55f ), glm::vec3( 0.30f, 0.45f, 0.50f ) ),
    StarSurfaceTemperatureInfo( StarType::MainSequenceClassA, 7500, glm::vec3( 0.50f, 0.55f, 0.85f ), glm::vec3( 0.30f, 0.45f, 0.70f ) ),
    StarSurfaceTemperatureInfo( StarType::MainSequenceClassA, 9000, glm::vec3( 0.50f, 0.55f, 0.85f ), glm::vec3( 0.30f, 0.45f, 0.70f ) )
};

///////////////////////////////////////////////////////////////////////////////
// StarInfo
///////////////////////////////////////////////////////////////////////////////

StarInfo::StarInfo( unsigned int seed )
    : m_Seed( seed )
    , m_SurfaceTemperature( 0 )
    , m_Distance( 0.0f )
    , m_StarType( StarType::MainSequenceClassM )
{
    static const unsigned int sMinimumSurfaceTemperature = 2500;
    static const unsigned int sMaximumSurfaceTemperature = 9000;

    srand( seed );
    m_SurfaceTemperature = (unsigned int)gRand( (float)sMinimumSurfaceTemperature, (float)sMaximumSurfaceTemperature );
    m_Distance = gRand( 1.5f, 2.5f );

    StarSurfaceTemperatureInfo& previousInfo = gStarSurfaceTemperatureInfoTable[ 0 ];
    for ( unsigned int index = 1; index < gStarSurfaceTemperatureInfoTable.size(); ++index )
    {
        const StarSurfaceTemperatureInfo& currentInfo = gStarSurfaceTemperatureInfoTable[ index ];
        if ( m_SurfaceTemperature < currentInfo.GetSurfaceTemperature() )
        {
            float ratio = 1.0f;
            const float divisor = static_cast<float>( currentInfo.GetSurfaceTemperature() - previousInfo.GetSurfaceTemperature() );
            if ( divisor >= std::numeric_limits<float>::epsilon() )
            {
                ratio = static_cast<float>( m_SurfaceTemperature - previousInfo.GetSurfaceTemperature() ) / divisor;
            }

            m_StarType = previousInfo.GetStarType();
            m_CoreColour = glm::mix( previousInfo.GetCoreColour(), currentInfo.GetCoreColour(), ratio );
            m_CoronaColour = glm::mix( previousInfo.GetCoronaColour(), currentInfo.GetCoronaColour(), ratio );

            // Offset in a ring around the centre of the background.
            // We avoid the very centre as that can make projectiles very difficult to see.
            const float a = gRand( 0.0f, Genesis::kPi * 2.0f );
            const float c = cosf( a );
            const float s = sinf( a );
            const float r = gRand( 0.2f, 0.5f );
            m_Offset = glm::vec2( c * r, s * r );

            break;
        }
        else
        {
            previousInfo = currentInfo;
        }
    }
}

void StarInfo::PrintOut() const
{
    Genesis::Logger* pLog = Genesis::FrameWork::GetLogger();
    pLog->LogInfo( "Star info: " );
    pLog->LogInfo( "- Seed: %u", m_Seed );

    if ( m_StarType == StarType::MainSequenceClassA )
        pLog->LogInfo( "- Main sequence star, class A" );
    else if ( m_StarType == StarType::MainSequenceClassF )
        pLog->LogInfo( "- Main sequence star, class F" );
    else if ( m_StarType == StarType::MainSequenceClassG )
        pLog->LogInfo( "- Main sequence star, class G" );
    else if ( m_StarType == StarType::MainSequenceClassK )
        pLog->LogInfo( "- Main sequence star, class K" );
    else if ( m_StarType == StarType::MainSequenceClassM )
        pLog->LogInfo( "- Main sequence star, class M" );

    pLog->LogInfo( "- Surface temperature: %u", m_SurfaceTemperature );
    pLog->LogInfo( "- Core colour: %.2f %.2f %.2f", m_CoreColour.r, m_CoreColour.g, m_CoreColour.b );
    pLog->LogInfo( "- Corona colour: %.2f %.2f %.2f", m_CoronaColour.r, m_CoronaColour.g, m_CoronaColour.b );
    pLog->LogInfo( "- Distance: %.2f", m_Distance );
}

///////////////////////////////////////////////////////////////////////////////
// StarSurfaceTemperatureInfo
///////////////////////////////////////////////////////////////////////////////

StarSurfaceTemperatureInfo::StarSurfaceTemperatureInfo( StarType starType, unsigned int surfaceTemperature, const glm::vec3& coreColour, const glm::vec3& coronaColour )
    : m_StarType( starType )
    , m_SurfaceTemperature( surfaceTemperature )
    , m_CoreColour( coreColour )
    , m_CoronaColour( coronaColour )
{
}

} // namespace Hexterminate