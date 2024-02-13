// Copyright 2021 Pedro Nunes
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

#include <configuration.h>
#include <scene/layer.h>
#include <scene/scene.h>
#include <shadercache.h>
#include <shaderuniform.h>

#include "hexterminate.h"
#include "hyperscape/hyperscaperep.h"
#include "menus/popup.h"
#include "sector/sector.h"

namespace Hexterminate
{

//-----------------------------------------------------------------------------
// HyperscapeRep
//-----------------------------------------------------------------------------

HyperscapeRep::HyperscapeRep( Hyperscape* pHyperscape )
    : m_pHyperscape( pHyperscape )
    , m_Show( false )
    , m_pBackgroundShader( nullptr )
    , m_pBackgroundVB( nullptr )
    , m_pGridShader( nullptr )
    , m_pGridDiffuseSampler( nullptr )
    , m_pGridParallax( nullptr )
    , m_pGridVB( nullptr )
    , m_pLayer( nullptr )
    , m_Parallax( 0.0f, 0.0f )
    , m_WasBleachBypassEnabled( true )
    , m_WasVignetteEnabled( true )
{
    using namespace Genesis;

    BuildBackground();

    m_LeftMouseButtonDownToken = FrameWork::GetInputManager()->AddMouseCallback( std::bind( &HyperscapeRep::OnLeftMouseButtonDown, this ), MouseButton::Left, ButtonState::Pressed );

    m_WasBleachBypassEnabled = Configuration::IsPostProcessingEffectEnabled( RenderSystem::PostProcessEffect::BleachBypass );
    if ( m_WasBleachBypassEnabled )
    {
        FrameWork::GetRenderSystem()->EnablePostProcessEffect( RenderSystem::PostProcessEffect::BleachBypass, false );
    }

    m_WasVignetteEnabled = Configuration::IsPostProcessingEffectEnabled( RenderSystem::PostProcessEffect::Vignette );
    if ( m_WasVignetteEnabled )
    {
        FrameWork::GetRenderSystem()->EnablePostProcessEffect( RenderSystem::PostProcessEffect::Vignette, false );
    }
}

HyperscapeRep::~HyperscapeRep()
{
    using namespace Genesis;

    FrameWork::GetInputManager()->RemoveMouseCallback( m_LeftMouseButtonDownToken );

    delete m_pBackgroundVB;
    delete m_pGridVB;

    if ( m_WasBleachBypassEnabled )
    {
        FrameWork::GetRenderSystem()->EnablePostProcessEffect( RenderSystem::PostProcessEffect::BleachBypass, true );
    }

    if ( m_WasVignetteEnabled )
    {
        FrameWork::GetRenderSystem()->EnablePostProcessEffect( RenderSystem::PostProcessEffect::Vignette, true );
    }
}

void HyperscapeRep::Initialise()
{
    Genesis::Scene* pScene = Genesis::FrameWork::GetScene();
    m_pLayer = pScene->AddLayer( LAYER_GALAXY, true );
    m_pLayer->AddSceneObject( this, false );
    Show( true );
}

void HyperscapeRep::RemoveFromScene()
{
    m_pLayer->RemoveSceneObject( this );
}

void HyperscapeRep::Update( float delta )
{
    if ( m_Show == false )
    {
        return;
    }

    UpdateInput();

    // if ( m_ExitMenu )
    //{
    //	PopupState state = g_pGame->GetPopup()->GetState();
    //	if ( state == PopupState::Yes )
    //	{
    //		m_ExitMenu = false;
    //		g_pGame->EndGame();
    //	}
    //	else if ( state == PopupState::No )
    //	{
    //		m_pGalaxyWindow->Show( true );
    //		m_ExitMenu = false;
    //	}
    // }
}

void HyperscapeRep::UpdateInput()
{
    using namespace Genesis;
    Genesis::InputManager* pInputManager = Genesis::FrameWork::GetInputManager();

    if ( pInputManager->IsButtonPressed( SDL_SCANCODE_ESCAPE ) )
    {
        g_pGame->GetPopup()->Show( PopupMode::YesNo, "Exit to main menu?" );
    }

    glm::vec2 mousePosition = pInputManager->GetMousePosition();
    m_Parallax.x = mousePosition.x / static_cast<float>( Configuration::GetScreenWidth() );
    m_Parallax.y = mousePosition.y / static_cast<float>( Configuration::GetScreenHeight() );
}

void HyperscapeRep::Render()
{
    using namespace Genesis;

    if ( m_Show )
    {
        DrawBackground();
    }
}

void HyperscapeRep::BuildBackground()
{
    using namespace Genesis;
    m_pBackgroundShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "hyperscape_background" );

    ShaderUniform* pBackgroundUniform = m_pBackgroundShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
    pBackgroundUniform->Set( FrameWork::GetResourceManager()->GetResource<ResourceImage*>( "data/backgrounds/hyperscape.jpg" ), GL_TEXTURE0 );

    ShaderUniform* pNoiseUniform = m_pBackgroundShader->RegisterUniform( "k_sampler1", ShaderUniformType::Texture );
    pNoiseUniform->Set( FrameWork::GetResourceManager()->GetResource<ResourceImage*>( "data/images/noise1.png" ), GL_TEXTURE1 );

    m_pBackgroundVB = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV );
    m_pBackgroundVB->CreateTexturedQuad( 0.0f, 0.0f, static_cast<float>( Configuration::GetScreenWidth() ), static_cast<float>( Configuration::GetScreenHeight() ) );
}

void HyperscapeRep::DrawBackground()
{
    m_pBackgroundShader->Use();
    m_pBackgroundVB->Draw();
}

void HyperscapeRep::Show( bool state )
{
    if ( state != IsVisible() )
    {
        m_Show = state;

        // Enabling the galaxy view disables the rendering of all other scene layers
        Genesis::Scene* pScene = Genesis::FrameWork::GetScene();
        pScene->SetLayerMask( state ? LAYER_GALAXY : ( ~LAYER_GALAXY ) );
    }
}

void HyperscapeRep::OnLeftMouseButtonDown()
{
}

} // namespace Hexterminate
