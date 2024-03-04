// Copyright 2018 Pedro Nunes
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

#pragma once

namespace Genesis::Gui
{
    ///////////////////////////////////////////////////////////////////////////
    // GuiManager
    ///////////////////////////////////////////////////////////////////////////

    inline Cursor* GuiManager::GetCursor() const
    {
        return m_pCursor;
    }

    inline void GuiManager::RemoveElement( GuiElement* pElement )
    {
        if ( pElement == m_pHighlighted )
        {
            SetHighlightedElement( nullptr );
        }

        m_ToRemove.push_back( pElement );
    }

    inline InputArea* GuiManager::GetFocusedInputArea() const
    {
        return m_FocusedInputArea;
    }

    inline void GuiManager::SetHighlightedElement( GuiElement* pElement )
    {
        m_pHighlighted = pElement;

        if ( m_pHighlighted )
        {
            RebuildHighlightVertexBuffer();
        }
    }

    inline Shader* GuiManager::GetUntexturedShader()
    {
        return m_pUntexturedShader;
    }

    inline ShaderUniform* GuiManager::GetUntexturedShaderColorUniform()
    {
        return m_pUntexturedColorUniform;
    }

    inline Shader* GuiManager::GetTexturedShader()
    {
        return m_pTexturedShader;
    }

    inline ShaderUniform* GuiManager::GetTexturedShaderColorUniform()
    {
        return m_pTexturedColorUniform;
    }

    inline ShaderUniform* GuiManager::GetTexturedSamplerUniform()
    {
        return m_pTexturedSamplerUniform;
    }

    inline Shader* GuiManager::GetHighlightShader()
    {
        return m_pHighlightShader;
    }

    ///////////////////////////////////////////////////////////////////////////
    // GuiElement
    ///////////////////////////////////////////////////////////////////////////

    inline bool GuiElement::IsOrphan() const
    {
        return mOrphan;
    }

    inline GuiElement* GuiElement::GetParent() const
    {
        return mParent;
    }

    inline void GuiElement::SetSize( const glm::vec2& size )
    {
        mSize = size;
        OnSizeChanged();
    }

    inline void GuiElement::SetSize( float width, float height )
    {
        mSize.x = width;
        mSize.y = height;
        OnSizeChanged();
    }

    inline void GuiElement::SetSize( int width, int height )
    {
        mSize.x = static_cast<float>( width );
        mSize.y = static_cast<float>( height );
        OnSizeChanged();
    }

    inline void GuiElement::SetWidth( float width )
    {
        mSize.x = width;
        OnSizeChanged();
    }

    inline void GuiElement::SetHeight( float height )
    {
        mSize.y = height;
        OnSizeChanged();
    }

    inline int GuiElement::GetDepth() const
    {
        return mDepth;
    }

    inline const glm::vec2& GuiElement::GetSize() const
    {
        return mSize;
    }

    inline const glm::vec2& GuiElement::GetPosition() const
    {
        return mPosition;
    }

    inline const glm::vec2& GuiElement::GetPositionAbsolute() const
    {
        return mPositionAbsolute;
    }

    inline BlendMode GuiElement::GetBlendMode() const
    {
        return mBlendMode;
    }

    inline void GuiElement::SetBlendMode( BlendMode blendMode )
    {
        mBlendMode = blendMode;
    }

    inline bool GuiElement::GetAcceptsInput() const
    {
        return mAcceptsInput;
    }

    inline void GuiElement::SetAcceptsInput( bool state )
    {
        mAcceptsInput = state;
    }

    inline void GuiElement::Show( bool state )
    {
        mShow = state;
    }

    inline const GuiElementList& GuiElement::GetChildren() const
    {
        return mChildren;
    }

    inline void GuiElement::SetHiddenForCapture( bool state )
    {
        mHiddenForCapture = state;
    }

    inline bool GuiElement::IsHiddenForCapture() const
    {
        return mHiddenForCapture;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Panel
    ///////////////////////////////////////////////////////////////////////////

    inline const Color& Panel::GetColor() const
    {
        return m_Color;
    }

    inline void Panel::SetColor( const Color& color )
    {
        m_Color = color;
    }

    inline void Panel::SetColor( float r, float g, float b, float a )
    {
        m_Color.Set( r, g, b, a );
    }

    inline const Color& Panel::GetBorderColor() const
    {
        return m_BorderColor;
    }

    inline void Panel::SetBorderColor( const Color& color )
    {
        m_BorderColor = color;
    }

    inline void Panel::SetBorderColor( float r, float g, float b, float a )
    {
        m_BorderColor.Set( r, g, b, a );
    }

    inline void Panel::SetBorderMode( char borderMode )
    {
        m_BorderMode = borderMode;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Image
    ///////////////////////////////////////////////////////////////////////////

    inline void Image::SetTexture( ResourceImage* pImage )
    {
        m_pImage = pImage;
    }

    inline ResourceImage* Image::GetTexture() const
    {
        return m_pImage;
    }

    inline Shader* Image::GetShader() const
    {
        return m_pOverrideShader;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Text
    ///////////////////////////////////////////////////////////////////////////

    inline void Text::SetFont( ResourceFont* pFont )
    {
        m_pFont = pFont;
        ProcessText();
    }

    inline ResourceFont* Text::GetFont() const
    {
        return m_pFont;
    }

    inline const std::string& Text::GetText() const
    {
        return m_Text;
    }

    inline void Text::SetText( const std::string& text )
    {
        if ( m_Text != text )
        {
            m_Text = text;
            ProcessText();
        }
    }

    inline const Color& Text::GetColor() const
    {
        return m_Color;
    }

    inline void Text::SetColor( const Color& color )
    {
        m_Color = color;
    }

    inline void Text::SetColor( float r, float g, float b, float a )
    {
        m_Color = Color( r, g, b, a );
    }

    inline void Text::SetLineSpacing( float value )
    {
        m_LineSpacing = value;
    }

    inline float Text::GetLineSpacing() const
    {
        return m_LineSpacing;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Button
    ///////////////////////////////////////////////////////////////////////////

    inline const Color& Button::GetHoverColor() const
    {
        return m_HoverColor;
    }

    inline void Button::SetHoverColor( const Color& color )
    {
        m_HoverColor = color;
    }

    inline void Button::SetHoverColor( float r, float g, float b, float a )
    {
        m_HoverColor.Set( r, g, b, a );
    }

    inline const Color& Button::GetIconColor() const
    {
        return m_IconColor;
    }

    inline void Button::SetIconColor( float r, float g, float b, float a )
    {
        m_IconColor.Set( r, g, b, a );
    }

    inline void Button::SetIconColor( const Color& color )
    {
        m_IconColor = color;
    }

    inline const Color& Button::GetIconHoverColor() const
    {
        return m_IconHoverColor;
    }

    inline void Button::SetIconHoverColor( float r, float g, float b, float a )
    {
        m_IconHoverColor.Set( r, g, b, a );
    }

    inline void Button::SetIconHoverColor( const Color& color )
    {
        m_IconHoverColor = color;
    }

    inline void Button::SetFont( ResourceFont* font )
    {
        m_pText->SetFont( font );
    }

    inline void Button::SetTextColor( const Color& color )
    {
        m_pText->SetColor( color );
    }

    ///////////////////////////////////////////////////////////////////////////
    // Checkbox
    ///////////////////////////////////////////////////////////////////////////

    inline bool Checkbox::IsPressed() const
    {
        return m_Checked;
    }

    inline void Checkbox::SetPressed( bool state )
    {
        m_Checked = state;
    }

    inline const Color& Checkbox::GetBulletColor() const
    {
        return m_BulletColor;
    }

    inline void Checkbox::SetBulletColor( const Color& color )
    {
        m_BulletColor = color;
    }

    inline void Checkbox::SetBulletColor( float r, float g, float b, float a )
    {
        m_BulletColor.Set( r, g, b, a );
    }

    ///////////////////////////////////////////////////////////////////////////
    // RadioButton
    ///////////////////////////////////////////////////////////////////////////

    inline bool RadioButton::IsChecked() const
    {
        return m_Checked;
    }

    inline const Color& RadioButton::GetBulletColor() const
    {
        return m_BulletColor;
    }

    inline void RadioButton::SetBulletColor( const Color& color )
    {
        m_BulletColor = color;
    }

    inline void RadioButton::SetBulletColor( float r, float g, float b, float a )
    {
        m_BulletColor.Set( r, g, b, a );
    }

    ///////////////////////////////////////////////////////////////////////////
    // InputArea
    ///////////////////////////////////////////////////////////////////////////

    inline const std::string& InputArea::GetText() const
    {
        return m_SavedText;
    }

    inline void InputArea::SetPrependText( const std::string& text )
    {
        m_PrependText = text;
    }

    inline void InputArea::SetFilter( const InputAreaFilter& filter )
    {
        m_Filter = filter;
    }

} // namespace Genesis::Gui
