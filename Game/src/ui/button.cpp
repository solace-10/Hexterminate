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

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <gui/gui.h>

#include "ui/button.h"
#include "ui/fonts.h"
#include "ui/togglegroup.h"

namespace Hexterminate::UI
{

static const char* sPropertyDefaultImage = "default_image";
static const char* sPropertyHoveredImage = "hovered_image";
static const char* sPropertyPressedImage = "pressed_image";
static const char* sPropertyLabel = "label";

Button::Button( const std::string& name, ButtonPressedCallback onPressed, std::any userData /* = {} */, ToggleGroupWeakPtr pToggleGroup /* = {} */ )
    : Element( name )
    , m_pImage( nullptr )
    , m_pDefault( nullptr )
    , m_pHovered( nullptr )
    , m_pPressed( nullptr )
    , m_pText( nullptr )
    , m_Label( "???" )
    , m_OnPressed( onPressed )
    , m_UserData( userData )
    , m_pToggleGroup( pToggleGroup )
    , m_State( State::Unselected )
{
    using namespace Genesis;

    GetPanel()->SetColour( 0.0f, 0.0f, 0.0f, 0.0f );
    GetPanel()->SetBorderColour( 0.0f, 0.0f, 0.0f, 0.0f );
    m_pImage = new Genesis::Gui::Image();
    GetPanel()->AddElement( m_pImage );

    m_pText = new Genesis::Gui::Text();
    m_pText->SetSize( 256.0f, 16.0f );
    m_pText->SetPosition( 8.0f, 2.0f );
    m_pText->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
    m_pText->SetMultiLine( false );
    m_pText->SetFont( reinterpret_cast<ResourceFont*>( FrameWork::GetResourceManager()->GetResource( "data/fonts/kimberley18.fnt" ) ) );
    m_pText->SetText( m_Label );
    m_pImage->AddElement( m_pText );
    CenterText();

    m_LeftClickPressedToken = FrameWork::GetInputManager()->AddMouseCallback( std::bind( &Button::OnLeftClickPressed, this ), MouseButton::Left, ButtonState::Pressed );

    m_pClickSFX = Genesis::Gui::LoadSFX( "data/sfx/beep.wav" );
    m_pHoverSFX = Genesis::Gui::LoadSFX( "data/sfx/hover.wav" );
}

Button::~Button()
{
    Genesis::FrameWork::GetInputManager()->RemoveMouseCallback( m_LeftClickPressedToken );
}

void Button::Update()
{
    Element::Update();

    if ( IsAcceptingInput() == false )
    {
        if ( m_State == State::Hovered )
        {
            m_State = State::Unselected;
            m_pImage->SetTexture( m_pDefault );
        }
    }
    else if ( m_State == State::Disabled )
    {
        m_pImage->SetTexture( m_pDefault );
    }
    else if ( m_State == State::Unselected && IsHovered() )
    {
        m_State = State::Hovered;
        m_pImage->SetTexture( m_pHovered );
        Genesis::Gui::PlaySFX( m_pHoverSFX );
    }
    else if ( ( m_State == State::Hovered || m_State == State::Unselected ) && !IsHovered() )
    {
        m_State = State::Unselected;
        m_pImage->SetTexture( m_pDefault );
    }
}

void Button::LoadResources()
{
    using namespace Genesis;

    if ( m_DefaultPath.empty() == false )
    {
        m_pDefault = static_cast<ResourceImage*>( FrameWork::GetResourceManager()->GetResource( m_DefaultPath ) );
        if ( m_pDefault == nullptr )
        {
            FrameWork::GetLogger()->LogWarning( "Couldn't load '%s'.", m_DefaultPath.c_str() );
        }
        else
        {
            m_pImage->SetTexture( m_pDefault );
            glm::vec2 size( static_cast<float>( m_pDefault->GetWidth() ), static_cast<float>( m_pDefault->GetHeight() ) );
            m_pImage->SetSize( size );
            GetPanel()->SetSize( size );
        }
    }

    if ( m_HoveredPath.empty() == false )
    {
        m_pHovered = static_cast<ResourceImage*>( FrameWork::GetResourceManager()->GetResource( m_HoveredPath ) );
        if ( m_pHovered == nullptr )
        {
            FrameWork::GetLogger()->LogWarning( "Couldn't load '%s'.", m_HoveredPath.c_str() );
        }
    }

    if ( m_PressedPath.empty() == false )
    {
        m_pPressed = static_cast<ResourceImage*>( FrameWork::GetResourceManager()->GetResource( m_PressedPath ) );
        if ( m_pPressed == nullptr )
        {
            FrameWork::GetLogger()->LogWarning( "Couldn't load '%s'.", m_PressedPath.c_str() );
        }
        else if ( m_State == State::Selected )
        {
            m_pImage->SetTexture( m_pPressed );
        }
    }
}

void Button::SetDefaultImage( const std::string& path )
{
    m_DefaultPath = path;
    LoadResources();
}

void Button::SetHoveredImage( const std::string& path )
{
    m_HoveredPath = path;
    LoadResources();
}

void Button::SetPressedImage( const std::string& path )
{
    m_PressedPath = path;
    LoadResources();
}

void Button::SetText( const std::string& text, bool centerHorizontally /* = true */, bool centerVertically /* = true */ )
{
    m_pText->SetText( text );
    CenterText( centerHorizontally, centerVertically );
}

void Button::SetFont( const std::string& fontName )
{
    Genesis::ResourceFont* pFont = Fonts::Get( fontName );
    if ( pFont == nullptr )
    {
        Genesis::FrameWork::GetLogger()->LogWarning( "Couldn't find font '%s' for UI element '%s'.", fontName.c_str(), GetName().c_str() );
    }
    else
    {
        m_pText->SetFont( pFont );
    }
}

void Button::SaveProperties( json& properties )
{
    Element::SaveProperties( properties );

    properties[ sPropertyDefaultImage ] = m_DefaultPath;
    properties[ sPropertyHoveredImage ] = m_HoveredPath;
    properties[ sPropertyPressedImage ] = m_PressedPath;
    properties[ sPropertyLabel ] = m_Label;
}

void Button::LoadProperties( const json& properties )
{
    Element::LoadProperties( properties );

    if ( properties.contains( sPropertyDefaultImage ) )
    {
        m_DefaultPath = properties[ sPropertyDefaultImage ].get<std::string>();
    }

    if ( properties.contains( sPropertyHoveredImage ) )
    {
        m_HoveredPath = properties[ sPropertyHoveredImage ].get<std::string>();
    }

    if ( properties.contains( sPropertyPressedImage ) )
    {
        m_PressedPath = properties[ sPropertyPressedImage ].get<std::string>();
    }

    if ( properties.contains( sPropertyLabel ) )
    {
        m_Label = properties[ sPropertyLabel ].get<std::string>();
        m_pText->SetText( m_Label );
    }

    LoadResources();
    CenterText();
}

void Button::RenderProperties()
{
    Element::RenderProperties();

    if ( ImGui::CollapsingHeader( "Button", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        ImGui::InputText( "Default image", &m_DefaultPath );
        ImGui::InputText( "Hovered image", &m_HoveredPath );
        ImGui::InputText( "Pressed image", &m_PressedPath );
        if ( ImGui::Button( "Reload images" ) )
        {
            LoadResources();
        }

        ImGui::InputText( "Label", &m_Label );
        m_pText->SetText( m_Label );
        CenterText();
    }
}

void Button::CenterText( bool horizontally /* = true */, bool vertically /* = true */ )
{
    if ( m_pDefault == nullptr )
    {
        return;
    }

    Genesis::ResourceFont* pFont = m_pText->GetFont();
    const float w = pFont->GetTextLength( m_Label );
    const float h = pFont->GetLineHeight();
    m_pText->SetSize( w, h );

    const float x = horizontally ? ( static_cast<float>( m_pDefault->GetWidth() ) - w ) / 2.0f : 8.0f;
    const float y = vertically ? ( static_cast<float>( m_pDefault->GetHeight() ) - h ) / 2.0f : 4.0f;
    m_pText->SetPosition( x, y );
}

void Button::OnLeftClickPressed()
{
    if ( GetPanel()->IsMouseInside() && m_State != State::Disabled && IsAcceptingInput() )
    {
        Genesis::Gui::PlaySFX( m_pClickSFX );
        ToggleGroupSharedPtr pToggleGroup = m_pToggleGroup.lock();
        if ( pToggleGroup == nullptr )
        {
            if ( m_OnPressed != nullptr )
            {
                m_OnPressed( m_UserData );
            }
        }
        else
        {
            pToggleGroup->Select( this );
        }
    }
}

void Button::Toggle( bool state )
{
    if ( m_State == State::Disabled )
    {
        return;
    }

    if ( state == true && m_State != State::Selected )
    {
        m_State = State::Selected;
        m_pText->SetColour( 0.0f, 0.0f, 0.0f, 0.9f );
        m_pImage->SetTexture( m_pPressed );

        if ( m_OnPressed != nullptr )
        {
            m_OnPressed( m_UserData );
        }
    }
    else if ( state == false )
    {
        m_State = State::Unselected;
        m_pText->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
        m_pImage->SetTexture( m_pDefault );
    }
}

void Button::Enable( bool state )
{
    if ( state == true && m_State == State::Disabled )
    {
        m_State = State::Unselected;
        m_pText->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
    }
    else if ( state == false && m_State != State::Disabled )
    {
        m_State = State::Disabled;
        m_pText->SetColour( 0.5f, 0.5f, 0.5f, 1.0f );
    }
}

void Button::SetUserData( const std::any& userData )
{
    m_UserData = userData;
}

} // namespace Hexterminate::UI
