// @date 2016/07/30
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/window.hpp>
#include <graphics/backend.hpp>
#include <graphics/opengl.hpp>

NS_LEMON_GRAPHICS_BEGIN

using namespace lemon::core;

static const char* orientation_tostring(Orientation orientation)
{
    switch( orientation )
    {
        case Orientation::LANDSCAPE_LEFT: return "LandscapeLeft";
        case Orientation::LANDSCAPE_RIGHT: return "LandscapeRight";
        case Orientation::PORTRAIT: return "Portrait";
        case Orientation::PORTRAIT_UPSIDE_DOWN: return "PortraitUpsideDown";
        default: FATAL("not supported orientation"); return "";
    }
}

bool WindowDevice::open(int width, int height, int multisample, WindowOption options)
{
    #ifdef PLATFORM_IOS
    // iOS app always take the fullscree (and with status bar hidden)
    options |= WindowOption::FULLSCREEN;
    // iOS window needs to be resizable to handle orientation changes properly
    options |= WindowOption::RESIZABLE;
    #endif

    // makes no sense to have boarderless in fullscreen, they are mutally exclusive
    if( value(options & WindowOption::FULLSCREEN) )
        options &= ~WindowOption::BORDERLESS;

    // fullscreen or borderless can not be resizable
    if( value(options & WindowOption::FULLSCREEN) || value(options & WindowOption::BORDERLESS) )
        options &= ~WindowOption::RESIZABLE;

    width = std::max(width, 1);
    height = std::max(height, 1);
    multisample = std::max(std::min(multisample, 16), 1);

    if( _window != nullptr &&
        width == _size[0] && height == _size[1] && multisample == _multisamples && options == _options )
        return true;

    // if only vsync changes, there is no need to dispose and reinitialize the window
    if( _window != nullptr &&
        width == _size[0] && height == _size[1] && multisample == _multisamples &&
        (options & ~WindowOption::VSYNC) == (_options & ~WindowOption::VSYNC) )
    {
        SDL_GL_SetSwapInterval( value(options & WindowOption::VSYNC) ? 1 : 0 );
        _options = options;
        return true;
    }

    // close the existing window and OpenGL context, mark GPU objects as lost
    auto backend = get_subsystem<Backend>();
    backend->release();

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, multisample > 1 ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multisample > 1 ? multisample : 0);
    SDL_SetHint(SDL_HINT_ORIENTATIONS, orientation_tostring(_orientation));

    math::Vector2i position = value(options & WindowOption::FULLSCREEN) ? math::Vector2i({0, 0}) : _position;
    unsigned flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

    if( value(options & WindowOption::FULLSCREEN) ) flags |= SDL_WINDOW_FULLSCREEN;
    if( value(options & WindowOption::BORDERLESS) ) flags |= SDL_WINDOW_BORDERLESS;
    if( value(options & WindowOption::RESIZABLE) ) flags |= SDL_WINDOW_RESIZABLE;
    if( value(options & WindowOption::HIGHDPI) ) flags |= SDL_WINDOW_ALLOW_HIGHDPI;

    _window = SDL_CreateWindow("LEMON-TOOLKIT", position[0], position[1], width, height, flags);
    // if failed width multisampling, retry first without it
    if( !_window && multisample > 1 )
    {
        multisample = 1;
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        _window = SDL_CreateWindow("LEMON-TOOLKIT", position[0], position[1], width, height, flags);
    }

    if( !_window )
    {
        LOGE("failed to create window, %s.", SDL_GetError());
        return false;
    }

    if( !backend->restore(_window) )
        return false;

    // set vsync
    SDL_GL_SetSwapInterval( value(options & WindowOption::VSYNC) ? 1 : 0 );

    _size = { width, height };
    _position = position;
    _multisamples = multisample;
    _options = options;

    SDL_GL_GetDrawableSize(_window, &_size[0], &_size[1]);
    if( !value(options & WindowOption::FULLSCREEN) )
        SDL_GetWindowPosition(_window, &_position[0], &_position[1]);

    // reset rendertargets and viewport for the new mode
    // reset_render_targets();

    // clear the initial window content to black
    backend->clear(ClearOption::COLOR);
    SDL_GL_SwapWindow(_window);
    return true;
}

void WindowDevice::close()
{
    get_subsystem<Backend>()->release();
    SDL_ShowCursor(SDL_TRUE);

    if( _window != nullptr )
    {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }
}

void WindowDevice::process_message(void* data)
{
    SDL_Event& event = *static_cast<SDL_Event*>(data);
    if( event.type != SDL_WINDOWEVENT )
        return;

    switch( event.window.event )
    {
        case SDL_WINDOWEVENT_MINIMIZED:
            _minimized = true;
            break;

        case SDL_WINDOWEVENT_MAXIMIZED:
        case SDL_WINDOWEVENT_RESTORED:
#if defined(PLATFORM_IOS) || defined (PLATFORM_ANDROID)
            // on iOS we never lose the GL context,
            // but may have done GPU object changes that could not be applied yet. Apply them now
            // on Android the old GL context may be lost already,
            // restore GPU objects to the new GL context
            get_subsystem<Backend>()->restore(_window);
#endif
            _minimized = false;
            break;

        case SDL_WINDOWEVENT_RESIZED:
            SDL_GL_GetDrawableSize(_window, &_size[0], &_size[1]);
            break;

        case SDL_WINDOWEVENT_MOVED:
            SDL_GetWindowPosition(_window, &_position[0], &_position[1]);
            break;

        default:
            break;
    }
}

void WindowDevice::set_orientations(Orientation orientation)
{
    _orientation = orientation;
    SDL_SetHint(SDL_HINT_ORIENTATIONS, orientation_tostring(orientation));
}

void WindowDevice::set_window_position(const math::Vector2i& position)
{
    _position = position;
    if( _window )
    {
        SDL_SetWindowPosition(_window, position[0], position[1]);
        SDL_GetWindowPosition(_window, &_position[0], &_position[1]);
    }
}

void WindowDevice::set_window_size(const math::Vector2i& size)
{
    _size = size;
    if( _window )
    {
        SDL_SetWindowSize(_window, size[0], size[0]);
        SDL_GL_GetDrawableSize(_window, &_size[0], &_size[1]);
    }
}

SDL_Window* WindowDevice::get_window_object() const
{
    return _window;
}

unsigned WindowDevice::get_window_flags() const
{
    return _window ? SDL_GetWindowFlags(_window) : 0;
}

NS_LEMON_GRAPHICS_END
