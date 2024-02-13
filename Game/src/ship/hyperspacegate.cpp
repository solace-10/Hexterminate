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

#include <genesis.h>
#include <math/misc.h>
#include <physics/rigidbody.h>
#include <resources/resourcemodel.h>
#include <resources/resourcesound.h>
#include <sound/soundinstance.h>

#include "globals.h"
#include "hexterminate.h"
#include "sector/sector.h"
#include "ship/hexgrid.h"
#include "ship/hyperspacecore.h"
#include "ship/module.h"
#include "ship/moduleinfo.h"
#include "ship/ship.h"

#include "ship/hyperspacegate.h"

namespace Hexterminate
{

static const float sGateOpeningDuration = 1.0f;

HyperspaceGate::HyperspaceGate( Ship* pOwner )
    : m_pOwner( pOwner )
    , m_pModel( nullptr )
    , m_GateScale( 0.0f )
    , m_State( HyperspaceGateState::Invalid )
    , m_JumpTimer( HyperspaceCoreJumpDuration )
    , m_OpeningTimer( sGateOpeningDuration )
    , m_Direction( HyperspaceJumpDirection::JumpInvalid )
    , m_pSFX( nullptr )
{
    m_GatePosition = glm::vec3( 0.0f );
    m_GatePositionLocal = glm::vec3( 0.0f );

    m_BoundingBoxTopLeft = glm::vec3( 0.0f );
    m_BoundingBoxBottomRight = glm::vec3( 0.0f );
}

void HyperspaceGate::Initialise()
{
    m_pModel = (Genesis::ResourceModel*)Genesis::FrameWork::GetResourceManager()->GetResource( "data/models/misc/hyperspace/hyperspace.tmf" );

    if ( m_pModel == nullptr )
    {
        Genesis::FrameWork::GetLogger()->LogError( "Couldn't find required hyperspace model." );
    }
    else
    {
        m_pModel->SetFlipAxis( false );
    }

    CalculateBoundingBox();

    LoadSFX();
}

// Calculates the bounding box which the hyperspace gate traverse through
void HyperspaceGate::CalculateBoundingBox()
{
    glm::vec3 localSpaceBoundingBox( 0.0f );

    // Calculate the basic bounding box, but Module::GetLocalPosition() returns the center point of the module,
    // but we'll expand the resulting BB afterwards to take the module's size into consideration.
    bool valid = false;
    for ( auto& pModule : m_pOwner->GetModules() )
    {
        glm::vec3 localSpaceModule = pModule->GetLocalPosition();
        if ( !valid )
        {
            m_BoundingBoxTopLeft = localSpaceModule;
            m_BoundingBoxBottomRight = localSpaceModule;
            valid = true;
        }
        else
        {
            if ( localSpaceModule.x < m_BoundingBoxTopLeft.x )
                m_BoundingBoxTopLeft.x = localSpaceModule.x;
            if ( localSpaceModule.x > m_BoundingBoxBottomRight.x )
                m_BoundingBoxBottomRight.x = localSpaceModule.x;
            if ( localSpaceModule.y > m_BoundingBoxTopLeft.y )
                m_BoundingBoxTopLeft.y = localSpaceModule.y;
            if ( localSpaceModule.y < m_BoundingBoxBottomRight.y )
                m_BoundingBoxBottomRight.y = localSpaceModule.y;
        }
    }

    // The exact bounding box for the ship would be given by expanding it by the module's half width.
    // However, to make the visual effect look better, we actually expand it by a module's full width each way.
    m_BoundingBoxTopLeft += glm::vec3( -sModuleWidth, sModuleHeight, 0.0f );
    m_BoundingBoxBottomRight += glm::vec3( sModuleWidth, -sModuleHeight, 0.0f );
}

void HyperspaceGate::LoadSFX()
{
    using namespace Genesis;
    using namespace std::literals;

    m_pSFX = FrameWork::GetResourceManager()->GetResource<ResourceSound*>( "data/sfx/hyperspace.wav" );
    if ( m_pSFX != nullptr )
    {
        m_pSFX->Initialise( SOUND_FLAG_3D | SOUND_FLAG_FX );
        m_pSFX->SetInstancingLimit( 100ms );
    }
}

void HyperspaceGate::PlaySFX()
{
    if ( m_pSFX != nullptr )
    {
        using namespace Genesis;

        Sound::SoundInstanceSharedPtr pSoundInstance = FrameWork::GetSoundManager()->CreateSoundInstance( m_pSFX, Genesis::Sound::SoundBus::Type::SFX, m_GatePosition, 1000.0f );
    }
}

void HyperspaceGate::Open( HyperspaceJumpDirection jumpDirection )
{
    SDL_assert( m_State == HyperspaceGateState::Invalid || m_State == HyperspaceGateState::Closed );

    m_State = HyperspaceGateState::Opening;
    m_JumpTimer = HyperspaceCoreJumpDuration;
    m_OpeningTimer = sGateOpeningDuration;
    m_Direction = jumpDirection;

    PlaySFX();

    Update( 0.0f ); // Force update to correctly set up shader states.
}

void HyperspaceGate::Update( float delta )
{
    Genesis::SceneObject::Update( delta );

    if ( m_State == HyperspaceGateState::Invalid )
    {
        return;
    }

    const float maxGateScale = ( m_BoundingBoxBottomRight.x - m_BoundingBoxTopLeft.x ) / sModuleWidth;
    const float halfPointX = m_BoundingBoxTopLeft.x + ( m_BoundingBoxBottomRight.x - m_BoundingBoxTopLeft.x ) / 2.0f;

    glm::vec3 hyperspaceGatePosLocal( halfPointX, m_BoundingBoxTopLeft.y, 0.0f );
    glm::vec3 hyperspaceGateEndPosLocal( halfPointX, m_BoundingBoxBottomRight.y, 0.0f );

    const glm::mat4x4& ownerTransform = m_pOwner->GetRigidBody()->GetWorldTransform();
    m_GatePosition = glm::vec3( ownerTransform * glm::vec4( hyperspaceGatePosLocal, 1.0f ) );

    glm::vec3 hyperspaceGateEndPos;
    hyperspaceGateEndPos = glm::vec3( ownerTransform * glm::vec4( hyperspaceGateEndPosLocal, 1.0f ) );

    float jumpRatio = 0.0f;
    if ( m_State == HyperspaceGateState::Opening )
    {
        m_OpeningTimer -= delta;
        if ( m_OpeningTimer <= 0.0f )
        {
            m_State = HyperspaceGateState::Transitioning;
        }

        m_GateScale = powf( gClamp<float>( 1.0f - m_OpeningTimer / sGateOpeningDuration, 0.0f, 1.0f ), 4 ) * maxGateScale;
    }
    else if ( m_State == HyperspaceGateState::Transitioning )
    {
        m_JumpTimer -= delta;
        if ( m_JumpTimer <= 0.0f )
        {
            m_OpeningTimer = sGateOpeningDuration;
            m_State = HyperspaceGateState::Closing;
        }

        jumpRatio = powf( gClamp<float>( 1.0f - m_JumpTimer / HyperspaceCoreJumpDuration, 0.0f, 1.0f ), 4 );
    }
    else if ( m_State == HyperspaceGateState::Closing )
    {
        jumpRatio = 1.0f;

        m_OpeningTimer -= delta;
        if ( m_OpeningTimer <= 0.0f )
        {
            m_State = HyperspaceGateState::Closed;
        }

        m_GateScale = powf( gClamp<float>( m_OpeningTimer / sGateOpeningDuration, 0.0f, 1.0f ), 4 ) * maxGateScale;
    }
    else if ( m_State == HyperspaceGateState::Closed )
    {
        jumpRatio = 1.0f;
    }

    if ( m_Direction == HyperspaceJumpDirection::JumpIn )
    {
        m_GatePosition = glm::mix( hyperspaceGateEndPos, m_GatePosition, jumpRatio );
        m_GatePositionLocal = glm::mix( hyperspaceGateEndPosLocal, hyperspaceGatePosLocal, jumpRatio );
    }
    else
    {
        m_GatePosition = glm::mix( m_GatePosition, hyperspaceGateEndPos, jumpRatio );
        m_GatePositionLocal = glm::mix( hyperspaceGatePosLocal, hyperspaceGateEndPosLocal, jumpRatio );
    }
}

void HyperspaceGate::Render( const glm::mat4x4& shipTransform )
{
    if ( m_State != HyperspaceGateState::Invalid && m_State != HyperspaceGateState::Closed )
    {
        using namespace glm;

        const mat4x4 translation = translate( m_GatePositionLocal );
        const mat4x4 scaling = scale( glm::vec3( m_GateScale, 1.0f, 1.0f ) );
        const mat4x4 modelTransform( shipTransform * translation * scaling );

        Genesis::FrameWork::GetRenderSystem()->SetRenderTarget( Genesis::RenderTargetId::Glow );
        m_pModel->Render( modelTransform );

        Genesis::FrameWork::GetRenderSystem()->SetRenderTarget( Genesis::RenderTargetId::Default );
        m_pModel->Render( modelTransform );
    }
}

bool HyperspaceGate::HasClosed() const
{
    return m_State == HyperspaceGateState::Closed;
}

} // namespace Hexterminate