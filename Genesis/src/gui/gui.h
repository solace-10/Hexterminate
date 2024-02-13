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

#pragma once

#include <functional>

#include "../inputmanager.h"
#include "../rendersystem.h"
#include "../resources/resourcefont.h"
#include "../resources/resourceimage.h"
#include "../taskmanager.h"
#include "../vertexbuffer.h"
#include <list>

namespace Genesis
{

class VertexBuffer;
class ShaderUniform;
class ResourceSound;

namespace Gui
{
    class GuiElement;
    class Cursor;
    class InputArea;

    typedef std::list<GuiElement*> GuiElementList;

    ///////////////////////////////////////////////////////////////////////////
    // Miscellaneous auxiliary functions
    ///////////////////////////////////////////////////////////////////////////

    Genesis::ResourceSound* LoadSFX( const std::string& filename );
    void PlaySFX( ResourceSound* pSFX );

    ///////////////////////////////////////////////////////////////////////////
    // GuiManager
    ///////////////////////////////////////////////////////////////////////////

    class GuiManager : public Task
    {
    public:
        GuiManager();
        ~GuiManager();

        void Initialize();

        TaskStatus Update( float delta );
        TaskStatus Render( float delta );

        void AddElement( GuiElement* element );
        void RemoveElement( GuiElement* element );

        void SetFocusedInputArea( InputArea* inputArea );
        InputArea* GetFocusedInputArea() const;

        static Shader* GetUntexturedShader();
        static ShaderUniform* GetUntexturedShaderColourUniform();
        static Shader* GetTexturedShader();
        static ShaderUniform* GetTexturedShaderColourUniform();
        static ShaderUniform* GetTexturedSamplerUniform();

        Cursor* GetCursor() const;

    private:
        GuiElementList mChildren;
        GuiElementList mToRemove;

        Cursor* m_pCursor;

        // Since we only have one keyboard, we can only be typing
        // on one input area at a time. So all keyboard input
        // will be redirected to this element.
        InputArea* mFocusedInputArea;

        static Shader* m_pUntexturedShader;
        static ShaderUniform* m_pUntexturedColourUniform;
        static Shader* m_pTexturedShader;
        static ShaderUniform* m_pTexturedSamplerUniform;
        static ShaderUniform* m_pTexturedColourUniform;
    };


    ///////////////////////////////////////////////////////////////////////////
    // GuiElement
    ///////////////////////////////////////////////////////////////////////////

    class GuiElement
    {
    public:
        GuiElement();
        virtual ~GuiElement();

        virtual void Render();
        virtual void Update( float delta );

        virtual void AddElement( GuiElement* pElement );
        virtual void RemoveElement( GuiElement* pElement );
        GuiElement* GetParent() const;

        void SetSize( const glm::vec2& size );
        void SetSize( float width, float height );
        void SetSize( int width, int height );
        void SetWidth( float width );
        void SetHeight( float height );
        const glm::vec2& GetSize() const;
        int GetWidth() const;
        int GetHeight() const;
        void SetDepth( int value );
        int GetDepth() const;

        // SetPosition is virtual as some times it needs to overriden to
        // for child classes to update internal elements.
        virtual void SetPosition( const glm::vec2& position );
        virtual void SetPosition( float x, float y );
        virtual void SetPosition( int x, int y );
        const glm::vec2& GetPosition() const;

        const glm::vec2& GetPositionAbsolute() const;

        void UpdateClipRectangle();

        void SetBlendMode( BlendMode blendMode );
        BlendMode GetBlendMode() const;

        virtual void Show( bool state );
        bool IsVisible() const;

        bool GetAcceptsInput() const;
        void SetAcceptsInput( bool state );

        const GuiElementList& GetChildren() const;

        void SetHighlighted( bool state );
        bool IsHighlighted() const;
        void SetHiddenForCapture( bool state );
        bool IsHiddenForCapture() const;

        bool IsMouseInside() const;
        bool IsOrphan() const;
        void OnParentSet();

        static void SortedInsertByDepth( GuiElement* pElement, GuiElementList& elementList );

    protected:
        virtual void OnSizeChanged(){};
        void UpdatePosition();

        GuiElementList mChildren;
        glm::vec2 mPosition;
        glm::vec2 mPositionAbsolute;
        glm::vec2 mSize;
        bool mShow;
        bool mOrphan;

        struct ClipRectangle
        {
            glm::vec2 min;
            glm::vec2 max;
        } mClipRectangle;

    private:
        void RenderHighlight();

        GuiElement* mParent;
        GuiElementList mToRemove;
        BlendMode mBlendMode;

        bool mAcceptsInput;
        bool mHighlighted;
        bool mHiddenForCapture;
        int mDepth;
    };


    ///////////////////////////////////////////////////////////////////////////
    // Panel
    ///////////////////////////////////////////////////////////////////////////

    static const char PANEL_BORDER_LEFT = 1;
    static const char PANEL_BORDER_RIGHT = 1 << 1;
    static const char PANEL_BORDER_TOP = 1 << 2;
    static const char PANEL_BORDER_BOTTOM = 1 << 3;
    static const char PANEL_BORDER_ALL = 0x0F;
    static const char PANEL_BORDER_NONE = 0;

    class Panel : public GuiElement
    {
    public:
        Panel();
        virtual ~Panel();

        virtual void Render();

        const Color& GetColour() const;
        void SetColour( const Color& colour );
        void SetColour( float r, float g, float b, float a );

        const Color& GetBorderColour() const;
        void SetBorderColour( const Color& colour );
        void SetBorderColour( float r, float g, float b, float a );
        void SetBorderMode( char borderMode );

    protected:
        void DrawBorder();

        Color mColour;
        Color mBorderColour;
        char mBorderMode;

        VertexBuffer* m_pBackgroundVertexBuffer;
        VertexBuffer* m_pBorderVertexBuffer;
    };


    ///////////////////////////////////////////////////////////////////////////
    // Image
    ///////////////////////////////////////////////////////////////////////////

    class Image : public Panel
    {
    public:
        Image();
        virtual ~Image();
        virtual void Render();
        virtual void SetTexture( ResourceImage* pImage );
        ResourceImage* GetTexture() const;
        void SetShader( Shader* pShader );
        Shader* GetShader() const;

    private:
        VertexBuffer* m_pImageVertexBuffer;
        ResourceImage* m_pImage;
        Shader* m_pOverrideShader;
        ShaderUniform* m_pOverrideShaderColourUniform;
        ShaderUniform* m_pOverrideShaderSamplerUniform;
    };


    ///////////////////////////////////////////////////////////////////////////
    // Cursor
    ///////////////////////////////////////////////////////////////////////////

    class Cursor : public Image
    {
    public:
        Cursor();
        virtual void Update( float delta );
        virtual void SetTexture( ResourceImage* pImage );
    };


    ///////////////////////////////////////////////////////////////////////////
    // ButtonImage
    ///////////////////////////////////////////////////////////////////////////

    class ButtonImage : public Image
    {
    public:
        ButtonImage();
        virtual ~ButtonImage() override;
        virtual void OnPress(){};
        virtual void OnRelease(){};

    private:
        void OnMousePressedCallback();
        void OnMouseReleasedCallback();

        InputCallbackToken m_MousePressedToken;
        InputCallbackToken m_MouseReleasedToken;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Text
    ///////////////////////////////////////////////////////////////////////////

    class Text : public GuiElement
    {
    public:
        Text();
        virtual ~Text() override;
        virtual void Render() override;
        void SetFont( ResourceFont* pFont );
        ResourceFont* GetFont() const;
        const std::string& GetText() const;
        void SetText( const std::string& text );
        const Color& GetColour() const;
        void SetColour( const Color& colour );
        void SetColour( float r, float g, float b, float a );
        void SetMultiLine( bool enabled );
        int GetLineCount() const;
        void AlignToCentre();
        void SetLineSpacing( float lineSpacing );
        float GetLineSpacing() const;

    private:
        virtual void OnSizeChanged() override;

        void ProcessText();

        bool m_MultiLine;
        ResourceFont* m_pFont;
        std::string m_Text;
        std::string m_ProcessedText;
        Color m_Colour;
        VertexBuffer* m_pVertexBuffer;
        float m_LineSpacing;
    };


    ///////////////////////////////////////////////////////////////////////////
    // Button
    ///////////////////////////////////////////////////////////////////////////

    class Button : public Panel
    {
    public:
        Button();
        virtual ~Button();
        virtual void Render();
        virtual void OnPress(){};
        virtual void OnRelease(){};
        virtual void SetText( const std::string& text );
        void SetTextColour( const Color& colour );
        void SetFont( ResourceFont* font );
        bool IsMouseInside() const;
        const Color& GetHoverColour() const;
        void SetHoverColour( const Color& colour );
        void SetHoverColour( float r, float g, float b, float a );
        void SetIcon( const std::string& filename );
        const Color& GetIconColour() const;
        void SetIconColour( const Color& colour );
        void SetIconColour( float r, float g, float b, float a );
        const Color& GetIconHoverColour() const;
        void SetIconHoverColour( const Color& colour );
        void SetIconHoverColour( float r, float g, float b, float a );

        virtual void Enable( bool state ) { mIsEnabled = state; }
        virtual bool IsEnabled() const { return mIsEnabled; }

    protected:
        Color mHoverColour;
        Color mIconColour;
        Color mIconHoverColour;
        Text* mText;
        bool mIsEnabled;
        ResourceImage* m_pIcon;

    private:
        void OnMousePressedCallback();
        void OnMouseReleasedCallback();

        InputCallbackToken m_MousePressedToken;
        InputCallbackToken m_MouseReleasedToken;

        VertexBuffer* m_pIconVertexBuffer;
    };


    ///////////////////////////////////////////////////////////////////////////
    // Checkbox
    ///////////////////////////////////////////////////////////////////////////

	typedef std::function<void(bool)> CheckboxCallback;

    class Checkbox : public Button
    {
    public:
        Checkbox( int x, int y, ResourceFont* font, const std::string& text, bool checked = false, CheckboxCallback pCallback = nullptr );
        virtual ~Checkbox() override;
        virtual void Render() override;
        virtual void OnPress() override;

        bool IsPressed() const;
        void SetPressed( bool state );

        const Color& GetBulletColour() const;
        void SetBulletColour( const Color& colour );
        void SetBulletColour( float r, float g, float b, float a );

    private:
        Color m_BulletColour;
        bool m_Checked;
        VertexBuffer* m_pCheckboxBulletVertexBuffer;
        VertexBuffer* m_pCheckboxBorderVertexBuffer;
		CheckboxCallback m_pCheckboxCallback;
    };


	///////////////////////////////////////////////////////////////////////////
	// RadioButton
	///////////////////////////////////////////////////////////////////////////

	using RadioButtonCallback = std::function<void(void)>;

	class RadioButton : public Button
	{
	public:
		RadioButton( int x, int y, ResourceFont* font, const std::string& text, const std::string& group, bool checked = false, RadioButtonCallback pCallback = nullptr );
		virtual ~RadioButton() override;
		virtual void Render() override;
		virtual void OnPress() override;

		bool IsChecked() const;
		void SetChecked( bool state );

		const Color& GetBulletColour() const;
		void SetBulletColour( const Color& colour );
		void SetBulletColour( float r, float g, float b, float a );

	private:
		using RadioButtonVector = std::vector< RadioButton* >;
		using RadioButtonGroups = std::unordered_map< std::string, RadioButtonVector >;
		static RadioButtonGroups sRadioButtonGroups;

		void AddToGroup();
		void RemoveFromGroup();

		Color m_BulletColour;
		bool m_Checked;
		VertexBufferUniquePtr m_pRadioButtonBulletVertexBuffer;
		VertexBufferUniquePtr m_pRadioButtonBorderVertexBuffer;
		RadioButtonCallback m_pCallback;
		std::string m_Group;
	};


    ///////////////////////////////////////////////////////////////////////////
    // InputAreaFilter
    ///////////////////////////////////////////////////////////////////////////

    class InputAreaFilter
    {
    public:
        static const unsigned char ACCEPT_LETTERS = 1 << 0;
        static const unsigned char ACCEPT_NUMBERS = 1 << 1;
        static const unsigned char ACCEPT_SPACES = 1 << 2;
        static const unsigned char ACCEPT_PRINTABLE = 1 << 3;
        static const unsigned char ACCEPT_ALL = 0xFF;

        InputAreaFilter( unsigned char filterMask );
        bool Accepts( char c ) const;

    private:
        unsigned char m_FilterMask;
    };


    ///////////////////////////////////////////////////////////////////////////
    // InputArea
    ///////////////////////////////////////////////////////////////////////////

    class InputArea : public Button
    {
    public:
        InputArea();
        virtual void Update( float delta );
        virtual void OnPress();
        virtual void SetText( const std::string& text );
        virtual void Show( bool state );
        const std::string& GetText() const;
        void SetPrependText( const std::string& text );
        void ProcessTextEvent( const SDL_TextInputEvent& ev );
        void ProcessKeyboardEvent( const SDL_KeyboardEvent& ev );
        void SetFilter( const InputAreaFilter& filter );

    private:
        void UpdateText();
        std::string m_PrependText;
        std::string m_SavedText;
        float m_Accum;
        bool m_ShowCaret;
        InputAreaFilter m_Filter;
    };
}
}

#include "gui.inl"