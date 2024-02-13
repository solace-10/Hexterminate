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

namespace Genesis
{
namespace Gui
{

///////////////////////////////////////////////////////////////////////////
// GuiManager
///////////////////////////////////////////////////////////////////////////

inline Cursor* GuiManager::GetCursor() const
{
	return m_pCursor;
}

inline void GuiManager::RemoveElement( GuiElement* element )
{
	mToRemove.push_back( element );
}

inline InputArea* GuiManager::GetFocusedInputArea() const
{
	return mFocusedInputArea;
}

inline Shader* GuiManager::GetUntexturedShader()
{
	return m_pUntexturedShader;
}

inline ShaderUniform* GuiManager::GetUntexturedShaderColourUniform()
{
	return m_pUntexturedColourUniform;
}

inline Shader* GuiManager::GetTexturedShader()
{
	return m_pTexturedShader;
}

inline ShaderUniform* GuiManager::GetTexturedShaderColourUniform()
{
	return m_pTexturedColourUniform;
}

inline ShaderUniform* GuiManager::GetTexturedSamplerUniform()
{
	return m_pTexturedSamplerUniform;
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

inline void GuiElement::SetHighlighted( bool state )
{
	mHighlighted = state;
}

inline bool GuiElement::IsHighlighted() const
{
	return mHighlighted;
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

inline const Color& Panel::GetColour() const
{
	return mColour;
}

inline void Panel::SetColour( const Color& colour )
{
	mColour = colour;
}

inline void Panel::SetColour( float r, float g, float b, float a )
{
	mColour.Set( r, g, b, a );
}

inline const Color& Panel::GetBorderColour() const
{
	return mBorderColour;
}

inline void Panel::SetBorderColour( const Color& colour )
{
	mBorderColour = colour;
}

inline void Panel::SetBorderColour( float r, float g, float b, float a )
{
	mBorderColour.Set( r, g, b, a );
}

inline void Panel::SetBorderMode( char borderMode )
{
	mBorderMode = borderMode;
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

inline const Color& Text::GetColour() const
{
	return m_Colour;
}

inline void Text::SetColour( const Color& colour )
{
	m_Colour = colour;
}

inline void Text::SetColour( float r, float g, float b, float a )
{
	m_Colour.Set( r, g, b, a );
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

inline const Color& Button::GetHoverColour() const
{
	return mHoverColour;
}

inline void Button::SetHoverColour( const Color& colour )
{
	mHoverColour = colour;
}

inline void Button::SetHoverColour( float r, float g, float b, float a )
{
	mHoverColour.Set( r, g, b, a );
}

inline const Color& Button::GetIconColour() const
{
	return mIconColour;
}

inline void Button::SetIconColour( float r, float g, float b, float a )
{
	mIconColour.Set( r, g, b, a );
}

inline void Button::SetIconColour( const Color& colour )
{
	mIconColour = colour;
}

inline const Color& Button::GetIconHoverColour() const
{
	return mIconHoverColour;
}

inline void Button::SetIconHoverColour( float r, float g, float b, float a )
{
	mIconHoverColour.Set( r, g, b, a );
}

inline void Button::SetIconHoverColour( const Color& colour )
{
	mIconHoverColour = colour;
}

inline void Button::SetFont( ResourceFont* font )
{
	mText->SetFont( font );
}

inline void Button::SetTextColour( const Color& colour )
{
	mText->SetColour( colour );
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

inline const Color& Checkbox::GetBulletColour() const
{
	return m_BulletColour;
}

inline void Checkbox::SetBulletColour( const Color& colour )
{
	m_BulletColour = colour;
}

inline void Checkbox::SetBulletColour( float r, float g, float b, float a )
{
	m_BulletColour.Set( r, g, b, a );
}


///////////////////////////////////////////////////////////////////////////
// RadioButton
///////////////////////////////////////////////////////////////////////////

inline bool RadioButton::IsChecked() const
{
	return m_Checked;
}

inline const Color& RadioButton::GetBulletColour() const
{
	return m_BulletColour;
}

inline void RadioButton::SetBulletColour( const Color& colour )
{
	m_BulletColour = colour;
}

inline void RadioButton::SetBulletColour( float r, float g, float b, float a )
{
	m_BulletColour.Set( r, g, b, a );
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

}
}