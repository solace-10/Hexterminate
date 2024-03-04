#include <vector>

#include <imgui/imgui.h>
#include <SDL.h>

#include "imgui/imgui_impl.h"
#include "genesis.h"
#include "rendersystem.h"
#include "shadercache.h"
#include "shaderuniform.h"
#include "vertexbuffer.h"
#include "window.h"

namespace Genesis
{

double ImGuiImpl::g_Time = 0.0f;
bool ImGuiImpl::g_MousePressed[3] = { false, false, false };
float ImGuiImpl::g_MouseWheel = 0.0f;
GLuint ImGuiImpl::g_FontTexture = 0;
Shader* ImGuiImpl::m_pDiffuseShader = nullptr;
ResourceImage* ImGuiImpl::m_pTexture = nullptr;
VertexBuffer* ImGuiImpl::m_pVertexBuffer = nullptr;
ShaderUniform* ImGuiImpl::m_pDiffuseSampler = nullptr;
bool ImGuiImpl::m_Initialised = false;
bool ImGuiImpl::m_Enabled = false;
ImGuiImpl::MenuRegistry ImGuiImpl::m_MenuRegistry;

char* g_ClipboardTextData = nullptr;

static const char* ImGui_ImplSDL2_GetClipboardText( void* )
{
    if ( g_ClipboardTextData )
    {
        SDL_free( g_ClipboardTextData );
    }

    g_ClipboardTextData = SDL_GetClipboardText();
    return g_ClipboardTextData;
}

static void ImGui_ImplSDL2_SetClipboardText( void*, const char* pText )
{
    SDL_SetClipboardText( pText );
}

void ImGuiImpl::Initialise()
{
    ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;                     // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SDLK_a;
    io.KeyMap[ImGuiKey_C] = SDLK_c;
    io.KeyMap[ImGuiKey_V] = SDLK_v;
    io.KeyMap[ImGuiKey_X] = SDLK_x;
    io.KeyMap[ImGuiKey_Y] = SDLK_y;
    io.KeyMap[ImGuiKey_Z] = SDLK_z;

    io.SetClipboardTextFn = ImGui_ImplSDL2_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplSDL2_GetClipboardText;
    io.ClipboardUserData = nullptr;

	CreateFontsTexture();

	m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_UV | VBO_COLOR | VB_2D );
}

bool ImGuiImpl::IsInitialised()
{
	return m_Initialised;
}

void ImGuiImpl::CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

    // Upload texture to graphics system
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &g_FontTexture);
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

    // Restore state
    glBindTexture(GL_TEXTURE_2D, last_texture);
}

void ImGuiImpl::Shutdown()
{
    if ( g_ClipboardTextData )
    {
        SDL_free( g_ClipboardTextData );
        g_ClipboardTextData = nullptr;
    }

    delete m_pVertexBuffer;

    ImGui::DestroyContext();
}

void ImGuiImpl::NewFrame( float delta )
{
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
	Window* pWindow = FrameWork::GetWindow();
	int w = pWindow->GetWidth();
	int h = pWindow->GetHeight();
	int display_w = pWindow->GetDrawableWidth();
	int display_h = pWindow->GetDrawableHeight();

    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

    // Setup time step
    Uint32	time = SDL_GetTicks();
    double current_time = time / 1000.0;
    io.DeltaTime = delta;
    g_Time = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from SDL_PollEvent())
	if ( IsEnabled() )
	{
		int mx, my;
		Uint32 mouseMask = SDL_GetMouseState(&mx, &my);
		//if ( pWindow->HasFocus() )
			io.MousePos = ImVec2((float)mx, (float)my);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
		//else
		//    io.MousePos = ImVec2(-1, -1);

		io.MouseDown[0] = g_MousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		io.MouseDown[1] = g_MousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
		io.MouseDown[2] = g_MousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
		g_MousePressed[0] = g_MousePressed[1] = g_MousePressed[2] = false;

		io.MouseWheel = g_MouseWheel;
		g_MouseWheel = 0.0f;
	}
	else
	{
		io.MouseDown[0] = io.MouseDown[1] = io.MouseDown[2] = false;
		io.MouseWheel = 0.0f;
	}

	SDL_ShowCursor(IsEnabled());

    // Start the frame
    ImGui::NewFrame();
	m_Initialised = true;

	// Setup the menus we've registered
	if ( IsInitialised() && IsEnabled() && ImGui::BeginMainMenuBar() )
    {
		for ( auto& pair : m_MenuRegistry )
		{
			if ( ImGui::BeginMenu( pair.first.data() ) )
			{
				for ( auto& menu : pair.second )
				{
					ImGui::MenuItem( menu.name.data(), nullptr, menu.pShow );
				}

				ImGui::EndMenu();
			}
		}

        ImGui::EndMainMenuBar();
    }
}

bool ImGuiImpl::HandleEvent(SDL_Event* pEvent)
{
	if ( IsInitialised() == false || IsEnabled() == false )
	{
		return false;
	}

    ImGuiIO& io = ImGui::GetIO();
    switch (pEvent->type)
    {
    case SDL_MOUSEWHEEL:
        {
            if (pEvent->wheel.y > 0)
                g_MouseWheel = 1;
            if (pEvent->wheel.y < 0)
                g_MouseWheel = -1;
            return true;
        }
    case SDL_MOUSEBUTTONDOWN:
        {
            if (pEvent->button.button == SDL_BUTTON_LEFT) g_MousePressed[0] = true;
            if (pEvent->button.button == SDL_BUTTON_RIGHT) g_MousePressed[1] = true;
            if (pEvent->button.button == SDL_BUTTON_MIDDLE) g_MousePressed[2] = true;
            return true;
        }
    case SDL_TEXTINPUT:
        {
            io.AddInputCharactersUTF8(pEvent->text.text);
            return true;
        }
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        {
			if ( pEvent->key.keysym.scancode == SDL_SCANCODE_F1 && pEvent->type == SDL_KEYDOWN )
			{
				Enable( false );
			}
			else
			{
				int key = pEvent->key.keysym.sym & ~SDLK_SCANCODE_MASK;
				io.KeysDown[key] = (pEvent->type == SDL_KEYDOWN);
				io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
				io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
				io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
				io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
			}
            return true;
        }
    }
    return false;
}

void ImGuiImpl::Render()
{
	if ( IsInitialised() == false || IsEnabled() == false )
	{
		return;
	}

	if ( m_pDiffuseShader == nullptr )
	{
		m_pDiffuseShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "imgui" );
		m_pDiffuseSampler = m_pDiffuseShader->RegisterUniform( "k_sampler0", ShaderUniformType::Texture );
		m_pDiffuseSampler->Set( g_FontTexture, GL_TEXTURE0 );
	}

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;

    ImDrawData* draw_data = ImGui::GetDrawData();
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // We are using the OpenGL fixed pipeline to make the example code simpler to read!
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);

	FrameWork::GetRenderSystem()->ViewOrtho();

	std::vector<float> posData;
	std::vector<float> uvData;
	std::vector<float> colorData;

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
		int idx_buffer_offset = 0;
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
        const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;

		int numVertices = cmd_list->IdxBuffer.size();
		posData.reserve( numVertices * 2 );
		uvData.reserve( numVertices * 2 );
		colorData.reserve( numVertices * 4 );
		for (int i = 0; i < numVertices; i++)
		{
			const int idx = idx_buffer[i];
			posData.push_back( vtx_buffer[idx].pos.x );
			posData.push_back( vtx_buffer[idx].pos.y );
			uvData.push_back( vtx_buffer[idx].uv.x );
			uvData.push_back( vtx_buffer[idx].uv.y );

			const unsigned int color = vtx_buffer[idx].col;
			const float a = static_cast<float>( (color & 0xFF000000) >> 24 ) / 255.0f;
			const float b = static_cast<float>( (color & 0x00FF0000) >> 16 ) / 255.0f;
			const float g = static_cast<float>( (color & 0x0000FF00) >> 8 ) / 255.0f;
            const float r = static_cast<float>( color & 0x000000FF ) / 255.0f;
			
			colorData.push_back( r );
			colorData.push_back( g );
			colorData.push_back( b );
			colorData.push_back( a );
		}

		m_pVertexBuffer->CopyData( posData.data(), posData.size(), VBO_POSITION );
		m_pVertexBuffer->CopyData( uvData.data(), uvData.size(), VBO_UV );
		m_pVertexBuffer->CopyData( colorData.data(), colorData.size(), VBO_COLOR );

		m_pDiffuseShader->Use();

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
				m_pDiffuseSampler->Set( (GLuint)(intptr_t)pcmd->TextureId, GL_TEXTURE0 );
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));

				m_pDiffuseShader->Use();
				m_pVertexBuffer->Draw(idx_buffer_offset, pcmd->ElemCount);
            }
            idx_buffer_offset += pcmd->ElemCount;
        }

		posData.clear();
		uvData.clear();
		colorData.clear();
    }

    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
}

bool ImGuiImpl::IsEnabled()
{
	return m_Enabled;
}

void ImGuiImpl::Enable( bool state )
{
	m_Enabled = state;
}

void ImGuiImpl::RegisterMenu( const std::string& menuRoot, const std::string& menuName, bool* pShow )
{
	Menu menu;
	menu.name = menuName;
	menu.pShow = pShow;
	m_MenuRegistry[ menuRoot ].push_back( menu );
}

void ImGuiImpl::UnregisterMenu( const std::string& menuRoot, const std::string& menuName )
{
	MenuRegistry::iterator it = m_MenuRegistry.find( menuRoot );
	if ( it != m_MenuRegistry.end() )
	{
		it->second.remove_if( [&menuName](Menu menu){ return ( menu.name == menuName); } );
	}
}

}
