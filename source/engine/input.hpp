// @date 2016/08/17
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <engine/engine.hpp>
#include <codebase/enumeration.hpp>
#include <math/vector.hpp>

#include <unordered_set>

NS_LEMON_BEGIN

enum class KeyboardCode
{
    A = 0,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    NUM_0,
    NUM_1,
    NUM_2,
    NUM_3,
    NUM_4,
    NUM_5,
    NUM_6,
    NUM_7,
    NUM_8,
    NUM_9,
    BACKSPACE,
    TAB,
    RETURN,
    RETURN2,
    KP_ENTER,
    SHIFT,
    CTRL,
    ALT,
    GUI,
    PAUSE,
    CAPSLOCK,
    ESCAPE,
    SPACE,
    PAGEUP,
    PAGEDOWN,
    END,
    HOME,
    LEFT,
    UP,
    RIGHT,
    DOWN,
    SELECT,
    PRINTSCREEN,
    INSERT,
    DELETE,
    LGUI,
    RGUI,
    APPLICATION,
    KP_0,
    KP_1,
    KP_2,
    KP_3,
    KP_4,
    KP_5,
    KP_6,
    KP_7,
    KP_8,
    KP_9,
    KP_MULTIPLY,
    KP_PLUS,
    KP_MINUS,
    KP_PERIOD,
    KP_DIVIDE,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    NUMLOCKCLEAR,
    SCROLLLOCK,
    LSHIFT,
    RSHIFT,
    LCTRL,
    RCTRL,
    LALT,
    RALT,
    MAX = 512
};

enum class MouseCode
{
    LEFT = 0,
    MIDDLE,
    RIGHT,
    X1,
    X2
};

enum class KeyboardQualifier
{
    NONE    = 0,
    SHIFT   = 1,
    CTRL    = 2,
    ALT     = 4,
};

// input subsystem. convert operating system window messages to input state
// and internal events.
struct Input : public core::Subsystem
{
    bool initialize() override;
    void dispose() override;

    // begin frame input
    void begin_frame();
    // end frame input and check interal state
    void end_frame();
    // handle window messages, called from engine
    void process_message(void*);
    // set touch emulation by mouse
    void set_touch_emulation(bool);
    // set the visibility of mouse
    void set_mouse_visible(bool);

    // check if a key is held down
    bool get_key_down(KeyboardCode) const;
    bool get_key_down(const char*) const;
    // check if a key has been pressed on this frame
    bool get_key_press(KeyboardCode) const;
    bool get_key_press(const char*) const;
    // check if a qualifier is held down
    bool get_qualifier_down(KeyboardQualifier) const;
    // check if a qualifier has been pressed on this frame
    bool get_qualifier_press(KeyboardQualifier) const;
    // return the currently held down qualifiers
    KeyboardQualifier get_qualifiers() const;

    // return mouse position within window. should only be used with a visible mouse cursor
    math::Vector2i get_mouse_position() const;
    // return mouse movement since last frame
    math::Vector2i get_mouse_delta() const;
    // check if a mouse button is held down
    bool get_mouse_button_down(MouseCode) const;
    // check if a mouse button has been pressed on this frame
    bool get_mouse_button_press(MouseCode) const;

    // return number of active finger touches
    // unsigned get_touches_num() const;
    // return active finger touch by index
    // TouchState get_touch(unsigned) const;

protected:
    void reset_state();

    bool _input_focus = true;
    bool _touch_emulation = false;
    bool _mouse_visible = true;

    math::Vector2i _window_size;
    math::Vector2i _last_mouse_position;
    std::unordered_set<int> _key_down;
    std::unordered_set<int> _key_press;
    std::unordered_set<int> _mouse_down;
    std::unordered_set<int> _mouse_press;
};

NS_LEMON_END
ENABLE_BITMASK_OPERATORS(lemon::KeyboardQualifier);
