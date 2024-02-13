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

#include <cassert>

#include <math/constants.h>
#include <math/misc.h>
#include <rendersystem.h>
#include <resources/resourceimage.h>
#include <shadercache.h>
#include <shaderuniform.h>
#include <vertexbuffer.h>

#include "ammo/antiproton.h"
#include "hexterminate.h"
#include "misc/mathaux.h"
#include "muzzleflash/muzzleflashmanager.h"
#include "sector/sector.h"
#include "shadertweaks.h"

namespace Hexterminate
{

Genesis::Shader* Antiproton::m_pShader = nullptr;

Antiproton::Antiproton()
    : m_pVertexBuffer( nullptr )
    , m_QuantumState( QuantumState::Black )
{
}

Antiproton::~Antiproton()
{
    delete m_pVertexBuffer;
}

void Antiproton::Create( Weapon* pWeapon, float additionalRotation /* = 0.0f */ )
{
    using namespace Genesis;

    Ammo::Create( pWeapon, additionalRotation );

    m_IsGlowSource = false;
    m_DiesOnHit = true;

    float muzzleflashScale = pWeapon->GetInfo()->GetMuzzleflashScale();
    g_pGame->GetCurrentSector()->GetMuzzleflashManager()->Add(
        MuzzleflashData( pWeapon, m_MuzzleOffset, 0, gRand( muzzleflashScale * 0.8f, muzzleflashScale * 1.2f ), gRand( 0.075f, 0.125f ) ) );

    AddonQuantumStateAlternator* pAlternator = pWeapon->GetOwner()->GetQuantumStateAlternator();
    if ( pAlternator != nullptr )
    {
        m_QuantumState = pAlternator->GetQuantumState();

        if ( m_QuantumState == QuantumState::Inactive )
        {
            m_QuantumState = QuantumState::Black;
        }
    }

    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );

    if ( m_pShader == nullptr )
    {
        RenderSystem* pRenderSystem = FrameWork::GetRenderSystem();
        m_pShader = pRenderSystem->GetShaderCache()->Load( "antiproton" );
    }

    m_pInternalRadiusUniform = m_pShader->RegisterUniform( "k_internalRadius", ShaderUniformType::Float );
    m_pExternalRadiusUniform = m_pShader->RegisterUniform( "k_externalRadius", ShaderUniformType::Float );
}

void Antiproton::Update( float delta )
{
    m_pInternalRadiusUniform->Set( ShaderTweaksDebugWindow::GetAntiprotonInternalRadius() );
    m_pExternalRadiusUniform->Set( ShaderTweaksDebugWindow::GetAntiprotonExternalRadius() );

    if ( m_Range <= 0.0f )
    {
        Kill();
    }
    else
    {
        m_Src += m_Dir * m_Speed * delta;
        m_Dst += m_Dir * m_Speed * delta;
        m_Range -= m_Speed * delta;
    }
}

void Antiproton::Render()
{
    using namespace glm;
    using namespace Genesis;

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Blend );

    const mat4 modelMatrix = translate( m_Src );

    const float w = ShaderTweaksDebugWindow::GetAntiprotonGeometrySize();
    m_pVertexBuffer->CreateTexturedQuad( -w, w, w * 2.0f, -w * 2.0f, m_QuantumState == QuantumState::Black ? vec4( 1 ) : vec4( 0 ) );
    m_pShader->Use( modelMatrix );
    m_pVertexBuffer->Draw();

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Disabled );
}

QuantumState Antiproton::GetQuantumState() const
{
    return m_QuantumState;
}

} // namespace Hexterminate