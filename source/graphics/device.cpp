// @date 2016/07/30
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/device.hpp>
#include <graphics/opengl.hpp>
#include <core/application.hpp>

NS_FLOW2D_GFX_BEGIN

using namespace flow2d::core;

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

struct DeviceContext
{
    SDL_Window*     window = nullptr;
    SDL_GLContext   context;
};


bool Device::initialize()
{
    ENSURE( _context.has_subsystems<Application>() );

    _device = new (std::nothrow) DeviceContext;
    if( !_device )
    {
        LOGW("failed to create device context.");
        return false;
    }

    return true;
}

void Device::dispose()
{
    if( _device )
    {
        delete _device;
        _device = nullptr;
    }
}

void Device::set_orientations(Orientation orientation)
{
    _orientation = orientation;
    SDL_SetHint(SDL_HINT_ORIENTATIONS, orientation_tostring(orientation));
}

void Device::set_position(const math::Vector2i& position)
{
    _position = position;
    if( _device->window )
        SDL_SetWindowPosition(_device->window, position[0], position[1]);
}

bool Device::set_window(int width, int height, int multisample, WindowOption options)
{
    #ifdef PLATFORM_IOS
    // iOS app always take the fullscree (and with status bar hidden)
    options |= WindowOption::FULLSCREEN;
    // iOS window needs to be resizable to handle orientation changes properly
    options |= WindowOption::RESIZABLE;
    #endif

    // makes no sense to have boarderless in fullscreen, they are mutally exclusive
    if( options & WindowOption::FULLSCREEN )
        options &= ~WindowOption::BORDERLESS;

    // fullscreen or borderless can not be resizable
    if( (options & WindowOption::FULLSCREEN) || (options & WindowOption::BORDERLESS) )
        options &= ~WindowOption::RESIZABLE;

    width = std::max(width, 1);
    height = std::max(height, 1);
    multisample = std::max(std::min(multisample, 16), 1);

    if( _device->window != nullptr &&
        width == _size[0] && height == _size[1] && multisample == _multisamples && options == _options )
        return true;

    // if only vsync changes, there is no need to dispose and reinitialize the window
    if( _device->window != nullptr &&
        width == _size[0] && height == _size[1] && multisample == _multisamples &&
        (options & ~WindowOption::VSYNC) == (_options & ~WindowOption::VSYNC) )
    {
        SDL_GL_SetSwapInterval( options & WindowOption::VSYNC ? 1 : 0 );
        _options = options;
        return true;
    }

    // close the existing window and OpenGL context, mark GPU objects as lost
    release_context();

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, multisample > 1 ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multisample > 1 ? multisample : 0);
    SDL_SetHint(SDL_HINT_ORIENTATIONS, orientation_tostring(_orientation));

    math::Vector2i position = options & WindowOption::FULLSCREEN ? math::Vector2i({0, 0}) : _position;
    unsigned flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

    if( options & WindowOption::FULLSCREEN ) flags |= SDL_WINDOW_FULLSCREEN;
    if( options & WindowOption::BORDERLESS ) flags |= SDL_WINDOW_BORDERLESS;
    if( options & WindowOption::RESIZABLE ) flags |= SDL_WINDOW_RESIZABLE;
    if( options & WindowOption::HIGHDPI ) flags |= SDL_WINDOW_ALLOW_HIGHDPI;

    _device->window = SDL_CreateWindow("FLOW2D", position[0], position[1], width, height, flags);
    // if failed width multisampling, retry first without it
    if( !_device->window && multisample > 1 )
    {
        multisample = 1;
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        _device->window = SDL_CreateWindow("FLOW2D", position[0], position[1], width, height, flags);
    }

    if( !_device->window )
    {
        LOGE("failed to create window, %s.", SDL_GetError());
        return false;
    }

    if( !restore_context() )
        return false;

    // set vsync
    SDL_GL_SetSwapInterval( options & WindowOption::VSYNC ? 1 : 0 );

    _size = { width, height };
    _position = position;
    _multisamples = multisample;
    _options = options;

    SDL_GL_GetDrawableSize(_device->window, &_size[0], &_size[1]);
    if( !(options & WindowOption::FULLSCREEN) )
        SDL_GetWindowPosition(_device->window, &_position[0], &_position[1]);

    // reset rendertargets and viewport for the new mode
    // reset_render_targets();

    // clear the initial window content to black
    clear(ClearOption::COLOR);
    SDL_GL_SwapWindow(_device->window);
    return true;
}

void Device::close_window()
{
    release_context();
    SDL_ShowCursor(SDL_TRUE);

    if( _device->window != nullptr )
    {
        SDL_DestroyWindow(_device->window);
        _device->window = nullptr;
    }
}

bool Device::restore_context()
{
    if( _device->window == nullptr )
    {
        LOGW("failed to restore OpenGL context due to the lack of window instance.");
        return false;
    }

    // the context might be lost behind the scene as the application is minimized in Android
    if( _device->context && !SDL_GL_GetCurrentContext() )
        _device->context = 0;

    if( _device->context == 0 )
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        _device->context = SDL_GL_CreateContext(_device->window);
    }

    if( _device->context == 0 )
    {
        LOGE("failed to create OpenGL context, %s.", SDL_GetError());
        return false;
    }

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    return true;
}

void Device::release_context()
{
    if( _device->window == nullptr )
        return;

    if( _device->context != 0 )
    {
        SDL_GL_DeleteContext(_device->context);
        _device->context = 0;
    }
}

bool Device::begin_frame()
{
    return _device->window != nullptr;
}

void Device::end_frame()
{
    SDL_GL_SwapWindow(_device->window);
}

void Device::clear(ClearOption options, const Color& color, float depth, unsigned stencil)
{
    unsigned flags = 0;
    if( options & ClearOption::COLOR )
    {
        flags |= GL_COLOR_BUFFER_BIT;
        glClearColor(color.r, color.g, color.b, color.a);
    }

    if( options & ClearOption::DEPTH )
    {
        flags |= GL_DEPTH_BUFFER_BIT;
        glClearDepth(depth);
    }

    if( options & ClearOption::STENCIL )
    {
        flags |= GL_STENCIL_BUFFER_BIT;
        glClearStencil(stencil);
    }

    glClear(flags);
}

NS_FLOW2D_GFX_END