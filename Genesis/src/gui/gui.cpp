// Copyright 2016 Pedro Nunes
//
// This file is part of Genesis.
//
// Genesis is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Genesis is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Genesis. If not, see <http://www.gnu.org/licenses/>.

#include "sound/soundmanager.h"
#include "gui.h"
#include "../configuration.h"
#include "../genesis.h"
#include "../imgui/imgui_impl.h"
#include "../rendersystem.h"
#include "../resources/resourcesound.h"
#include "../shader.h"
#include "../shadercache.h"
#include "../shaderuniform.h"
#include "../vertexbuffer.h"

namespace Genesis
{
namespace Gui
{

    ///////////////////////////////////////////////////////////////////////////
    // Miscellaneous auxiliary functions
    ///////////////////////////////////////////////////////////////////////////

    ResourceSound* LoadSFX( const std::string& filename )
    {
        using namespace std::literals;
        ResourceSound* pSFX = (ResourceSound*)FrameWork::GetResourceManager()->GetResource( filename );
        if ( pSFX != nullptr )
        {
            pSFX->Initialise( SOUND_FLAG_FX );
            pSFX->SetInstancingLimit( 100ms );
            return pSFX;
        }
        else
        {
            FrameWork::GetLogger()->LogWarning( "Couldn't load SFX '%s'", filename.c_str() );
            return nullptr;
        }
    }

    void PlaySFX( Genesis::ResourceSound* pSFX )
    {
        if ( pSFX != nullptr )
        {
            FrameWork::GetSoundManager()->CreateSoundInstance( pSFX, Genesis::Sound::SoundBus::Type::SFX );
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // GuiManager
    ///////////////////////////////////////////////////////////////////////////

    Shader* GuiManager::m_pUntexturedShader = nullptr;
    ShaderUniform* GuiManager::m_pUntexturedColourUniform = nullptr;
    Shader* GuiManager::m_pTexturedShader = nullptr;
    ShaderUniform* GuiManager::m_pTexturedSamplerUniform = nullptr;
    ShaderUniform* GuiManager::m_pTexturedColourUniform = nullptr;

    GuiManager::GuiManager()
        : m_pCursor( nullptr )
        , mFocusedInputArea( nullptr )
    {
    }

    GuiManager::~GuiManager()
    {
        for ( auto& pChildElement : mChildren )
        {
            delete pChildElement;
        }

        delete m_pCursor;
    }

    void GuiManager::Initialize()
    {
        ShaderCache* pShaderCache = FrameWork::GetRenderSystem()->GetShaderCache();
        m_pUntexturedShader = pShaderCache->Load( "gui_untextured" );
        m_pUntexturedColourUniform = m_pUntexturedShader->RegisterUniform( "k_colour", ShaderUniformType::FloatVector4 );
        m_pTexturedShader = pShaderCache->Load( "gui_textured" );
        m_pTexturedColourUniform = m_pTexturedShader->RegisterUniform( "k_colour", ShaderUniformType::FloatVector4 );
        m_pTexturedSamplerUniform = m_pTexturedShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );

        m_pCursor = new Cursor();
    }

    // Takes ownership of the element!
    void GuiManager::AddElement( GuiElement* pElement )
    {
        if ( pElement == nullptr )
        {
            FrameWork::GetLogger()->LogError( "Attempting to add null GuiElement." );
            return;
        }

        SDL_assert( pElement->IsOrphan() );

        GuiElement::SortedInsertByDepth( pElement, mChildren );
        pElement->OnParentSet();
    }

    TaskStatus GuiManager::Update( float delta )
    {
        // Delete all the pending "to remove" GUI elements
        if ( !mToRemove.empty() )
        {
            for ( GuiElementList::iterator it = mToRemove.begin(); it != mToRemove.end(); it++ )
            {
                for ( GuiElementList::iterator it2 = mChildren.begin(); it2 != mChildren.end(); it2++ )
                {
                    if ( *it == *it2 )
                    {
                        mChildren.erase( it2 );
                        break;
                    }
                }
                delete *it;
            }
            mToRemove.clear();
        }

        // Update all children
        for ( auto& pChildElement : mChildren )
        {
            if ( pChildElement->IsVisible() )
                pChildElement->Update( delta );
        }

        // Update the cursor
        m_pCursor->Update( delta );

        return TaskStatus::Continue;
    }

    TaskStatus GuiManager::Render( float delta )
    {
        RenderSystem* renderSystem = FrameWork::GetRenderSystem();

        glDisable( GL_DEPTH_TEST );
        glEnable( GL_SCISSOR_TEST );

        renderSystem->ViewOrtho();

        for ( auto& pChildElement : mChildren )
        {
            if ( pChildElement->IsVisible() == false )
                continue;

            renderSystem->SetBlendMode( pChildElement->GetBlendMode() );
            pChildElement->UpdateClipRectangle();
            pChildElement->Render();
        }

        // The cursor should be rendered after everything else
		if ( m_pCursor->IsVisible() && ImGuiImpl::IsEnabled() == false )
        {
            renderSystem->SetBlendMode( BlendMode::Blend );
            m_pCursor->UpdateClipRectangle();
            m_pCursor->Render();
        }

        renderSystem->ViewPerspective();
        renderSystem->SetBlendMode( BlendMode::Disabled );

        glDisable( GL_SCISSOR_TEST );

        return TaskStatus::Continue;
    }

    void GuiManager::SetFocusedInputArea( InputArea* pInputArea )
    {
        if ( pInputArea == nullptr )
        {
            SDL_StopTextInput();
        }
        else
        {
            SDL_StartTextInput();
        }

        mFocusedInputArea = pInputArea;
    }

    ///////////////////////////////////////////////////////////////////////////
    // GuiElement
    ///////////////////////////////////////////////////////////////////////////

    GuiElement::GuiElement()
        : mPosition( 0.0f, 0.0f )
        , mPositionAbsolute( 0.0f, 0.0f )
        , mSize( 0.0f, 0.0f )
        , mParent( nullptr )
        , mBlendMode( BlendMode::Blend )
        , mShow( true )
        , mOrphan( true )
        , mAcceptsInput( true )
        , mHighlighted( false )
        , mHiddenForCapture( false )
        , mDepth( 0 )
    {
    }

    GuiElement::~GuiElement()
    {
        for ( auto& pChildElement : mChildren )
        {
            delete pChildElement;
        }
    }

    void GuiElement::SetDepth( int value )
    {
        SDL_assert( IsOrphan() );
        mDepth = value;
    }

    void GuiElement::OnParentSet()
    {
        mOrphan = false;
    }

    void GuiElement::SortedInsertByDepth( GuiElement* pElement, GuiElementList& elementList )
    {
        for ( GuiElementList::iterator it = elementList.begin(); it != elementList.end(); ++it )
        {
            if ( ( *it )->GetDepth() > pElement->GetDepth() )
            {
                elementList.insert( it, pElement );
                return;
            }
        }

        elementList.push_back( pElement );
    }

    void GuiElement::AddElement( GuiElement* pElement )
    {
        if ( pElement == nullptr )
        {
            FrameWork::GetLogger()->LogError( "Attempting to add null GuiElement." );
            return;
        }

        SDL_assert( pElement->IsOrphan() );
		SDL_assert( pElement != this );

        SortedInsertByDepth( pElement, mChildren );
        pElement->mParent = this;
        pElement->SetPosition( pElement->GetPosition() ); // update the position after the hierarchy changed
    }

    void GuiElement::RemoveElement( GuiElement* pElementToRemove )
    {
#ifdef _DEBUG
        bool found = false;
        for ( auto& pChildElement : mChildren )
        {
            if ( pChildElement == pElementToRemove )
            {
                found = true;
                break;
            }
        }

        SDL_assert( found );
#endif

        mToRemove.push_back( pElementToRemove );
    }

    void GuiElement::Update( float delta )
    {
        if ( !mShow )
            return;

        for ( auto& pChildElement : mChildren )
        {
            if ( pChildElement->IsVisible() )
                pChildElement->Update( delta );
        }

        if ( !mToRemove.empty() )
        {
            for ( auto& pElementToRemove : mToRemove )
            {
                for ( GuiElementList::iterator it = mChildren.begin(); it != mChildren.end(); )
                {
                    if ( pElementToRemove == *it )
                    {
                        delete pElementToRemove;
                        it = mChildren.erase( it );
                    }
                    else
                    {
                        it++;
                    }
                }
            }
            mToRemove.clear();
        }
    }

    void GuiElement::Render()
    {
        if ( mHighlighted )
        {
            RenderHighlight();
        }

        GuiElementList::const_iterator itEnd = mChildren.end();
        for ( GuiElementList::const_iterator it = mChildren.begin(); it != itEnd; it++ )
        {
            if ( ( *it )->IsVisible() == false )
                continue;

            FrameWork::GetRenderSystem()->SetBlendMode( ( *it )->GetBlendMode() );
            ( *it )->UpdateClipRectangle();
            ( *it )->Render();
        }
    }

    void GuiElement::RenderHighlight()
    {
        glm::vec2 pos = GetPositionAbsolute();

        // Allows sharp lines when FSAA is enabled
        pos.x += 0.5f;
        pos.y += 0.5f;

        glColor4f( 1.0f, 0.0f, 0.0f, 0.5f );
        glBegin( GL_LINE_LOOP );
        glVertex2f( pos.x + 1.0f, pos.y );
        glVertex2f( pos.x + 1.0f, pos.y + mSize.y - 1.0f );
        glVertex2f( pos.x + mSize.x - 1.0f, pos.y + mSize.y - 1.0f );
        glVertex2f( pos.x + mSize.x - 1.0f, pos.y );
        glEnd();
    }

    void GuiElement::UpdateClipRectangle()
    {
        if ( !mParent )
        {
            // If we don't have a parent, then we are the root for the clipping calculations.
            // So the clipping rectangle should have the size of the whole element.
            mClipRectangle.min = GetPosition();
            mClipRectangle.max = mClipRectangle.min + GetSize();
        }
        else
        {
            mClipRectangle.min = mParent->GetPositionAbsolute();
            mClipRectangle.max = mClipRectangle.min + mParent->GetSize();

            // Limit to parent's clip rectangle to prevent overflowing
            if ( mClipRectangle.min.x < mParent->mClipRectangle.min.x )
                mClipRectangle.min.x = mParent->mClipRectangle.min.x;
            if ( mClipRectangle.min.y < mParent->mClipRectangle.min.y )
                mClipRectangle.min.y = mParent->mClipRectangle.min.y;
            if ( mClipRectangle.max.x > mParent->mClipRectangle.max.x )
                mClipRectangle.max.x = mParent->mClipRectangle.max.x;
            if ( mClipRectangle.max.y > mParent->mClipRectangle.max.y )
                mClipRectangle.max.y = mParent->mClipRectangle.max.y;
        }

        // glScissor origin is on the bottom left corner of the screen...
        const int width = std::max( 0,  static_cast<int>( mClipRectangle.max.x - mClipRectangle.min.x ) );
        const int height = std::max( 0, static_cast<int>( mClipRectangle.max.y - mClipRectangle.min.y ) );
        glScissor(
            static_cast<int>( mClipRectangle.min.x ),
            static_cast<int>( Configuration::GetScreenHeight() ) - static_cast<int>( mClipRectangle.max.y ),
            width,
            height );
    }

    void GuiElement::SetPosition( const glm::vec2& position )
    {
        mPosition = position;
        UpdatePosition();
    }

    void GuiElement::SetPosition( int x, int y )
    {
        mPosition.x = (float)x;
        mPosition.y = (float)y;
        UpdatePosition();
    }

    void GuiElement::UpdatePosition()
    {
        if ( mParent )
            mPositionAbsolute = mParent->GetPositionAbsolute() + mPosition;
        else
            mPositionAbsolute = mPosition;

        GuiElementList::const_iterator itEnd = mChildren.end();
        for ( GuiElementList::const_iterator it = mChildren.begin(); it != itEnd; it++ )
        {
            ( *it )->UpdatePosition();
        }
    }

    void GuiElement::SetPosition( float x, float y )
    {
        SetPosition( glm::vec2( x, y ) );
    }

    bool GuiElement::IsVisible() const
    {
        bool visible = mShow;
        visible &= !( IsHiddenForCapture() && ( FrameWork::GetRenderSystem()->IsScreenshotScheduled() || FrameWork::GetRenderSystem()->IsCaptureInProgress() ) );
        if ( visible && GetParent() != nullptr )
        {
            visible &= GetParent()->IsVisible();
        }
        return visible;
    }

    int GuiElement::GetWidth() const
    {
        return static_cast<int>( ceilf( mSize.x ) );
    }

    int GuiElement::GetHeight() const
    {
        return static_cast<int>( ceilf( mSize.y ) );
    }

    bool GuiElement::IsMouseInside() const
    {
		if ( ImGuiImpl::IsEnabled() )
		{
			return false;
		}
		else
		{
			const glm::vec2& mousePos = FrameWork::GetInputManager()->GetMousePosition();
			const glm::vec2& buttonAbsPos = GetPositionAbsolute();
			return ( mousePos.x > buttonAbsPos.x && mousePos.x < buttonAbsPos.x + mSize.x && mousePos.y > buttonAbsPos.y && mousePos.y < buttonAbsPos.y + mSize.y );
		}
    }

    ///////////////////////////////////////////////////////////////////////////
    // Panel
    ///////////////////////////////////////////////////////////////////////////

    Panel::Panel()
        : mColour( 1.0f, 1.0f, 1.0f, 1.0 )
        , mBorderColour( 1.0f, 1.0f, 1.0f, 1.0f )
        , mBorderMode( PANEL_BORDER_NONE )
        , m_pBackgroundVertexBuffer( nullptr )
        , m_pBorderVertexBuffer( nullptr )
    {
        m_pBackgroundVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION );
        m_pBorderVertexBuffer = new VertexBuffer( GeometryType::Line, VBO_POSITION );
    }

    Panel::~Panel()
    {
        delete m_pBackgroundVertexBuffer;
        delete m_pBorderVertexBuffer;
    }

    void Panel::Render()
    {
        if ( mColour.a > 0.001f || IsHighlighted() )
        {
            const glm::vec2 pos = GetPositionAbsolute();
            m_pBackgroundVertexBuffer->CreateUntexturedQuad( pos.x, pos.y, mSize.x, mSize.y );
            glm::vec4 colour = IsHighlighted() ? glm::vec4( 1.0f, 0.0f, 0.0f, 0.5f ) : glm::vec4( mColour.r, mColour.g, mColour.b, mColour.a );
            GuiManager::GetUntexturedShaderColourUniform()->Set( colour );
            GuiManager::GetUntexturedShader()->Use();
            m_pBackgroundVertexBuffer->Draw();
        }

        DrawBorder();

        GuiElement::Render();
    }

    void Panel::DrawBorder()
    {
        char borderMode = IsHighlighted() ? PANEL_BORDER_ALL : mBorderMode;
        if ( borderMode == PANEL_BORDER_NONE )
        {
            return;
        }

        glm::vec2 pos = GetPositionAbsolute();

        // Allows sharp lines when FSAA is enabled
        pos.x += 0.5f;
        pos.y += 0.5f;

        PositionData posData;
        if ( borderMode & PANEL_BORDER_LEFT )
        {
            posData.push_back( glm::vec3( pos.x + 1.0f, pos.y, 0.0f ) );
            posData.push_back( glm::vec3( pos.x + 1.0f, pos.y + mSize.y - 1.0f, 0.0f ) );
        }
        if ( borderMode & PANEL_BORDER_RIGHT )
        {
            posData.push_back( glm::vec3( pos.x + mSize.x - 1.0f, pos.y, 0.0f ) );
            posData.push_back( glm::vec3( pos.x + mSize.x - 1.0f, pos.y + mSize.y - 1.0f, 0.0f ) );
        }
        if ( borderMode & PANEL_BORDER_TOP )
        {
            posData.push_back( glm::vec3( pos.x + 1.0f, pos.y, 0.0f ) );
            posData.push_back( glm::vec3( pos.x + mSize.x - 1.0f, pos.y, 0.0f ) );
        }
        if ( borderMode & PANEL_BORDER_BOTTOM )
        {
            posData.push_back( glm::vec3( pos.x + 1.0f, pos.y + mSize.y - 1.0f, 0.0f ) );
            posData.push_back( glm::vec3( pos.x + mSize.x - 1.0f, pos.y + mSize.y - 1.0f, 0.0f ) );
        }

        m_pBorderVertexBuffer->CopyPositions( posData );
        glm::vec4 colour = IsHighlighted() ? glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f ) : glm::vec4( mBorderColour.r, mBorderColour.g, mBorderColour.b, mBorderColour.a );
        GuiManager::GetUntexturedShaderColourUniform()->Set( glm::vec4( colour ) );
        GuiManager::GetUntexturedShader()->Use();
        m_pBorderVertexBuffer->Draw();
    }

    ///////////////////////////////////////////////////////////////////////////
    // Cursor
    ///////////////////////////////////////////////////////////////////////////

    Cursor::Cursor()
    {
        SetSize( 16, 16 );
        SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
        SetBorderMode( PANEL_BORDER_NONE );
        SetHiddenForCapture( true );

        // Hide the system's cursor
        SDL_ShowCursor( 0 );
    }

    void Cursor::Update( float delta )
    {
        glm::vec2 position = FrameWork::GetInputManager()->GetMousePosition();
        position.x -= GetSize().x / 2.0f;
        position.y -= GetSize().y / 2.0f;

        SetPosition( position );
        GuiElement::Update( delta );
    }

    void Cursor::SetTexture( ResourceImage* pImage )
    {
        Image::SetTexture( pImage );

        if ( pImage != nullptr )
        {
            SetSize( static_cast<int>( pImage->GetWidth() ), static_cast<int>( pImage->GetHeight() ) );
        }
        else
        {
            SetSize( 16, 16 );
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Image
    ///////////////////////////////////////////////////////////////////////////

    Image::Image()
        : m_pImage( nullptr )
        , m_pImageVertexBuffer( nullptr )
        , m_pOverrideShader( nullptr )
        , m_pOverrideShaderColourUniform( nullptr )
        , m_pOverrideShaderSamplerUniform( nullptr )
    {
        m_pImageVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV );
    }

    Image::~Image()
    {
        delete m_pImageVertexBuffer;
    }

    void Image::Render()
    {
        const glm::vec2 pos = GetPositionAbsolute();
        m_pImageVertexBuffer->CreateTexturedQuad( pos.x, pos.y, mSize.x, mSize.y );

        if ( m_pOverrideShader != nullptr )
        {
            if ( m_pOverrideShaderColourUniform != nullptr )
            {
                m_pOverrideShaderColourUniform->Set( mColour.glm() );
            }

            if ( m_pOverrideShaderSamplerUniform != nullptr && m_pImage != nullptr )
            {
                m_pOverrideShaderSamplerUniform->Set( m_pImage, GL_TEXTURE0 );
            }

            m_pOverrideShader->Use();
        }
        else
        {
            GuiManager::GetTexturedShaderColourUniform()->Set( mColour.glm() );

            if ( m_pImage != nullptr )
            {
                GuiManager::GetTexturedSamplerUniform()->Set( m_pImage, GL_TEXTURE0 );
            }

            GuiManager::GetTexturedShader()->Use();
        }

        m_pImageVertexBuffer->Draw();

        DrawBorder();

        GuiElement::Render();
    }

    void Image::SetShader( Shader* pShader )
    {
        m_pOverrideShader = pShader;

        if ( m_pOverrideShader != nullptr )
        {
            m_pOverrideShaderColourUniform = m_pOverrideShader->RegisterUniform( "k_colour", ShaderUniformType::FloatVector4 );
            m_pOverrideShaderSamplerUniform = m_pOverrideShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
        }
        else
        {
            m_pOverrideShaderColourUniform = nullptr;
            m_pOverrideShaderSamplerUniform = nullptr;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // ButtonImage
    ///////////////////////////////////////////////////////////////////////////

    ButtonImage::ButtonImage()
    {
        m_MousePressedToken = FrameWork::GetInputManager()->AddMouseCallback( std::bind( &ButtonImage::OnMousePressedCallback, this ), MouseButton::Left, ButtonState::Pressed );
        m_MouseReleasedToken = FrameWork::GetInputManager()->AddMouseCallback( std::bind( &ButtonImage::OnMouseReleasedCallback, this ), MouseButton::Left, ButtonState::Released );
    }

    ButtonImage::~ButtonImage()
    {
        FrameWork::GetInputManager()->RemoveMouseCallback( m_MousePressedToken );
        FrameWork::GetInputManager()->RemoveMouseCallback( m_MouseReleasedToken );
    }

    void ButtonImage::OnMousePressedCallback()
    {
        if ( GetAcceptsInput() && IsVisible() && IsMouseInside() )
        {
            OnPress();
        }
    }

    void ButtonImage::OnMouseReleasedCallback()
    {
        if ( GetAcceptsInput() && IsVisible() && IsMouseInside() )
        {
            OnRelease();
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Text
    ///////////////////////////////////////////////////////////////////////////

    Text::Text()
        : m_MultiLine( true )
        , m_pFont( nullptr )
        , m_Text( "" )
        , m_ProcessedText( "" )
        , m_Colour( 1.0f, 1.0f, 1.0f, 1.0f )
        , m_pVertexBuffer( nullptr )
        , m_LineSpacing( 1.0f )
    {
        m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV );
    }

    Text::~Text()
    {
        delete m_pVertexBuffer;
    }

    void Text::OnSizeChanged()
    {
        ProcessText();
    }

    void Text::Render()
    {
        if ( m_pFont == nullptr || m_ProcessedText.empty() )
        {
            return;
        }

        const glm::vec2& pos = GetPositionAbsolute();
        const unsigned int vertexCount = m_pFont->PopulateVertexBuffer( *m_pVertexBuffer, floorf( pos.x ), floorf( pos.y ), m_ProcessedText, m_LineSpacing );
        GuiManager::GetTexturedShaderColourUniform()->Set( glm::vec4( m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a ) );
        GuiManager::GetTexturedSamplerUniform()->Set( m_pFont->GetPage(), GL_TEXTURE0 );
        GuiManager::GetTexturedShader()->Use();
        m_pVertexBuffer->Draw( vertexCount );

        GuiElement::Render();
    }

    // Breaks down the text into multiple lines so that they fit this element's size.
    // The result is cached to avoid unnecessary processing.
    void Text::ProcessText()
    {
        if ( m_pFont == nullptr )
        {
            return;
        }
        else if ( m_MultiLine == false )
        {
            m_ProcessedText = m_Text;
            return;
        }

        std::string line;
        std::string word;
        const float width = GetSize().x;
        size_t lastWordIdx = 0;
        m_ProcessedText.clear();

        for ( size_t i = 0, c = m_Text.length(); i < c; ++i )
        {
            if ( m_Text[ i ] == ' ' || m_Text[ i ] == '\n' || i == c - 1 )
            {
                word = m_Text.substr( lastWordIdx, i - lastWordIdx + 1 );
                lastWordIdx = i + 1;

                if ( m_Text[ i ] == '\n' )
                {
                    line += word;
                    m_ProcessedText += line;
                    line.clear();
                }
                else if ( m_pFont->GetTextLength( line + " " + word ) > width )
                {
                    m_ProcessedText += line + '\n';
                    line = word;
                }
                else
                {
                    line += word;
                }
            }
        }

        if ( line.empty() == false )
        {
            m_ProcessedText += line;
        }
    }

    void Text::SetMultiLine( bool enabled )
    {
        m_MultiLine = enabled;
        if ( m_ProcessedText.empty() == false )
            ProcessText();
    }

    int Text::GetLineCount() const
    {
        if ( m_ProcessedText.empty() )
            return 0;

        int lines = 1;
        size_t l = m_ProcessedText.length();
        for ( size_t i = 0; i < l; ++i )
        {
            if ( m_ProcessedText[ i ] == '\n' )
                lines++;
        }
        return lines;
    }

    void Text::AlignToCentre()
    {
        unsigned int centreX = GetWidth() / 2;
        SetPosition( floorf( centreX - GetFont()->GetTextLength( GetText() ) / 2.0f ), GetPosition().y );
    }

    ///////////////////////////////////////////////////////////////////////////
    // Button
    ///////////////////////////////////////////////////////////////////////////

    Button::Button()
        : mHoverColour( 1.0f, 1.0f, 1.0f, 1.0f )
        , mIconColour( 1.0f, 1.0f, 1.0f, 1.0f )
        , mIconHoverColour( 1.0f, 1.0f, 1.0f, 1.0f )
        , mIsEnabled( true )
        , m_pIcon( nullptr )
        , m_pIconVertexBuffer( nullptr )
    {
        m_MousePressedToken = FrameWork::GetInputManager()->AddMouseCallback( std::bind( &Button::OnMousePressedCallback, this ), MouseButton::Left, ButtonState::Pressed );
        m_MouseReleasedToken = FrameWork::GetInputManager()->AddMouseCallback( std::bind( &Button::OnMouseReleasedCallback, this ), MouseButton::Left, ButtonState::Released );

        SetBorderMode( PANEL_BORDER_ALL );

        mText = new Text();
        mText->SetPosition( 0.0f, 0.0f );
        mText->SetSize( 256.0f, 256.0f );
		mText->SetMultiLine( false );
        AddElement( mText );
    }

    Button::~Button()
    {
        FrameWork::GetInputManager()->RemoveMouseCallback( m_MousePressedToken );
        FrameWork::GetInputManager()->RemoveMouseCallback( m_MouseReleasedToken );

        delete m_pIconVertexBuffer;
    }

    void Button::SetIcon( const std::string& filename )
    {
        m_pIcon = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( filename );

        if ( m_pIconVertexBuffer == nullptr )
        {
            m_pIconVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOUR );
        }
    }

    void Button::Render()
    {
        const bool buttonHovered = IsMouseInside() && GetAcceptsInput();

        // The rectangle itself
        glm::vec4 colour( mColour.r, mColour.g, mColour.b, mColour.a );
        if ( buttonHovered )
        {
            colour = glm::vec4( mHoverColour.r, mHoverColour.g, mHoverColour.b, mHoverColour.a );
        }

        const glm::vec2 pos = GetPositionAbsolute();
        m_pBackgroundVertexBuffer->CreateUntexturedQuad( pos.x, pos.y, mSize.x, mSize.y );
        GuiManager::GetUntexturedShaderColourUniform()->Set( colour );
        GuiManager::GetUntexturedShader()->Use();
        m_pBackgroundVertexBuffer->Draw();

        if ( m_pIcon != nullptr )
        {
            const glm::vec2 iconSize( (float)m_pIcon->GetWidth(), (float)m_pIcon->GetHeight() );
            const glm::vec2 iconPos( pos.x + 2.0f, pos.y );
            const glm::vec4 iconColour = buttonHovered ? mIconHoverColour.glm() : mIconColour.glm();
            m_pIconVertexBuffer->CreateTexturedQuad( iconPos.x, iconPos.y, iconSize.x, iconSize.y, iconColour );
            GuiManager::GetTexturedSamplerUniform()->Set( m_pIcon, GL_TEXTURE0 );
            GuiManager::GetTexturedShaderColourUniform()->Set( iconColour );
            GuiManager::GetTexturedShader()->Use();
            m_pIconVertexBuffer->Draw();
        }

        DrawBorder();

        GuiElement::Render();
    }

    bool Button::IsMouseInside() const
    {
        const glm::vec2& mousePos = FrameWork::GetInputManager()->GetMousePosition();

        if ( mousePos.x < mClipRectangle.min.x || mousePos.x > mClipRectangle.max.x || mousePos.y < mClipRectangle.min.y || mousePos.y > mClipRectangle.max.y )
            return false;
		else if ( ImGuiImpl::IsEnabled() )
			return false;

        const glm::vec2& buttonAbsPos = GetPositionAbsolute();
        return ( mousePos.x > buttonAbsPos.x && mousePos.x < buttonAbsPos.x + mSize.x && mousePos.y > buttonAbsPos.y && mousePos.y < buttonAbsPos.y + mSize.y );
    }

    void Button::SetText( const std::string& text )
    {
        const float textWidth = mText->GetFont()->GetTextLength( text );
        // Make sure the text always "fits" in the button
        mText->SetSize( textWidth * 1.5f, 32.0f );
        // Center the text on the button - flooring coordinates to make sure we render correctly
        const float textPosX = floor( ( GetSize().x - textWidth ) * 0.5f );
        const float textPosY = floor( ( GetSize().y - mText->GetFont()->GetLineHeight() ) * 0.5f + 0.5f );
        mText->SetPosition( textPosX, textPosY );
        mText->SetText( text );
    }

    void Button::OnMousePressedCallback()
    {
        if ( IsEnabled() && GetAcceptsInput() && IsVisible() && IsMouseInside() )
        {
            OnPress();
        }
    }

    void Button::OnMouseReleasedCallback()
    {
        if ( IsEnabled() && GetAcceptsInput() && IsVisible() && IsMouseInside() )
        {
            OnRelease();
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Checkbox
    ///////////////////////////////////////////////////////////////////////////

    static const float CheckboxSquareSize = 16.0f;

    Checkbox::Checkbox( int x, int y, ResourceFont* font, const std::string& text, bool checked /* = false */, CheckboxCallback pCallback /* = nullptr */ )
        : m_Checked( checked )
        , m_BulletColour( 1.0f, 1.0f, 1.0f, 1.0f )
        , m_pCheckboxBulletVertexBuffer( nullptr )
        , m_pCheckboxBorderVertexBuffer( nullptr )
		, m_pCheckboxCallback( pCallback )
    {
        SetPosition( glm::vec2( x, y ) );
        SetSize( 32.0f + font->GetTextLength( text ), std::max( font->GetLineHeight(), CheckboxSquareSize ) );
        SetFont( font );
        SetText( text );
        mText->SetPosition( CheckboxSquareSize + 8.0f, 0.0f );

        m_pCheckboxBulletVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION );
        m_pCheckboxBorderVertexBuffer = new VertexBuffer( GeometryType::LineStrip, VBO_POSITION );
    }

    Checkbox::~Checkbox()
    {
        delete m_pCheckboxBulletVertexBuffer;
        delete m_pCheckboxBorderVertexBuffer;
    }

    void Checkbox::Render()
    {
        const glm::vec2& pos = GetPositionAbsolute();

        m_pBackgroundVertexBuffer->CreateUntexturedQuad( pos.x, pos.y, CheckboxSquareSize, CheckboxSquareSize );
        GuiManager::GetUntexturedShaderColourUniform()->Set( glm::vec4( mColour.r, mColour.g, mColour.b, mColour.a ) );
        GuiManager::GetUntexturedShader()->Use();
        m_pBackgroundVertexBuffer->Draw();

        // Bullet
        if ( m_Checked )
        {
            const float offset = 3.0f;
            m_pCheckboxBulletVertexBuffer->CreateUntexturedQuad( pos.x + offset, pos.y + offset, CheckboxSquareSize - offset * 2.0f, CheckboxSquareSize - offset * 2.0f - 1.0f );
            GuiManager::GetUntexturedShaderColourUniform()->Set( glm::vec4( m_BulletColour.r, m_BulletColour.g, m_BulletColour.b, m_BulletColour.a ) );
            GuiManager::GetUntexturedShader()->Use();
            m_pCheckboxBulletVertexBuffer->Draw();
        }

        // Border around the square
        const float offset = 1.5f;

        PositionData posData;
        posData.push_back( glm::vec3( pos.x + offset, pos.y + offset, 0.0f ) );
        posData.push_back( glm::vec3( pos.x + CheckboxSquareSize - offset, pos.y + offset, 0.0f ) );
        posData.push_back( glm::vec3( pos.x + CheckboxSquareSize - offset, pos.y + CheckboxSquareSize - offset - 1.0f, 0.0f ) );
        posData.push_back( glm::vec3( pos.x + offset, pos.y + CheckboxSquareSize - offset - 1.0f, 0.0f ) );
        posData.push_back( glm::vec3( pos.x + offset, pos.y + offset, 0.0f ) );
        m_pCheckboxBorderVertexBuffer->CopyPositions( posData );

        //m_pCheckboxBorderVertexBuffer->CreateUntexturedQuad( pos.x + offset, pos.y + offset, CheckboxSquareSize - offset * 2.0f, 8.0f );
        GuiManager::GetUntexturedShaderColourUniform()->Set( glm::vec4( mBorderColour.r, mBorderColour.g, mBorderColour.b, mBorderColour.a ) );
        GuiManager::GetUntexturedShader()->Use();
        m_pCheckboxBorderVertexBuffer->Draw();

        GuiElement::Render();
    }

    void Checkbox::OnPress()
    {
        if ( GetAcceptsInput() )
        {
			m_Checked = !m_Checked;

			if (m_pCheckboxCallback != nullptr)
			{
				m_pCheckboxCallback(m_Checked);
			}
        }
    }


	///////////////////////////////////////////////////////////////////////////
	// RadioButton
	///////////////////////////////////////////////////////////////////////////

	static const float RadioButtonSquareSize = 16.0f;
	RadioButton::RadioButtonGroups RadioButton::sRadioButtonGroups;

	RadioButton::RadioButton( int x, int y, ResourceFont* font, const std::string& text, const std::string& group, bool checked /* = false */, RadioButtonCallback pCallback /* = nullptr */ ) : 
	m_Group( group ),
	m_Checked( checked ),
	m_BulletColour( 1.0f, 1.0f, 1.0f, 1.0f ),
	m_pCallback( pCallback )
	{
		SetPosition( glm::vec2( x, y ) );
		SetSize( 32.0f + font->GetTextLength( text ), std::max( font->GetLineHeight(), RadioButtonSquareSize ) );
		SetFont( font );
		SetText( text );
		mText->SetPosition( RadioButtonSquareSize + 8.0f, 0.0f );

		m_pRadioButtonBulletVertexBuffer = std::make_unique< VertexBuffer >( GeometryType::Triangle, VBO_POSITION );
		m_pRadioButtonBorderVertexBuffer = std::make_unique< VertexBuffer >( GeometryType::Line, VBO_POSITION );

		AddToGroup();
	}

	RadioButton::~RadioButton()
	{
		RemoveFromGroup();
	}

	void RadioButton::Render()
	{
		const glm::vec2& pos = GetPositionAbsolute();

		m_pBackgroundVertexBuffer->CreateUntexturedQuad( pos.x, pos.y, RadioButtonSquareSize, RadioButtonSquareSize );
		GuiManager::GetUntexturedShaderColourUniform()->Set( glm::vec4( mColour.r, mColour.g, mColour.b, mColour.a ) );
		GuiManager::GetUntexturedShader()->Use();
		m_pBackgroundVertexBuffer->Draw();

		// Bullet
		float offset = m_Checked ? 4.0f : 6.0f;
		m_pRadioButtonBulletVertexBuffer->CreateUntexturedQuad( pos.x + offset, pos.y + offset, RadioButtonSquareSize - offset * 2.0f, RadioButtonSquareSize - offset * 2.0f - 1.0f );
		glm::vec4 colour = m_Checked ? glm::vec4( m_BulletColour.r, m_BulletColour.g, m_BulletColour.b, m_BulletColour.a ) : glm::vec4( 0.5f, 0.5f, 0.5f, 0.5f );
		GuiManager::GetUntexturedShaderColourUniform()->Set( colour );
		GuiManager::GetUntexturedShader()->Use();
		m_pRadioButtonBulletVertexBuffer->Draw();

		// Border around the square
		offset = 1.5f;

		PositionData posData( 8 );
		posData.emplace_back( pos.x + offset, pos.y + offset, 0.0f );
		posData.emplace_back( pos.x + RadioButtonSquareSize - offset, pos.y + offset, 0.0f );
		posData.emplace_back( pos.x + RadioButtonSquareSize - offset, pos.y + offset, 0.0f );
		posData.emplace_back( pos.x + RadioButtonSquareSize - offset, pos.y + RadioButtonSquareSize - offset - 1.0f, 0.0f );
		posData.emplace_back( pos.x + RadioButtonSquareSize - offset, pos.y + RadioButtonSquareSize - offset - 1.0f, 0.0f );
		posData.emplace_back( pos.x + offset, pos.y + RadioButtonSquareSize - offset - 1.0f, 0.0f );
		posData.emplace_back( pos.x + offset, pos.y + RadioButtonSquareSize - offset - 1.0f, 0.0f );
		posData.emplace_back( pos.x + offset, pos.y + offset, 0.0f );

		m_pRadioButtonBorderVertexBuffer->CopyPositions( posData );

		GuiManager::GetUntexturedShaderColourUniform()->Set( glm::vec4( mBorderColour.r, mBorderColour.g, mBorderColour.b, mBorderColour.a ) );
		GuiManager::GetUntexturedShader()->Use();
		m_pRadioButtonBorderVertexBuffer->Draw();

		GuiElement::Render();
	}

	void RadioButton::OnPress()
	{
		SetChecked( true );

		if ( m_pCallback != nullptr )
		{
			m_pCallback();
		}
	}

	void RadioButton::SetChecked( bool state )
	{
		m_Checked = state;

		if ( m_Checked )
		{
			auto it = sRadioButtonGroups.find( m_Group );
			SDL_assert( it != sRadioButtonGroups.end() );
			RadioButtonVector& radioButtons = it->second;
			for ( auto& pRadioButton : radioButtons )
			{
				if ( pRadioButton != this )
				{
					pRadioButton->SetChecked( false );
				}
			}
		}
	}

	void RadioButton::AddToGroup()
	{
		auto it = sRadioButtonGroups.find( m_Group );
		if ( it == sRadioButtonGroups.end() )
		{
			sRadioButtonGroups.insert( { m_Group, { this } } );
		}
		else
		{
			it->second.push_back( this );
		}
	}

	void RadioButton::RemoveFromGroup()
	{
		auto it = sRadioButtonGroups.find( m_Group );
		if ( it != sRadioButtonGroups.end() )
		{
			RadioButtonVector& radioButtons = it->second;
			for ( auto it2 = radioButtons.begin(); it2 != radioButtons.end(); it2++ )
			{
				RadioButton* pRadioButton = *it2;
				if ( pRadioButton == this )
				{
					radioButtons.erase( it2 );
					return;
				}
			}
		}
		SDL_assert( false ); // Radio button not found in the group?
	}


    ///////////////////////////////////////////////////////////////////////////
    // InputAreaFilter
    ///////////////////////////////////////////////////////////////////////////

    InputAreaFilter::InputAreaFilter( unsigned char filterMask )
        : m_FilterMask( filterMask )
    {
    }

    bool InputAreaFilter::Accepts( char c ) const
    {
        // http://www.asciitable.com/
        if ( m_FilterMask == ACCEPT_ALL )
        {
            return true;
        }
        else if ( ( m_FilterMask & ACCEPT_LETTERS ) && ( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) ) )
        {
            return true;
        }
        else if ( ( m_FilterMask & ACCEPT_NUMBERS ) && c >= '0' && c <= '9' )
        {
            return true;
        }
        else if ( ( m_FilterMask & ACCEPT_SPACES ) && c == ' ' )
        {
            return true;
        }
        else if ( ( m_FilterMask & ACCEPT_PRINTABLE ) && c >= 32 && c <= 126 )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // InputArea
    ///////////////////////////////////////////////////////////////////////////

    InputArea::InputArea()
        : m_PrependText( ">> " )
        , m_SavedText( "" )
        , m_Accum( 0.0f )
        , m_ShowCaret( false )
        , m_Filter( InputAreaFilter::ACCEPT_PRINTABLE )
    {
        mText->SetPosition( 0.0f, 0.0f );
    }

    void InputArea::Update( float delta )
    {
        if ( FrameWork::GetGuiManager()->GetFocusedInputArea() == this )
        {
            m_Accum += delta;
            if ( m_Accum > 0.5f )
            {
                m_Accum = 0.0f;
                m_ShowCaret = !m_ShowCaret;
                UpdateText();
            }
        }
        else
        {
            // This way we force the caret to appear when the box is focused again.
            m_ShowCaret = false;
            m_Accum = 0.75f;
            UpdateText();
        }
    }

    void InputArea::SetText( const std::string& text )
    {
        m_SavedText = text;
        UpdateText();
    }

    void InputArea::OnPress()
    {
        FrameWork::GetGuiManager()->SetFocusedInputArea( this );
    }

    void InputArea::ProcessTextEvent( const SDL_TextInputEvent& ev )
    {
        if ( GetAcceptsInput() == false )
        {
            return;
        }

        // We only accept ASCII characters
        char c = ev.text[ 0 ];

        // Don't add anything else if we don't have enough space or if it is an invalid character
        if ( m_Filter.Accepts( c ) && mText->GetFont()->GetTextLength( m_PrependText + m_SavedText + c ) < mText->GetSize().x )
        {
            m_SavedText += c;
            UpdateText();
        }
    }

    void InputArea::Show( bool state )
    {
        GuiElement::Show( state );

        if ( state == false && FrameWork::GetGuiManager()->GetFocusedInputArea() == this )
        {
            FrameWork::GetGuiManager()->SetFocusedInputArea( nullptr );
        }
    }

    void InputArea::ProcessKeyboardEvent( const SDL_KeyboardEvent& ev )
    {
        SDL_Keysym ks = ev.keysym;

        if ( ks.sym == SDLK_RETURN || ks.sym == SDLK_KP_ENTER )
        {
            // Enter pressed, we finished receiving input and we process it
            FrameWork::GetGuiManager()->SetFocusedInputArea( nullptr );
            m_ShowCaret = false;
            UpdateText();
        }
        else if ( ks.sym == SDLK_ESCAPE )
        {
            // Escape pressed, we finished receiving input but we just clear the input area
            FrameWork::GetGuiManager()->SetFocusedInputArea( nullptr );
            m_ShowCaret = false;
            m_SavedText.clear();
            UpdateText();
        }
        else if ( ks.sym == SDLK_BACKSPACE || ks.sym == SDLK_DELETE )
        {
            if ( !m_SavedText.empty() )
            {
                m_SavedText.erase( m_SavedText.end() - 1 );
                UpdateText();
            }
        }
    }

    void InputArea::UpdateText()
    {
        mText->SetSize( GetSize().x - 8.0f, mText->GetFont()->GetLineHeight() );
        int yPos = (int)( ( GetSize().y - mText->GetFont()->GetLineHeight() ) * 0.5f + 0.5f );
        mText->SetPosition( 8.0f, (float)yPos );
        if ( m_ShowCaret )
            mText->SetText( m_PrependText + m_SavedText + '_' );
        else
            mText->SetText( m_PrependText + m_SavedText );
    }

} // namespace Gui
} // namespace Genesis