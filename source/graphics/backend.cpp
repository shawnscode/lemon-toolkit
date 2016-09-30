// @date 2016/07/30
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/backend.hpp>
#include <graphics/opengl.hpp>

NS_LEMON_GRAPHICS_BEGIN

using namespace lemon::core;

static const unsigned GL_CULL_FACE_FUNC[] =
{
    GL_FRONT,
    GL_BACK,
    GL_FRONT_AND_BACK
};

static const unsigned GL_FRONT_FACE_FUNC[] =
{
    GL_CW,
    GL_CCW
};

static const unsigned GL_PRIMITIVE[] =
{
    GL_POINTS,
    GL_LINES,
    GL_LINE_LOOP,
    GL_LINE_STRIP,
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP,
    GL_TRIANGLE_FAN
};

static const unsigned GL_COMPARE_FUNC[] =
{
    GL_NEVER,
    GL_LESS,
    GL_LEQUAL,
    GL_GREATER,
    GL_GEQUAL,
    GL_EQUAL,
    GL_NOTEQUAL,
    GL_ALWAYS
};

static const unsigned GL_STENCIL_OP[] =
{
    GL_KEEP,
    GL_ZERO,
    GL_REPLACE,
    GL_INCR,
    GL_INCR_WRAP,
    GL_DECR,
    GL_DECR_WRAP,
    GL_INVERT
};

static const unsigned GL_BLEND_FACTOR[] =
{
    GL_ZERO,
    GL_ONE,
    GL_SRC_COLOR,
    GL_ONE_MINUS_SRC_COLOR,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA,
    GL_SRC_ALPHA_SATURATE
};

static const unsigned GL_BLEND_EQUATION_FUNC[] =
{
    GL_FUNC_ADD,
    GL_FUNC_SUBTRACT,
    GL_FUNC_REVERSE_SUBTRACT
};

bool Backend::restore(SDL_Window* window)
{
    if( window == nullptr )
    {
        LOGW("failed to restore OpenGL context due to the lack of window instance.");
        return false;
    }

    _window = window;

    // the context might be lost behind the scene as the application is minimized in Android
    if( _context && !SDL_GL_GetCurrentContext() )
        _context = 0;

    if( _context == 0 )
    {
#ifndef GL_ES_VERSION_2_0
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
#endif
        _context = SDL_GL_CreateContext(_window);
    }

#ifndef GL_ES_VERSION_2_0
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if( GLEW_OK != err )
    {
        LOGW("failed to initialize OpenGL extensions, %s.", glewGetErrorString(err));
        return false;
    }
#endif

    if( _context == 0 )
    {
        LOGE("failed to create OpenGL context, %s.", SDL_GetError());
        return false;
    }

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // get default render framebuffer
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_system_frame_object);
    reset_cached_state();

    // ouput informations
    LOGI("Restore OpenGL context with:");
    LOGI("      VENDOR: %s", glGetString(GL_VENDOR));
    LOGI("    RENDERER: %s", glGetString(GL_RENDERER));
    LOGI("     VERSION: %s", glGetString(GL_VERSION));
    LOGI("GLSL VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    return true;
}

void Backend::release()
{
    if( _window != nullptr && _context != 0 )
        SDL_GL_DeleteContext(_context);

    _context = 0;
    _window = nullptr;
}

bool Backend::begin_frame()
{
    return _window != nullptr;
}

void Backend::end_frame()
{
    SDL_GL_SwapWindow(_window);
}

void Backend::clear(ClearOption options, const math::Color& color, float depth, unsigned stencil)
{
    unsigned flags = 0;
    if( value(options & ClearOption::COLOR) )
    {
        flags |= GL_COLOR_BUFFER_BIT;
        glClearColor(color.r, color.g, color.b, color.a);
    }

    if( value(options & ClearOption::DEPTH) )
    {
        flags |= GL_DEPTH_BUFFER_BIT;
        glClearDepth(depth);
    }

    if( value(options & ClearOption::STENCIL) )
    {
        flags |= GL_STENCIL_BUFFER_BIT;
        glClearStencil(stencil);
    }

    glClear(flags);
}

void Backend::reset_cached_state()
{
    _bound_fbo = _system_frame_object;
    _bound_vbo = _bound_ibo = _bound_program = 0;
    _active_texunit = _bound_textype = _bound_texture = 0;
}

void Backend::set_cull_face(bool enable, CullFace face)
{
    if( enable != _render_state.cull.enable )
    {
        if( enable ) glEnable(GL_CULL_FACE);
        else glDisable(GL_CULL_FACE);
        _render_state.cull.enable = enable;
    }

    if( face != _render_state.cull.face )
    {
        if( enable ) glCullFace(GL_CULL_FACE_FUNC[value(face)]);
        _render_state.cull.face = face;
    }
}

void Backend::set_front_face(FrontFaceOrder winding)
{
    if( _render_state.cull.winding != winding )
    {
        glFrontFace(GL_FRONT_FACE_FUNC[value(winding)]);
        _render_state.cull.winding = winding;
    }
}

void Backend::set_scissor_test(bool enable, const math::Rect2i& scissor)
{
    if( enable != _render_state.scissor.enable )
    {
        if( enable ) glEnable(GL_SCISSOR_TEST);
        else glDisable(GL_SCISSOR_TEST);
        _render_state.scissor.enable = enable;
    }

    if( scissor != _render_state.scissor.area )
    {
        if( enable ) glScissor(scissor.min[0], scissor.min[1], scissor.length<0>(), scissor.length<1>());
        _render_state.scissor.area = scissor;
    }
}

void Backend::set_stencil_test(bool enable, CompareEquation equation, unsigned reference, unsigned mask)
{
    if( enable != _render_state.stencil.enable )
    {
        if( enable ) glEnable(GL_STENCIL_TEST);
        else glDisable(GL_STENCIL_TEST);
        _render_state.stencil.enable = enable;
    }

    if( equation != _render_state.stencil.compare ||
        reference != _render_state.stencil.reference ||
        mask != _render_state.stencil.mask )
    {
        if( enable ) glStencilFunc(GL_COMPARE_FUNC[value(equation)], reference, mask);
        _render_state.stencil.compare = equation;
        _render_state.stencil.reference = reference;
        _render_state.stencil.mask = mask;
    }
}

void Backend::set_stencil_write(StencilWriteEquation sfail, StencilWriteEquation dpfail, StencilWriteEquation dppass, unsigned mask)
{
    if( sfail != _render_state.stencil_write.sfail ||
        dpfail != _render_state.stencil_write.dpfail ||
        dppass != _render_state.stencil_write.dppass )
    {
        glStencilOp(GL_STENCIL_OP[value(sfail)], GL_STENCIL_OP[value(dpfail)], GL_STENCIL_OP[value(dppass)]);
        _render_state.stencil_write.sfail = sfail;
        _render_state.stencil_write.dpfail = dpfail;
        _render_state.stencil_write.dppass = dppass;
    }

    if( mask != _render_state.stencil_write.mask )
    {
        glStencilMask(mask);
        _render_state.stencil_write.mask = mask;
    }
}

void Backend::set_depth_test(bool enable, CompareEquation equation)
{
    if( enable != _render_state.depth.enable )
    {
        if( enable ) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);
        _render_state.depth.enable = enable;
    }

    if( equation != _render_state.depth.compare )
    {
        if( enable ) glDepthFunc(GL_COMPARE_FUNC[value(equation)]);
        _render_state.depth.compare = equation;
    }
}

void Backend::set_depth_write(bool enable, float slope_scaled, float constant)
{
    if( enable != _render_state.depth_write.enable )
    {
        if( enable ) glDepthMask(GL_TRUE);
        else glDepthMask(GL_FALSE);
    }

    if( enable != _render_state.depth_write.enable ||
        slope_scaled != _render_state.depth_write.bias_slope_scaled ||
        constant != _render_state.depth_write.bias_constant )
    {
        if( enable && (slope_scaled != 0.f || constant != 0.f) )
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(slope_scaled, constant);
        }
        else glDisable(GL_POLYGON_OFFSET_FILL);
    }

    _render_state.depth_write.enable = enable;
    _render_state.depth_write.bias_slope_scaled = slope_scaled;
    _render_state.depth_write.bias_constant = constant;
}

void Backend::set_color_blend(bool enable, BlendEquation equation, BlendFactor src, BlendFactor dst)
{
    if( enable != _render_state.blend.enable )
    {
        if( enable ) glEnable(GL_BLEND);
        else glDisable(GL_BLEND);
        _render_state.blend.enable = enable;
    }

    if( equation != _render_state.blend.equation ||
        src != _render_state.blend.source_factor ||
        dst != _render_state.blend.destination_factor )
    {
        glBlendFunc(GL_BLEND_FACTOR[value(src)], GL_BLEND_FACTOR[value(dst)]);
        glBlendEquation(GL_BLEND_EQUATION_FUNC[value(equation)]);
        _render_state.blend.equation = equation;
        _render_state.blend.source_factor = src;
        _render_state.blend.destination_factor = dst;
    }
}

void Backend::set_color_write(ColorMask mask)
{
    if( mask != _render_state.color_write )
    {
        GLboolean r = value(mask & ColorMask::RED) > 0 ? GL_TRUE : GL_FALSE;
        GLboolean g = value(mask & ColorMask::GREEN) > 0 ? GL_TRUE : GL_FALSE;
        GLboolean b = value(mask & ColorMask::BLUE) > 0 ? GL_TRUE : GL_FALSE;
        GLboolean a = value(mask & ColorMask::ALPHA) > 0 ? GL_TRUE : GL_FALSE;
        glColorMask(r, g, b, a);
        _render_state.color_write = mask;
    }
}

void Backend::set_viewport(const math::Rect2i& viewport)
{
    if( _viewport != viewport )
    {
        glViewport(viewport.min[0], viewport.min[1], viewport.length<0>(), viewport.length<1>());
        _viewport = viewport;
    }
}

void Backend::set_shader(unsigned program)
{
    if( program != _bound_program )
    {
        glUseProgram(program);
        _bound_program = program;
    }
}

void Backend::set_index_buffer(unsigned ibo)
{
    if( ibo != _bound_ibo )
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        _bound_ibo = ibo;
    }
}

void Backend::set_vertex_buffer(unsigned vbo)
{
    if( vbo != _bound_vbo )
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        _bound_vbo = vbo;
    }
}

void Backend::set_texture(unsigned unit, unsigned type, unsigned object)
{
    if( _active_texunit != unit )
    {
        glActiveTexture(GL_TEXTURE0+unit);
        _active_texunit = unit;
    }

    if( _bound_textype != type || _bound_texture != object )
    {
        glBindTexture(_bound_textype, _bound_texture);
        _bound_textype = type;
        _bound_texture = object;
    }
}

void Backend::prepare_draw()
{

}

void Backend::draw(PrimitiveType type, unsigned start, unsigned count)
{
    glDrawArrays(GL_PRIMITIVE[value(type)], start, count);
    CHECK_GL_ERROR();
}

bool Backend::is_device_lost() const
{
    return _window == nullptr || _context == 0;
}

static const char* to_string(GLenum error)
{
    switch(error) {
        case GL_INVALID_OPERATION:
            return "INVALID_OPERATION";
        case GL_INVALID_ENUM:
            return "INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "INVALID_VALUE";
        case GL_OUT_OF_MEMORY:
            return "OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "INVALID_FRAMEBUFFER_OPERATION";
    }
    return "UNDEFINED";
}

void check_device_error(const char* file, unsigned line)
{
    GLenum error = glGetError();
    if( error != GL_NO_ERROR && error != GL_INVALID_ENUM )
        FATAL("GL_%s", to_string(error));
}

NS_LEMON_GRAPHICS_END
