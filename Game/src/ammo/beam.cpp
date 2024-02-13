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

#include <cassert>

#include <glm/gtc/matrix_access.hpp>

#include <math/constants.h>
#include <math/misc.h>
#include <rendersystem.h>
#include <resources/resourceimage.h>
#include <shadercache.h>
#include <shaderuniform.h>
#include <vertexbuffer.h>

#include "ammo/beam.h"
#include "misc/mathaux.h"

namespace Hexterminate
{

Genesis::Shader* Beam::m_pShader = nullptr;
Genesis::Shader* Beam::m_pFlareShader = nullptr;

Beam::Beam()
    : m_pBeamVertexBuffer( nullptr )
    , m_pFlareVertexBuffer( nullptr )
{
}

Beam::~Beam()
{
    delete m_pBeamVertexBuffer;
    delete m_pFlareVertexBuffer;
}

void Beam::Create( Weapon* pWeapon, float additionalRotation /* = 0.0f */ )
{
    Ammo::Create( pWeapon, additionalRotation );

    m_IsGlowSource = true;
    m_DiesOnHit = false;

    SetupBeam();
    SetupBeamFlare();
}

void Beam::SetupBeam()
{
    using namespace Genesis;

    m_pBeamVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );

    if ( m_pShader == nullptr )
    {
        RenderSystem* pRenderSystem = FrameWork::GetRenderSystem();
        m_pShader = pRenderSystem->GetShaderCache()->Load( "beam" );
        ResourceImage* pTexture = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/images/beam.jpg" );
        ShaderUniform* pTextureSamplerUniform = m_pShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
        pTextureSamplerUniform->Set( pTexture, GL_TEXTURE0 );
    }
}

void Beam::SetupBeamFlare()
{
    using namespace Genesis;

    m_pFlareVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );

    if ( m_pFlareShader == nullptr )
    {
        RenderSystem* pRenderSystem = FrameWork::GetRenderSystem();
        m_pFlareShader = pRenderSystem->GetShaderCache()->Load( "beamflare" );
        ResourceImage* pTexture = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/images/beamflare.png" );
        ResourceImage* pMask = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/images/beamflaremask.png" );
        ShaderUniform* pTextureSamplerUniform = m_pFlareShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
        ShaderUniform* pMaskSamplerUniform = m_pFlareShader->RegisterUniform( "k_sampler1", ShaderUniformType::Texture );
        pTextureSamplerUniform->Set( pTexture, GL_TEXTURE0 );
        pMaskSamplerUniform->Set( pMask, GL_TEXTURE1 );
    }
}

void Beam::Update( float delta )
{
    m_Angle = m_pOwner->GetAngle() + m_AdditionalRotation;

    glm::mat4x4 weaponTransform = m_pOwner->GetWorldTransform();

    m_MuzzleOffset = m_pOwner->GetMuzzleOffset();
    m_pOwner->MarkMuzzleAsUsed();
    glm::mat4x4 muzzleOffsetTransform = glm::translate( m_MuzzleOffset );

    weaponTransform = weaponTransform * muzzleOffsetTransform;

    glm::vec3 weaponPosition( glm::column( weaponTransform, 3 ) );
    glm::vec3 weaponForward( glm::column( weaponTransform, 1 ) );
    Math::RotateVector( weaponForward, Genesis::kDegToRad * m_AdditionalRotation );

    m_Src = weaponPosition;
    m_Dst = m_Src + weaponForward * m_RayLength;
    m_Dir = weaponForward;
}

void Beam::Render()
{
    using namespace glm;
    using namespace Genesis;

    if ( GetHitFraction() <= 0.0f )
    {
        return;
    }

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Add );

    const mat4 translation = translate( m_Src );
    const mat4 rotation = rotate( mat4( 1.0f ), -m_Angle, vec3( 0.0f, 0.0f, 1.0f ) );
    const mat4 modelMatrix = translation * rotation;

    const float opacity = GetOpacity();
    const Genesis::Color& beamColour = m_pOwner->GetInfo()->GetBeamColour();

    RenderBeam( modelMatrix, beamColour, opacity );
    RenderBeamFlare( modelMatrix, beamColour, opacity );

    FrameWork::GetRenderSystem()->SetBlendMode( BlendMode::Disabled );
}

void Beam::RenderBeam( const glm::mat4& modelMatrix, const Genesis::Color& beamColour, float opacity )
{
    using namespace Genesis;

    PositionData posData;
    UVData uvData;
    ColourData colourData;

    const float u = 1.0f;
    const float v = 1.0f;
    const float hw = m_pOwner->GetInfo()->GetBeamWidth() / 2.0f;
    const float l = m_pOwner->GetInfo()->GetRange( m_pOwner->GetOwner() ) * GetHitFraction();
    const float offset = m_pOwner->GetInfo()->GetBeamWidth() * 0.5f;

    const glm::vec4 colour( beamColour.r, beamColour.g, beamColour.b, opacity );
    for ( int i = 0; i < 6; ++i )
    {
        colourData.push_back( colour );
    }

    posData.emplace_back( -hw, offset, 0.0f ); // 0
    posData.emplace_back( -hw, l, 0.0f ); // 1
    posData.emplace_back( hw, l, 0.0f ); // 2
    posData.emplace_back( -hw, offset, 0.0f ); // 0
    posData.emplace_back( hw, l, 0.0f ); // 2
    posData.emplace_back( hw, offset, 0.0f ); // 3

    uvData.emplace_back( 0.0f, 0.0f ); // 0
    uvData.emplace_back( 0.0f, v ); // 1
    uvData.emplace_back( u, v ); // 2
    uvData.emplace_back( 0.0f, 0.0f ); // 0
    uvData.emplace_back( u, v ); // 2
    uvData.emplace_back( u, 0.0f ); // 3

    m_pBeamVertexBuffer->CopyPositions( posData );
    m_pBeamVertexBuffer->CopyUVs( uvData );
    m_pBeamVertexBuffer->CopyColours( colourData );

    m_pShader->Use( modelMatrix );
    m_pBeamVertexBuffer->Draw();
}

void Beam::RenderBeamFlare( const glm::mat4& modelMatrix, const Genesis::Color& beamColour, float opacity )
{
    const float w = m_pOwner->GetInfo()->GetBeamWidth() * 1.5f;
    m_pFlareVertexBuffer->CreateTexturedQuad( -w, w, w * 2.0f, -w, beamColour.glm() );
    m_pFlareShader->Use( modelMatrix );
    m_pFlareVertexBuffer->Draw();
}

} // namespace Hexterminate