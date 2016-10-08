// @date 2016/07/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <core/subsystem.hpp>
#include <codebase/type/enumeration.hpp>
#include <math/vector.hpp>

struct SDL_Window;

NS_LEMON_GRAPHICS_BEGIN

enum class Orientation : uint8_t
{
    LANDSCAPE_LEFT = 0,
    LANDSCAPE_RIGHT,
    PORTRAIT,
    PORTRAIT_UPSIDE_DOWN
};

enum class WindowOption : uint16_t
{
    NONE        = 0x0,
    FULLSCREEN  = 0x1,
    BORDERLESS  = 0x2,
    RESIZABLE   = 0x4,
    HIGHDPI     = 0x8,
    VSYNC       = 0x10,
    TRIPLEBUF   = 0x20,
};

// window subsystem, manages the window device states and messages
struct WindowDevice : public core::Subsystem
{
    SUBSYSTEM("lemon::graphics::WindowDevice")

    // create window handle and main OpenGL context
    bool open(int, int, int multisample = 1, WindowOption options = WindowOption::NONE);
    void close();

    // handle window messages, called from engine
    void process_message(void*);

    // set allowed screen orientations as a space-separated way
    void set_orientations(Orientation);
    Orientation get_orientations() const { return _orientation; }
    // set the initial left-upper position of window
    void set_window_position(const math::Vector2i&);
    math::Vector2i get_window_position() const { return _position; }
    // set the window size if fullscreen not enable
    void set_window_size(const math::Vector2i&);
    math::Vector2i get_window_size() const { return _size; }

    // return whether application window is minimized
    bool is_minimized() const { return _minimized; }
    // return window internal representation
    SDL_Window* get_window_object() const;
    // return window flags
    unsigned get_window_flags() const;

protected:
    SDL_Window* _window = nullptr;
    int _multisamples = 0;
    math::Vector2i _size = {1, 1};
    math::Vector2i _position = {0, 0};
    Orientation _orientation = Orientation::PORTRAIT;
    WindowOption _options = WindowOption::NONE;
    bool _minimized = false;
};

NS_LEMON_GRAPHICS_END

ENABLE_BITMASK_OPERATORS(lemon::graphics::WindowOption);
