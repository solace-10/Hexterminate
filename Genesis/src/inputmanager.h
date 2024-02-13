#pragma once

#include <array>
#include <functional>

#include <glm/vec2.hpp>
#include <SDL.h>

#include "taskmanager.h"

namespace Genesis
{
enum class MouseButton
{
    Left,
    Right,
    Middle
};

enum class ButtonState
{
    Pressed,
    Released,
    Held
};

typedef unsigned int InputCallbackToken;
typedef std::function<void()> InputCallbackKeyboardFn;
typedef std::function<void()> InputCallbackMouseFn;
typedef std::function<void( float x, float y )> InputCallbackMouseWheelFn;

class InputManager : public Task
{
public:
    InputManager();
    virtual ~InputManager() override {}
    TaskStatus Update( float delta );
    bool IsButtonPressed( SDL_Scancode button ) const;
    bool IsMouseButtonPressed( MouseButton button ) const;
    const glm::vec2& GetMousePosition() const;
    const glm::vec2& GetMouseDelta() const;
    void ShowCursor( bool state );

    InputCallbackToken AddKeyboardCallback( InputCallbackKeyboardFn callback, SDL_Scancode button, ButtonState buttonState );
    InputCallbackToken AddMouseCallback( InputCallbackMouseFn callback, MouseButton button, ButtonState buttonState );
    InputCallbackToken AddMouseWheelCallback( InputCallbackMouseWheelFn callback );

    void RemoveKeyboardCallback( InputCallbackToken token );
    void RemoveMouseCallback( InputCallbackToken token );
    void RemoveMouseWheelCallback( InputCallbackToken token );

    void HandleEvent( SDL_Event& ev );

    static InputCallbackToken sInvalidInputCallbackToken;

private:
    struct InputCallbackKeyboardInfo
    {
        InputCallbackToken token;
        InputCallbackKeyboardFn callback;
        SDL_Scancode button;
        ButtonState buttonState;
    };

    struct InputCallbackMouseInfo
    {
        InputCallbackToken token;
        InputCallbackMouseFn callback;
        MouseButton button;
        ButtonState buttonState;
    };

    struct InputCallbackMouseWheelInfo
    {
        InputCallbackToken token;
        InputCallbackMouseWheelFn callback;
    };

    typedef std::list<InputCallbackKeyboardInfo> CallbackKeyboardList;
    typedef std::list<InputCallbackMouseInfo> CallbackMouseList;
    typedef std::list<InputCallbackMouseWheelInfo> CallbackMouseWheelList;

    InputCallbackToken GenerateToken();

    void HandleMouseMotionEvent( const SDL_MouseMotionEvent& event );
    void HandleMouseButtonEvent( const SDL_MouseButtonEvent& event, ButtonState state );
    void HandleMouseWheelEvent( const SDL_MouseWheelEvent& event );
    void HandleKeyboardEvent( const SDL_KeyboardEvent& event, ButtonState state );
    void HandleInputAreaEvent( const SDL_Event& event );

    CallbackKeyboardList m_KeyboardCallbacks;
    CallbackMouseList m_MouseCallbacks;
    CallbackMouseWheelList m_MouseWheelCallbacks;

    std::array<bool, SDL_NUM_SCANCODES> m_ButtonPressed;

    glm::vec2 m_MousePosition;
    glm::vec2 m_MouseDelta;
    static InputCallbackToken m_sToken;
};

//////////////////////////////////////////////////////////////////////////
// Inline implementations
//////////////////////////////////////////////////////////////////////////

inline InputCallbackToken InputManager::GenerateToken()
{
    return m_sToken++;
}

inline const glm::vec2& InputManager::GetMousePosition() const
{
    return m_MousePosition;
}

inline const glm::vec2& InputManager::GetMouseDelta() const
{
    return m_MouseDelta;
}

inline bool InputManager::IsButtonPressed( SDL_Scancode button ) const
{
    return m_ButtonPressed[ button ];
}

inline bool InputManager::IsMouseButtonPressed( MouseButton button ) const
{
    Uint32 mouseState = SDL_GetMouseState( nullptr, nullptr );
    if ( button == MouseButton::Left )
    {
        return ( mouseState & SDL_BUTTON_LMASK ) > 0u;
    }
    else if ( button == MouseButton::Right )
    {
        return ( mouseState & SDL_BUTTON_RMASK ) > 0u;
    }
    else if ( button == MouseButton::Middle )
    {
        return ( mouseState & SDL_BUTTON_MMASK ) > 0u;
    }
    else
    {
        return false;
    }
}
}
