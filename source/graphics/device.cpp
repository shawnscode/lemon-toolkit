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

static const unsigned GL_COMPARE_FUNC[] =
{
    GL_ALWAYS,
    GL_EQUAL,
    GL_NOTEQUAL,
    GL_LESS,
    GL_LEQUAL,
    GL_GREATER,
    GL_GEQUAL
};

static const unsigned GL_BLEND_SOURCE[] =
{
    GL_ONE,
    GL_ONE,
    GL_DST_COLOR,
    GL_SRC_ALPHA,
    GL_SRC_ALPHA,
    GL_ONE,
    GL_ONE_MINUS_DST_ALPHA,
    GL_ONE,
    GL_SRC_ALPHA
};

static const unsigned GL_BLEND_DESTINATION[] =
{
    GL_ZERO,
    GL_ONE,
    GL_ZERO,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_ONE,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE,
    GL_ONE
};

static const unsigned GL_BLEND_OP[] =
{
    GL_FUNC_ADD,
    GL_FUNC_ADD,
    GL_FUNC_ADD,
    GL_FUNC_ADD,
    GL_FUNC_ADD,
    GL_FUNC_ADD,
    GL_FUNC_ADD,
    GL_FUNC_REVERSE_SUBTRACT,
    GL_FUNC_REVERSE_SUBTRACT
};

static const unsigned GL_STENCIL_OP[] =
{
    GL_KEEP,
    GL_ZERO,
    GL_REPLACE,
    GL_INCR_WRAP,
    GL_DECR_WRAP
};

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

    reset_cached_state();
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

    // get default render framebuffer
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_system_frame_object);

    reset_cached_state();
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

void Device::reset_cached_state()
{
    _current_frame_object = _system_frame_object;
    _current_texture = _current_vertex_object = 0;

    _shader = nullptr;
    _index_buffer = nullptr;
    for(size_t i = 0; i < kMaxVertexBuffers; i++ )
        _vertex_buffers[i] = nullptr;

    _blend_mode = BlendMode::REPLACE;
    _cull_mode  = CullMode::NONE;

    _color_write = true;

    _scissor_test = false;
    _scissor = { {0, 0}, {0, 0} };

    // even if the depth buffer exists and the depth mask is non-zero,
    // the depth buffer is not updated if the depth test is disabled.
    // in order to unconditionally write to the depth buffer, the depth
    // test should be enabled and set to GL_ALWAYS.
    _depth_test = true;
    _depth_test_mode = CompareMode::LESS;
    _depth_constant_bias = 0.f;
    _depth_slope_scaled_bias = 0.f;
    _depth_write = true;

    // 
    _stencil_test = false;
    _stencil_test_mode = CompareMode::ALWAYS;
    _stencil_ref = 0;
    _stencil_compare_mask = math::max<unsigned>();
    _stencil_pass_op = _stencil_fail_op = _stencil_zfail_op = StencilOp::KEEP;
    _stencil_write_mask = math::max<unsigned>();
}

void Device::set_viewport(const math::Rect2i& viewport)
{
    // glViewport();
    set_scissor_test(false);
}

void Device::set_blend_mode(BlendMode mode)
{
    if( mode != _blend_mode )
    {
        if( mode == BlendMode::REPLACE )
        {
            glDisable(GL_BLEND);
        }
        else
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_BLEND_SOURCE[to_value(mode)], GL_BLEND_DESTINATION[to_value(mode)]);
            glBlendEquation(GL_BLEND_OP[to_value(mode)]);
        }
        _blend_mode = mode;
    }
}

void Device::set_cull_mode(CullMode mode)
{
    // OpenGL convention use ccw as front face
    if( mode != _cull_mode )
    {
        if( mode == CullMode::NONE )
        {
            glDisable(GL_CULL_FACE);
        }
        else
        {
            glEnable(GL_CULL_FACE);
            glCullFace(mode == CullMode::CCW ? GL_FRONT : GL_BACK);
        }
        _cull_mode = mode;
    }
}

void Device::set_scissor_test(bool enable, const math::Rect2i& scissor)
{
    if( enable != _scissor_test )
    {
        if( !enable ) glDisable(GL_SCISSOR_TEST);
        else glEnable(GL_SCISSOR_TEST);
        _scissor_test = enable;
    }

    if( enable && scissor != _scissor )
    {
        glScissor(scissor.min[0], scissor.min[0], scissor.length<0>(), scissor.length<1>());
        _scissor = scissor;
    }
}

void Device::set_stencil_test(bool enable, CompareMode mode, unsigned ref, unsigned compare_mask)
{
    if( enable != _stencil_test )
    {
        if( enable ) glEnable(GL_STENCIL_TEST);
        else glDisable(GL_STENCIL_TEST);
        _stencil_test = enable;
    }

    if( enable && (mode != _stencil_test_mode || ref != _stencil_ref || compare_mask != _stencil_compare_mask) )
    {
        glStencilFunc(GL_COMPARE_FUNC[to_value(mode)], ref, compare_mask);
        _stencil_test_mode = mode;
        _stencil_ref = ref;
        _stencil_compare_mask = compare_mask;
    }
}

void Device::set_stencil_write(StencilOp pass, StencilOp fail, StencilOp zfail, unsigned write_mask)
{
    if( _stencil_test )
    {
        if( write_mask != _stencil_write_mask )
        {
            glStencilMask(write_mask);
            _stencil_write_mask = write_mask;
        }

        if( pass != _stencil_pass_op || fail != _stencil_fail_op || zfail != _stencil_zfail_op )
        {
            glStencilOp(GL_STENCIL_OP[to_value(fail)], GL_STENCIL_OP[to_value(zfail)], GL_STENCIL_OP[to_value(pass)]);
            _stencil_pass_op = pass;
            _stencil_fail_op = fail;
            _stencil_zfail_op = zfail;
        }
    }
}

void Device::set_depth_test(bool enable, CompareMode mode)
{
    if( enable != _depth_test )
    {
        if( enable ) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);
        _depth_test = enable;
    }

    if( enable && mode != _depth_test_mode )
    {
        glDepthFunc(GL_COMPARE_FUNC[to_value(mode)]);
        _depth_test_mode = mode;
    }
}

void Device::set_depth_write(bool write)
{
    if( _depth_test && write != _depth_write )
    {
        glDepthMask(write ? GL_TRUE : GL_FALSE);
        _depth_write = write;
    }
}

void Device::set_depth_bias(float slope_scaled, float constant)
{
    if( _depth_slope_scaled_bias != slope_scaled || _depth_constant_bias != constant )
    {
        if( slope_scaled != 0.f || constant != 0.f )
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(slope_scaled, constant);
        }
        else glDisable(GL_POLYGON_OFFSET_FILL);

        _depth_slope_scaled_bias = slope_scaled;
        _depth_constant_bias = constant;
    }
}

void Device::set_color_write(bool write)
{
    if( write != _color_write )
    {
        if( write ) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        else glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        _color_write = write;
    }
}

void Device::prepare_draw()
{

}

NS_FLOW2D_GFX_END