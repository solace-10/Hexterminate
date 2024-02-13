#include "inputmanager.h"
#include "genesis.h"
#include "gui/gui.h"
#include "memory.h"

namespace Genesis
{

InputCallbackToken InputManager::m_sToken = 1u;
InputCallbackToken InputManager::sInvalidInputCallbackToken = 0u;

InputManager::InputManager()
{
    for ( auto& key : m_ButtonPressed )
    {
        key = false;
    }
}

TaskStatus InputManager::Update( float delta )
{
    m_MouseDelta.x = m_MouseDelta.y = 0.0f;
    int x, y;
    SDL_GetMouseState( &x, &y );
    m_MousePosition.x = static_cast<float>( x );
    m_MousePosition.y = static_cast<float>( y );

    // Notifies any callbacks which are listening to ButtonState::Held
    for ( auto& callbackInfo : m_KeyboardCallbacks )
    {
        if ( callbackInfo.buttonState == ButtonState::Held && m_ButtonPressed[ callbackInfo.button ] )
        {
            callbackInfo.callback();
        }
    }

    for ( auto& callbackInfo : m_MouseCallbacks )
    {
        if ( callbackInfo.buttonState == ButtonState::Held && IsMouseButtonPressed( callbackInfo.button ) )
        {
            callbackInfo.callback();
        }
    }

    return TaskStatus::Continue;
}

void InputManager::ShowCursor( bool state )
{
    SDL_SetRelativeMouseMode( state ? SDL_TRUE : SDL_FALSE );
}

void InputManager::HandleEvent( SDL_Event& ev )
{
    if ( ev.type == SDL_MOUSEMOTION )
    {
        HandleMouseMotionEvent( *reinterpret_cast<SDL_MouseMotionEvent*>( &ev ) );
    }
    else if ( ev.type == SDL_MOUSEBUTTONDOWN )
    {
        HandleMouseButtonEvent( *reinterpret_cast<SDL_MouseButtonEvent*>( &ev ), ButtonState::Pressed );
    }
    else if ( ev.type == SDL_MOUSEBUTTONUP )
    {
        HandleMouseButtonEvent( *reinterpret_cast<SDL_MouseButtonEvent*>( &ev ), ButtonState::Released );
    }
    else if ( ev.type == SDL_MOUSEWHEEL )
    {
        HandleMouseWheelEvent( ev.wheel );
    }
    else if ( ev.type == SDL_KEYDOWN )
    {
        HandleKeyboardEvent( ev.key, ButtonState::Pressed );
    }
    else if ( ev.type == SDL_KEYUP )
    {
        HandleKeyboardEvent( ev.key, ButtonState::Released );
    }

    if ( FrameWork::GetGuiManager() && FrameWork::GetGuiManager()->GetFocusedInputArea() )
    {
        HandleInputAreaEvent( ev );
    }
}

void InputManager::HandleMouseMotionEvent( const SDL_MouseMotionEvent& event )
{
    m_MouseDelta.x = static_cast<float>( event.xrel );
    m_MouseDelta.y = static_cast<float>( event.yrel );
}

void InputManager::HandleMouseButtonEvent( const SDL_MouseButtonEvent& event, ButtonState state )
{
    for ( auto& callbackInfo : m_MouseCallbacks )
    {
        if ( state != callbackInfo.buttonState )
        {
            continue;
        }
        else if (
            ( event.button == SDL_BUTTON_LEFT && callbackInfo.button == MouseButton::Left ) || ( event.button == SDL_BUTTON_RIGHT && callbackInfo.button == MouseButton::Right ) || ( event.button == SDL_BUTTON_MIDDLE && callbackInfo.button == MouseButton::Middle ) )
        {
            callbackInfo.callback();
        }
    }
}

void InputManager::HandleMouseWheelEvent( const SDL_MouseWheelEvent& event )
{
    for ( auto& callbackInfo : m_MouseWheelCallbacks )
    {
        callbackInfo.callback( static_cast<float>( event.x ), static_cast<float>( event.y ) );
    }
}

void InputManager::HandleKeyboardEvent( const SDL_KeyboardEvent& event, ButtonState state )
{
    for ( auto& callbackInfo : m_KeyboardCallbacks )
    {
        if ( callbackInfo.button == event.keysym.scancode )
        {
            if ( event.state == SDL_PRESSED && 
                 state == ButtonState::Pressed && 
                 callbackInfo.buttonState == ButtonState::Pressed &&
                 m_ButtonPressed[ event.keysym.scancode ] == SDL_RELEASED )
            {
                callbackInfo.callback();
            }
            else if ( event.state == SDL_RELEASED && 
                 state == ButtonState::Released && 
                 callbackInfo.buttonState == ButtonState::Released &&
                 m_ButtonPressed[ event.keysym.scancode ] == SDL_PRESSED )
			{
				callbackInfo.callback();
			}
		}
    }

    m_ButtonPressed[ event.keysym.scancode ] = ( event.state == SDL_PRESSED );
}

void InputManager::HandleInputAreaEvent( const SDL_Event& event )
{
    Gui::InputArea* pInputArea = FrameWork::GetGuiManager()->GetFocusedInputArea();
    if ( event.type == SDL_TEXTINPUT )
    {
        const SDL_TextInputEvent& textInputEvent = *reinterpret_cast<const SDL_TextInputEvent*>( &event );
        pInputArea->ProcessTextEvent( textInputEvent );
    }
    else if ( event.type == SDL_KEYDOWN )
    {
        const SDL_KeyboardEvent& keyboardEvent = *reinterpret_cast<const SDL_KeyboardEvent*>( &event );
        pInputArea->ProcessKeyboardEvent( keyboardEvent );
    }
}

InputCallbackToken InputManager::AddKeyboardCallback( InputCallbackKeyboardFn callback, SDL_Scancode button, ButtonState buttonState )
{
    InputCallbackKeyboardInfo info;
    info.callback = callback;
    info.button = button;
    info.buttonState = buttonState;
    info.token = GenerateToken();
    m_KeyboardCallbacks.push_back( info );
    return info.token;
}

InputCallbackToken InputManager::AddMouseCallback( InputCallbackMouseFn callback, MouseButton button, ButtonState buttonState )
{
    InputCallbackMouseInfo info;
    info.callback = callback;
    info.button = button;
    info.buttonState = buttonState;
    info.token = GenerateToken();
    m_MouseCallbacks.push_back( info );
    return info.token;
}

InputCallbackToken InputManager::AddMouseWheelCallback( InputCallbackMouseWheelFn callback )
{
    InputCallbackMouseWheelInfo info;
    info.callback = callback;
    info.token = GenerateToken();
    m_MouseWheelCallbacks.push_back( info );
    return info.token;
}

void InputManager::RemoveKeyboardCallback( InputCallbackToken token )
{
    m_KeyboardCallbacks.remove_if( [token]( const InputCallbackKeyboardInfo& callbackInfo ) { return token == callbackInfo.token; } );
}

void InputManager::RemoveMouseCallback( InputCallbackToken token )
{
    m_MouseCallbacks.remove_if( [token]( const InputCallbackMouseInfo& callbackInfo ) { return token == callbackInfo.token; } );
}

void InputManager::RemoveMouseWheelCallback( InputCallbackToken token )
{
    m_MouseWheelCallbacks.remove_if( [token]( const InputCallbackMouseWheelInfo& callbackInfo ) { return token == callbackInfo.token; } );
}
}