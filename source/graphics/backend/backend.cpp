// @date 2016/07/30
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/backend/backend.hpp>
#include <SDL2/SDL.h>

#include <iostream>
#include <thread>
NS_LEMON_GRAPHICS_BEGIN

void VertexBufferGL::create(const void* data, size_t size, const VertexLayout& layout, GLenum usage)
{
    ASSERT(_uid == 0, "duplicated create of vertex buffer");
    ASSERT(layout.get_stride() > 0, "failed to create vertex buffer with empty vertex layout.");

    glGenBuffers(1, &_uid);
    ASSERT(_uid != 0, "failed to create vertex buffer object.");

    _num = size / layout.get_stride();
    _usage = usage;
    _layout = layout;

    glBindBuffer(GL_ARRAY_BUFFER, _uid);
    glBufferData(GL_ARRAY_BUFFER, size, data, _usage);
    CHECK_GL_ERROR();
}

void VertexBufferGL::update(GLuint start, const void* data, size_t size)
{
    ASSERT(_uid != 0, "try to update invalid vertex buffer.");
    ASSERT(_usage == GL_DYNAMIC_DRAW, "try to update dynamic vertex buffer.");
    ASSERT(data != nullptr && size != 0, "try to update vertex buffer with nullptr.");
    ASSERT(start + (size / _layout.get_stride()) < _num, "update vertex buffer out-of-range.");

    glBindBuffer(GL_ARRAY_BUFFER, _uid);
    glBufferSubData(GL_ARRAY_BUFFER, start*_layout.get_stride(), size, data);
    CHECK_GL_ERROR()
}

void VertexBufferGL::free()
{
    ASSERT(_uid != 0, "try to free invalid vertex buffer");
    glDeleteBuffers(1, &_uid);
    _uid = 0;
    CHECK_GL_ERROR()
}

void IndexBufferGL::create(const void* data, size_t size, GLuint element_size, GLenum usage)
{
    ASSERT(_uid == 0, "duplicated create of index buffer");
    ASSERT(element_size > 0, "failed to create vertex buffer with zero-size.");

    glGenBuffers(1, &_uid);

    ASSERT(_uid != 0, "failed to create index buffer object.");

    _num = size / element_size;
    _usage = usage;
    _element_size = element_size;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _uid);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, _usage);
    CHECK_GL_ERROR();
}

void IndexBufferGL::update(GLuint start, const void* data, size_t size)
{
    ASSERT(_uid != 0, "try to update invalid index buffer.");
    ASSERT(_usage == GL_DYNAMIC_DRAW, "try to update dynamic index buffer.");
    ASSERT(data != nullptr && size != 0, "try to update index buffer with nullptr.");
    ASSERT(start + (size / _element_size) < _num, "update index buffer out-of-range.");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _uid);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, start*_element_size, size, data);
    CHECK_GL_ERROR()
}

void IndexBufferGL::free()
{
    ASSERT(_uid != 0, "try to free invalid index buffer");
    glDeleteBuffers(1, &_uid);
    _uid = 0;
    CHECK_GL_ERROR()
}

GLuint compile(GLenum type, const char* source)
{
    GLint status;
    
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        char buf[1024];
        GLint len;
        glGetShaderInfoLog(shader, 1024, &len, buf);

        LOGW("failed to compile, %s. source:\n %s\n", buf, source);
        glDeleteShader(shader);
        return 0;
    }

    CHECK_GL_ERROR();
    return shader;
}

void ProgramGL::create(const char* vsraw, const char* fsraw)
{
    ASSERT(_uid == 0, "duplicated create of program.");

    _uid = glCreateProgram();
    ASSERT(_uid != 0, "failed to create program object.");

    auto vs = compile(GL_VERTEX_SHADER, vsraw);
    ENSURE(vs != 0);

    auto fs = compile(GL_FRAGMENT_SHADER, fsraw);
    ENSURE(fs != 0);

    glAttachShader(_uid, vs);
    glAttachShader(_uid, fs);
    glLinkProgram(_uid);

    glDetachShader(_uid, fs);
    glDetachShader(_uid, vs);
    glDeleteShader(fs);
    glDeleteShader(vs);

    GLint status;
    glGetProgramiv(_uid, GL_LINK_STATUS, &status);
    if( status == 0 )
    {
        char buf[1024];
        GLint len;
        glGetProgramInfoLog(_uid, 1024, &len, buf);
        FATAL("failed to link program: %s\n", buf);
    }

    _uniform_size = 0;
    _texture_size = 0;

    for( uint8_t i = 0; i < VertexAttribute::kVertexAttributeCount; i++ )
    {
        _attributes[i].first.clear();
        _attributes[i].second = -1;
        // try to bind attribute with default name first.
        auto va = (VertexAttribute::Enum)i;
        bind_attribute(va, VertexAttribute::name(va));
    }

    CHECK_GL_ERROR();
}

void ProgramGL::free()
{
    ASSERT(_uid != 0, "try to free invalid program.");
    glDeleteProgram(_uid);
    _uid = 0;
    CHECK_GL_ERROR();
}

GLint ProgramGL::bind_uniform(const char* name)
{
    ASSERT(_uid != 0, "failed to get uniform location without a decent program.");

    auto hash = math::StringHash(name);
    for( uint8_t i = 0; i < _uniform_size; i++ )
    {
        if( _uniforms[i].first == hash )
            return _uniforms[i].second;
    }

    ASSERT( _uniform_size < kMaxUniformsPerMaterial,
        "too many uniforms(%d).", kMaxUniformsPerMaterial);

    auto location = glGetUniformLocation(_uid, name);
    _uniforms[_uniform_size].first = hash;
    _uniforms[_uniform_size++].second = location;
    return location;
}

GLint ProgramGL::bind_attribute(VertexAttribute::Enum va, const char* name)
{
    ASSERT(_uid != 0, "failed to get attribute location without a decent program.");

    name = name == nullptr ? VertexAttribute::name(va) : name;
    auto hash = math::StringHash(name);
    if( _attributes[va].first != hash  )
    {
        _attributes[va].first = hash;
        _attributes[va].second = glGetAttribLocation(_uid, name);
    }

    return _attributes[va].second;
}

void ProgramGL::update_uniform(math::StringHash hash, const UniformVariable& value)
{
    for( uint8_t i = 0; i < _uniform_size; i++ )
    {
        if( _uniforms[i].first == hash )
        {
            auto& uniform = _uniforms[i];
            if( value.is<math::Vector<1, float>>() )
            {
                auto v = value.get<math::Vector<1, float>>();
                glUniform1f(uniform.second, v[0]);
            }
            else if( value.is<math::Vector<2, float>>() )
            {
                auto v = value.get<math::Vector<2, float>>();
                glUniform2f(uniform.second, v[0], v[1]);
            }
            else if( value.is<math::Vector<3, float>>() )
            {
                auto v = value.get<math::Vector<3, float>>();
                glUniform3f(uniform.second, v[0], v[1], v[2]);
            }
            else if( value.is<math::Vector<4, float>>() )
            {
                auto v = value.get<math::Vector<4, float>>();
                glUniform4f(uniform.second, v[0], v[1], v[2], v[3]);
            }
            else if( value.is<math::Matrix<2, 2, float>>() )
            {
                auto v = value.get<math::Matrix<2, 2, float>>();
                glUniformMatrix2fv(uniform.second, 1, GL_TRUE, (float*)&v);
            }
            else if( value.is<math::Matrix<3, 3, float>>() )
            {
                auto v = value.get<math::Matrix<3, 3, float>>();
                glUniformMatrix3fv(uniform.second, 1, GL_TRUE, (float*)&v);
            }
            else if( value.is<math::Matrix<4, 4, float>>() )
            {
                auto v = value.get<math::Matrix<4, 4, float>>();
                glUniformMatrix4fv(uniform.second, 1, GL_TRUE, (float*)&v);
            }
            else if( value.is<Handle>() )
            {
                for( uint8_t i = 0; i < _texture_size; i++ )
                {
                    if( _textures[i].first == hash )
                    {
                        _textures[i].second = value.get<Handle>();
                        return;
                    }
                }

                ASSERT(_texture_size < kMaxTexturePerMaterial,
                    "too many textures(%d).", kMaxTexturePerMaterial);

                glUniform1i(uniform.second, _texture_size);
                _textures[_texture_size].first = hash;
                _textures[_texture_size++].second = value.get<Handle>();
            }
            else
                FATAL("invalid uniform variable variant type %d.", value.which());

            CHECK_GL_ERROR();
            return;
        }
    }
}

void TextureGL::create(
    const void* data,
    GLenum format, GLenum pixel_format,
    uint16_t width, uint16_t height,
    GLenum usage)
{
    ASSERT(_uid == 0, "duplicated creation of texture.");
    ASSERT(width > 0 && height > 0 && data != nullptr,
        "failed to create texture with empty data.");

    glGenTextures(1, &_uid);
    ASSERT(_uid != 0, "failed to create texture.");

    _format = format;
    _pixel_format = pixel_format;
    _width = width;
    _height = height;
    _usage = usage;
    _filter = GL_LINEAR;
    _address[0] = _address[1] = _address[2] = GL_REPEAT;
    _mipmap = false;
    _dirty = true;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _uid);

    update_parameters();

    glTexImage2D(
        /*target*/ GL_TEXTURE_2D,
        /*level*/ 0,
        /*internalFormat*/ _format,
        /*width*/ _width,
        /*height*/ _height,
        /*border*/ 0,
        /*format*/ _format,
        /*type*/ _pixel_format,
        /*data*/ data);

    CHECK_GL_ERROR();
}

void TextureGL::update_mipmap(bool mipmap)
{
    ASSERT(_uid != 0, "failed to generate mipmap with invalid texture.");

    if( _mipmap != mipmap && mipmap )
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _uid);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    _mipmap = mipmap;
}

void TextureGL::update_address_mode(int8_t index, GLenum wrap)
{
    if( _address[index] != wrap )
    {
        _address[index] = wrap;
        _dirty = true;
    }
}

void TextureGL::update_filter_mode(GLenum filter)
{
    if( _filter != filter )
    {
        _filter = filter;
        _dirty = true;
    }
}

void TextureGL::update_parameters()
{
    if( !_dirty || _uid == 0 )
        return;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _uid);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _address[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _address[1]);
#ifndef GL_ES_VERSION_2_0
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, _address[2]);
#endif

    switch(_filter)
    {
        case 0:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;

        case 1:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _mipmap ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;

        case 2:
        case 3:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        default:
        break;
    }

    _dirty = false;
}

void TextureGL::free()
{
    ASSERT(_uid != 0, "try to free invalid texture.");

    glDeleteTextures(1, &_uid);
    _uid = 0;
}

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

bool RenderBackend::initialize(SDL_Window* window)
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

    // ouput informations
    LOGI("Restore OpenGL context with:");
    LOGI("      VENDOR: %s", glGetString(GL_VENDOR));
    LOGI("    RENDERER: %s", glGetString(GL_RENDERER));
    LOGI("     VERSION: %s", glGetString(GL_VERSION));
    LOGI("GLSL VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    return true;
}

void RenderBackend::dispose()
{
    if( _window != nullptr && _context != 0 )
        SDL_GL_DeleteContext(_context);

    _context = 0;
    _window = nullptr;
}

bool RenderBackend::begin_frame()
{
    _active_material.invalidate();
    _active_vbo.invalidate();
    _active_ibo.invalidate();
    _active_vao.first.invalidate();
    _active_vao.second.invalidate();
    _active_texunit = _bound_textype = _bound_texture = 0;

    SDL_GL_MakeCurrent(_window, _context);
    return !is_device_lost();
}

void RenderBackend::end_frame()
{
    glFinish();
    SDL_GL_SwapWindow(_window);
    SDL_GL_MakeCurrent(_window, nullptr);
}

void RenderBackend::create_vertex_buffer(
    Handle handle, const void* data, size_t size, const VertexLayout& layout, BufferUsage usage)
{
    _vbs[handle.get_index()].create(data, size, layout,
        usage == BufferUsage::DYNAMIC ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

void RenderBackend::update_vertex_buffer(
    Handle handle, uint32_t start, const void* data, size_t size)
{
    _vbs[handle.get_index()].update(start, data, size);
}

void RenderBackend::free_vertex_buffer(Handle handle)
{
    _vbs[handle.get_index()].free();
}

GLuint INDEX_ELEMENT_SIZE[] =
{
    1,
    2
};

void RenderBackend::create_index_buffer(
    Handle handle, const void* data, size_t size, IndexElementFormat format, BufferUsage usage)
{
    _ibs[handle.get_index()].create(data, size, INDEX_ELEMENT_SIZE[value(format)],
        usage == BufferUsage::DYNAMIC ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

void RenderBackend::update_index_buffer(
    Handle handle, uint32_t start, const void* data, size_t size)
{
    _ibs[handle.get_index()].update(start, data, size);
}

void RenderBackend::free_index_buffer(Handle handle)
{
    _ibs[handle.get_index()].free();
}

static GLenum GL_TEXTURE_FORMAT[] =
{
    GL_ALPHA,
    GL_RGB,
    GL_RGBA,
    GL_LUMINANCE,
    GL_LUMINANCE_ALPHA
};

static GLenum GL_TEXTURE_PIXEL_FORMAT[] =
{
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_SHORT_5_6_5,
    GL_UNSIGNED_SHORT_4_4_4_4,
    GL_UNSIGNED_SHORT_5_5_5_1
};

void RenderBackend::create_texture(
    Handle handle, const void* data,
    TextureFormat format, TexturePixelFormat pixel_format,
    uint16_t width, uint16_t height,
    BufferUsage usage)
{
    _textures[handle.get_index()].create(data,
        GL_TEXTURE_FORMAT[value(format)],
        GL_TEXTURE_PIXEL_FORMAT[value(pixel_format)],
        width,
        height,
        usage == BufferUsage::DYNAMIC ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

void RenderBackend::update_texture_mipmap(Handle handle, bool mipmap)
{
    _textures[handle.get_index()].update_mipmap(mipmap);
}

static GLenum GL_WRAP_MODE[] =
{
    GL_REPEAT,
    GL_MIRRORED_REPEAT,
    GL_CLAMP_TO_EDGE,
#ifndef GL_ES_VERSION_2_0
    GL_CLAMP,
#else
    GL_CLAMP_TO_EDGE
#endif
};

void RenderBackend::update_texture_address_mode(
    Handle handle, TextureCoordinate coord, TextureAddressMode wrap)
{
    _textures[handle.get_index()].update_address_mode(
        value(coord), GL_WRAP_MODE[value(wrap)]);
}

void RenderBackend::update_texture_filter_mode(
    Handle handle, TextureFilterMode filter)
{
    _textures[handle.get_index()].update_filter_mode(value(filter));
}

void RenderBackend::free_texture(Handle handle)
{
    _textures[handle.get_index()].free();
}

void RenderBackend::create_program(Handle handle, const char* vs, const char* fs)
{
    _materials[handle.get_index()].create(vs, fs);
}

void RenderBackend::free_program(Handle handle)
{
    _materials[handle.get_index()].free();
}

void RenderBackend::create_program_uniform(Handle handle, const char* name)
{
    _materials[handle.get_index()].bind_uniform(name);
}

void RenderBackend::update_program_uniform(Handle handle, math::StringHash name, const UniformVariable& value)
{
    set_program(handle);
    _materials[handle.get_index()].update_uniform(name, value);
}

void RenderBackend::create_program_attribute(Handle handle, VertexAttribute::Enum va, const char* name)
{
    _materials[handle.get_index()].bind_attribute(va, name);
}

void RenderBackend::clear(ClearOption options, const math::Color& color, float depth, unsigned stencil)
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

void RenderBackend::set_cull_face(bool enable, CullFace face)
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

void RenderBackend::set_front_face(FrontFaceOrder winding)
{
    if( _render_state.cull.winding != winding )
    {
        glFrontFace(GL_FRONT_FACE_FUNC[value(winding)]);
        _render_state.cull.winding = winding;
    }
}

void RenderBackend::set_scissor_test(bool enable, const math::Rect2i& scissor)
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

void RenderBackend::set_stencil_test(bool enable, CompareEquation equation, unsigned reference, unsigned mask)
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

void RenderBackend::set_stencil_write(StencilWriteEquation sfail, StencilWriteEquation dpfail, StencilWriteEquation dppass, unsigned mask)
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

void RenderBackend::set_depth_test(bool enable, CompareEquation equation)
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

void RenderBackend::set_depth_write(bool enable, float slope_scaled, float constant)
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

void RenderBackend::set_color_blend(bool enable, BlendEquation equation, BlendFactor src, BlendFactor dst)
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

void RenderBackend::set_color_write(ColorMask mask)
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

void RenderBackend::set_viewport(const math::Rect2i& viewport)
{
    if( _viewport != viewport )
    {
        glViewport(viewport.min[0], viewport.min[1], viewport.length<0>(), viewport.length<1>());
        _viewport = viewport;
    }
}

void RenderBackend::set_program(Handle handle)
{
    if( _active_material != handle )
    {
        glUseProgram(_materials[handle.get_index()]._uid);
        CHECK_GL_ERROR();
        _active_material = handle;
    }
}

void RenderBackend::set_index_buffer(Handle handle)
{
    if( _active_ibo != handle )
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibs[handle.get_index()]._uid);
        CHECK_GL_ERROR();
        _active_ibo = handle;
    }
}

void RenderBackend::set_vertex_buffer(Handle handle)
{
    if( _active_vbo != handle )
    {
        glBindBuffer(GL_ARRAY_BUFFER, _vbs[handle.get_index()]._uid);
        CHECK_GL_ERROR();
        _active_vbo = handle;
    }
}

static const unsigned GL_ELEMENT_FORMAT[] =
{
    GL_BYTE,
    GL_UNSIGNED_BYTE,
    GL_SHORT,
    GL_UNSIGNED_SHORT,
    GL_FIXED,
    GL_FLOAT
};

void RenderBackend::set_attribute_layout(Handle mat_handle, Handle vb_handle)
{
    if( mat_handle == _active_vao.first && vb_handle == _active_vao.second )
        return;

    auto& material = _materials[mat_handle.get_index()];
    auto& vb = _vbs[vb_handle.get_index()];
    ENSURE(material._uid != 0 && vb._uid != 0);

    if( _vao_support )
    {
        const auto k = std::make_pair(mat_handle, vb_handle);

        auto found = _vao_cache.find(k);
        if( found != _vao_cache.end() )
        {
            glBindVertexArray(found->second);
            return;
        }

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        CHECK_GL_ERROR();

        _vao_cache.insert(std::make_pair(k, vao));
    }

    set_vertex_buffer(vb_handle);

    for( uint8_t i = 0; i < VertexAttribute::kVertexAttributeCount; i++ )
    {
        auto va = (VertexAttribute::Enum)i;
        auto location = material._attributes[va].second;
        if( location != -1 && vb._layout.has(va) )
        {
            auto attribute = vb._layout.get_attribute(va);
            glEnableVertexAttribArray(location);
            glVertexAttribPointer(
                /*index*/ location,
                /*size*/ attribute.size,
                /*type*/ GL_ELEMENT_FORMAT[value(attribute.format)],
                /*normalized*/ attribute.normalized,
                /*stride*/ vb._layout.get_stride(),
                /*pointer*/ (uint8_t*)0+vb._layout.get_offset(va));
        }
    }

    CHECK_GL_ERROR();
    _active_vao = std::make_pair(mat_handle, vb_handle);
}

void RenderBackend::set_texture_layout(Handle handle)
{
    auto& material = _materials[handle.get_index()];
    for( uint8_t i = 0; i < material._texture_size; i++ )
    {
        glActiveTexture(GL_TEXTURE0+i);
        if( !material._textures[i].second.is_valid() )
        {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            auto& texture = _textures[material._textures[i].second.get_index()];
            glBindTexture(GL_TEXTURE_2D, texture._uid);
            texture.update_parameters();
        }
    }
}

void RenderBackend::draw(PrimitiveType type, uint32_t start, uint32_t num)
{
    ASSERT( _active_vbo.is_valid(), "Vertex buffer is required to draw.");

    set_attribute_layout(_active_material, _active_vbo);
    set_texture_layout(_active_material);

    if( _active_ibo.is_valid() )
    {
        auto& ib = _ibs[_active_ibo.get_index()];
        glDrawElements(GL_PRIMITIVE[value(type)], num, ib._element_size, (uint8_t*)0+start);
    }
    else
    {
        glDrawArrays(GL_PRIMITIVE[value(type)], start, num);
    }

    CHECK_GL_ERROR();
}

void RenderBackend::set_texture(unsigned unit, unsigned type, unsigned object)
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

bool RenderBackend::is_device_lost() const
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
        ABORT(file, line, "GL_%s", to_string(error));
}

NS_LEMON_GRAPHICS_END
