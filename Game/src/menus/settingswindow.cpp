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

#include <functional>

#include <configuration.h>
#include <rendersystem.h>

#include "hexterminate.h"
#include "menus/settingswindow.h"
#include "ui/button.h"
#include "ui/checkbox.h"
#include "ui/image.h"
#include "ui/inputarea.h"
#include "ui/meter.h"
#include "ui/panel.h"
#include "ui/radiobutton.h"
#include "ui/scrollingelement.h"
#include "ui/slider.h"
#include "ui/text.h"

namespace Hexterminate
{

SettingsWindow::SettingsWindow()
    : UI::Window( "Settings window" )
{
    UI::ScrollingElementSharedPtr pScrollingElement = std::make_shared<UI::ScrollingElement>( "Scrolling element" );
    pScrollingElement->GetScrollingArea()->Add( CreateAudioPanel() );
    pScrollingElement->GetScrollingArea()->Add( CreateVideoPanel() );
    pScrollingElement->GetScrollingArea()->Add( CreateGameplayPanel() );
    GetContentPanel()->Add( pScrollingElement );
}

void SettingsWindow::Reset()
{
    using namespace Genesis;

    UI::Window::Reset();

    m_pMasterVolumeSlider->SetValue( static_cast<float>( Configuration::GetMasterVolume() ) / 100.0f );
    m_pMusicVolumeSlider->SetValue( static_cast<float>( Configuration::GetMusicVolume() ) / 100.0f );
    m_pSFXVolumeSlider->SetValue( static_cast<float>( Configuration::GetSFXVolume() ) / 100.0f );
    m_pPostProcessBleachBypassCheckbox->SetChecked( Configuration::IsPostProcessingEffectEnabled( RenderSystem::PostProcessEffect::BleachBypass ) );
    m_pPostProcessGlowCheckbox->SetChecked( Configuration::IsPostProcessingEffectEnabled( RenderSystem::PostProcessEffect::Glow ) );
    m_pPostProcessVignetteCheckbox->SetChecked( Configuration::IsPostProcessingEffectEnabled( RenderSystem::PostProcessEffect::Vignette ) );
    m_pOutlinesCheckbox->SetChecked( Configuration::GetOutlines() );
    m_pFireToggleCheckbox->SetChecked( Configuration::GetFireToggle() );
}

UI::ElementSharedPtr SettingsWindow::CreateAudioPanel()
{
    UI::PanelSharedPtr pPanel = std::make_shared<UI::Panel>( "Audio panel" );
    pPanel->Add( std::make_shared<UI::Text>( "Title" ) );

    m_pMasterVolumeSlider = std::make_shared<UI::Slider>( "Master volume slider", [ this ]( float value ) { SetMasterVolume( value ); } );
    pPanel->Add( m_pMasterVolumeSlider );

    m_pMusicVolumeSlider = std::make_shared<UI::Slider>( "Music volume slider", [ this ]( float value ) { SetMusicVolume( value ); } );
    pPanel->Add( m_pMusicVolumeSlider );

    m_pSFXVolumeSlider = std::make_shared<UI::Slider>( "SFX volume slider", [ this ]( float value ) { SetSFXVolume( value ); } );
    pPanel->Add( m_pSFXVolumeSlider );

    pPanel->Add( std::make_shared<UI::Text>( "Master volume text" ) );
    pPanel->Add( std::make_shared<UI::Text>( "Music volume text" ) );
    pPanel->Add( std::make_shared<UI::Text>( "SFX volume text" ) );

    return pPanel;
}

UI::ElementSharedPtr SettingsWindow::CreateVideoPanel()
{
    using namespace Genesis;
    UI::PanelSharedPtr pPanel = std::make_shared<UI::Panel>( "Video panel" );
    pPanel->Add( std::make_shared<UI::Text>( "Title" ) );

    auto CreatePostProcessCheckbox = [ pPanel ]( const char* pName, RenderSystem::PostProcessEffect effect ) -> UI::CheckboxSharedPtr {
        UI::CheckboxSharedPtr pCheckbox = std::make_shared<UI::Checkbox>( pName,
            [ effect ]( bool checked ) {
                Configuration::EnablePostProcessEffect( effect, checked );
                FrameWork::GetRenderSystem()->EnablePostProcessEffect( effect, checked );
            } );
        pPanel->Add( pCheckbox );
        return pCheckbox;
    };

    m_pPostProcessBleachBypassCheckbox = CreatePostProcessCheckbox( "Post process (bleach bypass) checkbox", RenderSystem::PostProcessEffect::BleachBypass );
    m_pPostProcessGlowCheckbox = CreatePostProcessCheckbox( "Post process (glow) checkbox", RenderSystem::PostProcessEffect::Glow );
    m_pPostProcessVignetteCheckbox = CreatePostProcessCheckbox( "Post process (vignette) checkbox", RenderSystem::PostProcessEffect::Vignette );

    UI::PanelSharedPtr pPostProcessingPanel = std::make_shared<UI::Panel>( "Post processing description panel" );
    pPostProcessingPanel->Add( std::make_shared<UI::Text>( "Text" ) );
    pPanel->Add( pPostProcessingPanel );

    return pPanel;
}

UI::ElementSharedPtr SettingsWindow::CreateGameplayPanel()
{
    UI::PanelSharedPtr pPanel = std::make_shared<UI::Panel>( "Gameplay panel" );
    pPanel->Add( std::make_shared<UI::Text>( "Title" ) );

    m_pOutlinesCheckbox = std::make_shared<UI::Checkbox>( "Outlines checkbox",
        []( bool checked ) { Genesis::Configuration::SetOutline( checked ); } );
    pPanel->Add( m_pOutlinesCheckbox );

    UI::PanelSharedPtr pOutlinesPanel = std::make_shared<UI::Panel>( "Outlines description panel" );
    pOutlinesPanel->Add( std::make_shared<UI::Text>( "Text" ) );
    pPanel->Add( pOutlinesPanel );

    m_pFireToggleCheckbox = std::make_shared<UI::Checkbox>( "Fire toggle checkbox",
        []( bool checked ) { Genesis::Configuration::SetFireToggle( checked ); } );
    pPanel->Add( m_pFireToggleCheckbox );

    UI::PanelSharedPtr pFireTogglePanel = std::make_shared<UI::Panel>( "Fire toggle description panel" );
    pFireTogglePanel->Add( std::make_shared<UI::Text>( "Text" ) );
    pPanel->Add( pFireTogglePanel );

    return pPanel;
}

void SettingsWindow::SetMasterVolume( float value )
{
    Genesis::Configuration::SetMasterVolume( static_cast<unsigned int>( value * 100.0f ) );
}

void SettingsWindow::SetMusicVolume( float value )
{
    Genesis::Configuration::SetMusicVolume( static_cast<unsigned int>( value * 100.0f ) );
}

void SettingsWindow::SetSFXVolume( float value )
{
    Genesis::Configuration::SetSFXVolume( static_cast<unsigned int>( value * 100.0f ) );
}

} // namespace Hexterminate
