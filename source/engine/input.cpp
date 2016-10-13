// @date 2016/08/17
// @author Mao Jingkai(oammix@gmail.com)

#include <engine/input.hpp>
#include <graphics/window.hpp>

#include <SDL2/SDL.h>

NS_LEMON_BEGIN

static int SDL_KEYBOARD_CODES[] = 
{
    SDLK_a,
    SDLK_b,
    SDLK_c,
    SDLK_d,
    SDLK_e,
    SDLK_f,
    SDLK_g,
    SDLK_h,
    SDLK_i,
    SDLK_j,
    SDLK_k,
    SDLK_l,
    SDLK_m,
    SDLK_n,
    SDLK_o,
    SDLK_p,
    SDLK_q,
    SDLK_r,
    SDLK_s,
    SDLK_t,
    SDLK_u,
    SDLK_v,
    SDLK_w,
    SDLK_x,
    SDLK_y,
    SDLK_z,
    SDLK_0,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_4,
    SDLK_5,
    SDLK_6,
    SDLK_7,
    SDLK_8,
    SDLK_9,
    SDLK_BACKSPACE,
    SDLK_TAB,
    SDLK_RETURN,
    SDLK_RETURN2,
    SDLK_KP_ENTER,
    SDLK_LSHIFT,
    SDLK_LCTRL,
    SDLK_LALT,
    SDLK_LGUI,
    SDLK_PAUSE,
    SDLK_CAPSLOCK,
    SDLK_ESCAPE,
    SDLK_SPACE,
    SDLK_PAGEUP,
    SDLK_PAGEDOWN,
    SDLK_END,
    SDLK_HOME,
    SDLK_LEFT,
    SDLK_UP,
    SDLK_RIGHT,
    SDLK_DOWN,
    SDLK_SELECT,
    SDLK_PRINTSCREEN,
    SDLK_INSERT,
    SDLK_DELETE,
    SDLK_LGUI,
    SDLK_RGUI,
    SDLK_APPLICATION,
    SDLK_KP_0,
    SDLK_KP_1,
    SDLK_KP_2,
    SDLK_KP_3,
    SDLK_KP_4,
    SDLK_KP_5,
    SDLK_KP_6,
    SDLK_KP_7,
    SDLK_KP_8,
    SDLK_KP_9,
    SDLK_KP_MULTIPLY,
    SDLK_KP_PLUS,
    SDLK_KP_MINUS,
    SDLK_KP_PERIOD,
    SDLK_KP_DIVIDE,
    SDLK_F1,
    SDLK_F2,
    SDLK_F3,
    SDLK_F4,
    SDLK_F5,
    SDLK_F6,
    SDLK_F7,
    SDLK_F8,
    SDLK_F9,
    SDLK_F10,
    SDLK_F11,
    SDLK_F12,
    SDLK_F13,
    SDLK_F14,
    SDLK_F15,
    SDLK_F16,
    SDLK_F17,
    SDLK_F18,
    SDLK_F19,
    SDLK_F20,
    SDLK_F21,
    SDLK_F22,
    SDLK_F23,
    SDLK_F24,
    SDLK_NUMLOCKCLEAR,
    SDLK_SCROLLLOCK,
    SDLK_LSHIFT,
    SDLK_RSHIFT,
    SDLK_LCTRL,
    SDLK_RCTRL,
    SDLK_LALT,
    SDLK_RALT
};

bool Input::initialize()
{
    return true;
}

void Input::dispose()
{
}

void Input::begin_frame()
{
    _key_press.clear();
    _mouse_press.clear();
}

void Input::end_frame()
{
    auto device = core::get_subsystem<graphics::WindowDevice>();

    // check for focus change this frame
    if( device->get_window_flags() & SDL_WINDOW_INPUT_FOCUS )
    {
        if( !_input_focus )
        {
            reset_state();
            if( !_mouse_visible )
                SDL_ShowCursor(SDL_FALSE);
        }
        _input_focus = true;
    }
    else
    {
        if( _input_focus )
        {
            reset_state();
            SDL_ShowCursor(SDL_TRUE);
        }
        _input_focus = false;
    }

    // recenter mouse if no mouse visibility
    if( _input_focus && !_mouse_visible && !_touch_emulation )
    {
        auto size = device->get_window_size();
        SDL_WarpMouseInWindow((SDL_Window*)device->get_window_object(), size[0]/2, size[1]/2);
    }
}

void Input::process_message(void* data)
{
    SDL_Event& event = *static_cast<SDL_Event*>(data);
    switch( event.type )
    {
        case SDL_KEYDOWN:
        {
            if( _key_down.find(event.key.keysym.sym) == _key_down.end() )
            {
                _key_down.insert((int)event.key.keysym.sym);
                _key_press.insert((int)event.key.keysym.sym);
            }
            break;
        }
        case SDL_KEYUP:
        {
            _key_down.erase(event.key.keysym.sym);
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        {
            break;
        }
        case SDL_MOUSEBUTTONUP:
        {
            break;
        }
        default:
            break;
    }
}

void Input::reset_state()
{
    _key_down.clear();
    _key_press.clear();
    _mouse_down.clear();
    _mouse_press.clear();
}

void Input::set_touch_emulation(bool enable)
{
    _touch_emulation = enable;
}

void Input::set_mouse_visible(bool visible)
{
    if( _mouse_visible != visible && _input_focus )
        SDL_ShowCursor(visible ? SDL_TRUE : SDL_FALSE );
    _mouse_visible = visible;
}

bool Input::get_key_down(KeyboardCode code) const
{
    return _key_down.find(SDL_KEYBOARD_CODES[value(code)]) != _key_down.end();
}

bool Input::get_key_down(const char* name) const
{
    return _key_down.find(SDL_GetKeyFromName(name)) != _key_down.end();
}

bool Input::get_key_press(KeyboardCode code) const
{
    return _key_press.find(SDL_KEYBOARD_CODES[value(code)]) != _key_press.end();
}

bool Input::get_key_press(const char* name) const
{
    return _key_press.find(SDL_GetKeyFromName(name)) != _key_press.end();
}

bool Input::get_qualifier_down(KeyboardQualifier qualifier) const
{
    bool result = true;
    if( value(qualifier & KeyboardQualifier::SHIFT) )
        result &= (get_key_down(KeyboardCode::LSHIFT) || get_key_down(KeyboardCode::RSHIFT));
    if( value(qualifier & KeyboardQualifier::CTRL) )
        result &= (get_key_down(KeyboardCode::LCTRL) || get_key_down(KeyboardCode::RCTRL));
    if( value(qualifier & KeyboardQualifier::ALT) )
        result &= (get_key_down(KeyboardCode::LALT) || get_key_down(KeyboardCode::RALT));
    return result;
}

bool Input::get_qualifier_press(KeyboardQualifier qualifier) const
{
    bool result = true;
    if( value(qualifier & KeyboardQualifier::SHIFT) )
        result &= (get_key_press(KeyboardCode::LSHIFT) || get_key_press(KeyboardCode::RSHIFT));
    if( value(qualifier & KeyboardQualifier::CTRL) )
        result &= (get_key_press(KeyboardCode::LCTRL) || get_key_press(KeyboardCode::RCTRL));
    if( value(qualifier & KeyboardQualifier::ALT) )
        result &= (get_key_press(KeyboardCode::LALT) || get_key_press(KeyboardCode::RALT));
    return result;
}

KeyboardQualifier Input::get_qualifiers() const
{
    KeyboardQualifier qualifiers = KeyboardQualifier::NONE;
    if( get_qualifier_down(KeyboardQualifier::SHIFT) )
        qualifiers |= KeyboardQualifier::SHIFT;
    if( get_qualifier_down(KeyboardQualifier::CTRL) )
        qualifiers |= KeyboardQualifier::CTRL;
    if( get_qualifier_down(KeyboardQualifier::ALT) )
        qualifiers |= KeyboardQualifier::ALT;
    return qualifiers;
}

math::Vector2i Input::get_mouse_position() const
{
    math::Vector2i result;
    SDL_GetMouseState(&result[0], &result[1]);
    return result;
}

math::Vector2i Input::get_mouse_delta() const
{
    if( _mouse_visible )
        return get_mouse_position() - _last_mouse_position;
    return {0, 0};
}

bool Input::get_mouse_button_down(MouseCode code) const
{
    return _mouse_down.find(SDL_KEYBOARD_CODES[value(code)]) != _mouse_down.end();
}

bool Input::get_mouse_button_press(MouseCode code) const
{
    return _mouse_press.find(SDL_KEYBOARD_CODES[value(code)]) != _mouse_press.end();
}

NS_LEMON_END