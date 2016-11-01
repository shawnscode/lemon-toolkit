// @date 2016/10/26
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/frontend.hpp>
#include <graphics/backend/backend.hpp>
#include <graphics/details/frame.hpp>

NS_LEMON_GRAPHICS_BEGIN

bool RenderFrontend::initialize()
{
    _submit = new RenderFrame(1024, 1024*1024);
    _backend.reset(new RenderBackend());
    return true;
}

void RenderFrontend::dispose()
{
}

struct CreateVertexBuffer : public FrameCommand
{
    Handle handle;
    void* data;
    size_t size;
    VertexLayout layout;
    BufferUsage usage;

    void dispatch(RenderBackend& backend) override
    {
        backend.create_vertex_buffer(handle, data, size, layout, usage);
    }
};

Handle RenderFrontend::create_vertex_buffer(
    const void* data, size_t size, const VertexLayout& layout, BufferUsage usage)
{
    if( auto handle = _vb_handles.create() )
    {
        auto cvb = _submit->make<CreateVertexBuffer>();
        cvb->handle = handle;
        cvb->layout = layout;
        cvb->usage = usage;
        cvb->size = size;
        cvb->data = _submit->allocate(size);
        memcpy(cvb->data, data, size);
        return handle;
    }

    return Handle();
}

struct UpdateVertexBuffer : public FrameCommand
{
    Handle handle;
    uint16_t start;
    void* data;
    size_t size;

    void dispatch(RenderBackend& backend) override
    {
        backend.update_vertex_buffer(handle, start, data, size);
    }
};

void RenderFrontend::update_vertex_buffer(
    Handle handle, uint16_t start, const void* data, size_t size)
{
    if( _vb_handles.is_alive(handle) )
    {
        auto uvb = _submit->make<UpdateVertexBuffer>();
        uvb->handle = handle;
        uvb->start = start;
        uvb->size = size;
        uvb->data = _submit->allocate(size);
        memcpy(uvb->data, data, size);
    }
}

struct FreeVertexBuffer : public FrameCommand
{
    Handle handle;

    void dispatch(RenderBackend& backend) override
    {
        backend.free_vertex_buffer(handle);
    }
};

void RenderFrontend::free_vertex_buffer(Handle handle)
{
    if( _vb_handles.free(handle) )
    {
        auto fvb = _submit->make<FreeVertexBuffer>();
        fvb->handle = handle;
    }
}

struct CreateIndexBuffer : public FrameCommand
{
    Handle handle;
    void* data;
    size_t size;
    IndexElementFormat format;
    BufferUsage usage;

    void dispatch(RenderBackend& backend) override
    {
        backend.create_index_buffer(handle, data, size, format, usage);
    }
};

Handle RenderFrontend::create_index_buffer(
    const void* data, size_t size, IndexElementFormat format, BufferUsage usage)
{
    if( auto handle = _ib_handles.create() )
    {
        auto cib = _submit->make<CreateIndexBuffer>();
        cib->handle = handle;
        cib->format = format;
        cib->usage = usage;
        cib->size = size;
        cib->data = _submit->allocate(size);
        memcpy(cib->data, data, size);
        return handle;
    }

    return Handle();
}

struct UpdateIndexBuffer : public FrameCommand
{
    Handle handle;
    uint16_t start;
    void* data;
    size_t size;

    virtual void dispatch(RenderBackend& backend) override
    {
        backend.update_index_buffer(handle, start, data, size);
    }
};

void RenderFrontend::update_index_buffer(
    Handle handle, uint16_t start, const void* data, size_t size)
{
    if( _ib_handles.is_alive(handle) )
    {
        auto uvb = _submit->make<UpdateIndexBuffer>();
        uvb->handle = handle;
        uvb->start = start;
        uvb->size = size;
        uvb->data = _submit->allocate(size);
        memcpy(uvb->data, data, size);
    }
}

struct FreeIndexBuffer : public FrameCommand
{
    Handle handle;

    virtual void dispatch(RenderBackend& backend) override
    {
        backend.free_index_buffer(handle);
    }
};

void RenderFrontend::free_index_buffer(Handle handle)
{
    if( _ib_handles.free(handle) )
    {
        auto fvb = _submit->make<FreeIndexBuffer>();
        fvb->handle = handle;
    }
}

Handle RenderFrontend::create_render_state(const RenderState& in)
{
    if( auto handle = _states.create() )
    {
        *_states.fetch(handle) = in;
        return handle;
    }

    return Handle();
}

void RenderFrontend::update_render_state(Handle handle, const RenderState& in)
{
    if( auto state = _states.fetch(handle) )
    {
        *state = in;
    }
}

void RenderFrontend::free_render_state(Handle handle)
{
    _states.free(handle);
}

struct CreateProgram : public FrameCommand
{
    Handle handle;
    char* vs;
    char* fs;

    void dispatch(RenderBackend& backend) override
    {
        backend.create_program(handle, vs, fs);
    }
};

Handle RenderFrontend::create_program(const char* vs, const char* fs)
{
    if( auto handle = _material_handles.create() )
    {
        auto cp = _submit->make<CreateProgram>();
        cp->handle = handle;

        auto vs_len = strlen(vs);
        cp->vs = (char*)_submit->allocate(vs_len+1);
        strncpy(cp->vs, vs, vs_len);

        auto fs_len = strlen(fs);
        cp->fs = (char*)_submit->allocate(fs_len+1);
        strncpy(cp->fs, fs, fs_len);
        return handle;
    }

    return Handle();
}

struct CreateProgramUniform : public FrameCommand
{
    Handle handle;
    char* name;

    void dispatch(RenderBackend& backend) override
    {
        backend.create_program_uniform(handle, name);
    }
};

void RenderFrontend::create_program_uniform(
    Handle handle, const char* name)
{
    if( _material_handles.is_alive(handle) )
    {
        auto cpu = _submit->make<CreateProgramUniform>();
        cpu->handle = handle;

        auto len = strlen(name);
        cpu->name = (char*)_submit->allocate(len+1);
        strncpy(cpu->name, name, len);
    }
}

struct CreateProgramAttribute : public FrameCommand
{
    Handle handle;
    VertexAttribute::Enum attribute;
    char* name;

    void dispatch(RenderBackend& backend) override
    {
        backend.create_program_attribute(handle, attribute, name);
    }
};

void RenderFrontend::create_program_attribute(
    Handle handle, VertexAttribute::Enum va, const char* name)
{
    if( _material_handles.is_alive(handle) )
    {
        auto cpa = _submit->make<CreateProgramAttribute>();
        cpa->handle = handle;
        cpa->attribute = va;

        auto len = strlen(name);
        cpa->name = (char*)_submit->allocate(len+1);
        strncpy(cpa->name, name, len);
    }
}

struct FreeProgram : public FrameCommand
{
    Handle handle;

    void dispatch(RenderBackend& backend) override
    {
        backend.free_program(handle);
    }
};

void RenderFrontend::free_program(Handle handle)
{
    if( _material_handles.free(handle) )
    {
        auto fp = _submit->make<FreeProgram>();
        fp->handle = handle;
    }
}

struct CreateTexture : public FrameCommand
{
    Handle handle;
    void* data;
    TextureFormat format;
    TexturePixelFormat pixel_format;
    uint16_t width;
    uint16_t height;
    BufferUsage usage;

    void dispatch(RenderBackend& backend) override
    {
        backend.create_texture(
            handle, data, format, pixel_format, width, height, usage);
    }
};

Handle RenderFrontend::create_texture(
    const void* data,
    TextureFormat format, TexturePixelFormat pixel_format,
    uint16_t width, uint16_t height,
    BufferUsage usage)
{
    if( auto handle = _texture_handles.create() )
    {
        auto cib = _submit->make<CreateTexture>();
        cib->handle = handle;
        cib->format = format;
        cib->pixel_format = pixel_format;
        cib->width = width;
        cib->height = height;
        cib->usage = usage;

        auto size = size_of_texture(format, pixel_format, width, height);
        cib->data = _submit->allocate(size);
        memcpy(cib->data, data, size);
        return handle;
    }

    return Handle();
}

struct UpdateTextureMipmap : public FrameCommand
{
    Handle handle;
    bool mipmap;

    void dispatch(RenderBackend& backend) override
    {
        backend.update_texture_mipmap(handle, mipmap);
    }
};

void RenderFrontend::update_texture_mipmap(Handle handle, bool mipmap)
{
    if( _texture_handles.is_alive(handle) )
    {
        auto utm = _submit->make<UpdateTextureMipmap>();
        utm->handle = handle;
        utm->mipmap = mipmap;
    }
}

struct UpdateTextureAddress : public FrameCommand
{
    Handle handle;
    TextureCoordinate coordinate;
    TextureAddressMode wrap;

    void dispatch(RenderBackend& backend) override
    {
        backend.update_texture_address_mode(handle, coordinate, wrap);
    }
};

void RenderFrontend::update_texture_address_mode(
    Handle handle, TextureCoordinate coordinate, TextureAddressMode wrap)
{
    if( _texture_handles.is_alive(handle) )
    {
        auto uta = _submit->make<UpdateTextureAddress>();
        uta->handle = handle;
        uta->coordinate = coordinate;
        uta->wrap = wrap;
    }
}

struct UpdateTextureFilter : public FrameCommand
{
    Handle handle;
    TextureFilterMode filter;

    void dispatch(RenderBackend& backend) override
    {
        backend.update_texture_filter_mode(handle, filter);
    }
};

void RenderFrontend::update_texture_filter_mode(
    Handle handle, TextureFilterMode filter)
{
    if( _texture_handles.is_alive(handle) )
    {
        auto utf = _submit->make<UpdateTextureFilter>();
        utf->handle = handle;
        utf->filter = filter;
    }
}

struct FreeTexture : public FrameCommand
{
    Handle handle;

    void dispatch(RenderBackend& backend) override
    {
        backend.free_texture(handle);
    }
};

void RenderFrontend::free_texture(Handle handle)
{
    if( _texture_handles.free(handle) )
    {
        auto ft = _submit->make<FreeTexture>();
        ft->handle = handle;
    }
}

Handle RenderFrontend::allocate_uniform_buffer(size_t num)
{
    if( auto handle = _ub_views.create() )
    {
        auto object = _ub_views.fetch(handle);
        object->first = _uniform_buffer._position.fetch_add(num);
        object->num = num;
        object->used = 0;

        ASSERT(_uniform_buffer._position.load() < kMaxUniforms,
            "too many uniform (%d) update requests.", kMaxUniforms);
        return handle;
    }

    return Handle();
}

bool RenderFrontend::is_uniform_buffer_alive(Handle handle) const
{
    return _ub_views.is_alive(handle);
}

void RenderFrontend::update_uniform_buffer(
    Handle handle, math::StringHash hash, const UniformVariable& value)
{
    if( auto object = _ub_views.fetch(handle) )
    {
        for( uint32_t i = object->first; i < object->first+object->used; i++ )
        {
            if( _uniform_buffer._names[i] == hash )
            {
                _uniform_buffer._values[i] = value;
                return;
            }
        }

        ASSERT(object->used < object->num, "update uniforms out-of-range.");
        
        auto index = object->first + object->used++;
        _uniform_buffer._names[index] = hash;
        _uniform_buffer._values[index] = value;
    }
}

bool RenderFrontend::restore_video_context(SDL_Window* window)
{
    return _backend->initialize(window);
}

void RenderFrontend::dispose_video_context()
{
    _backend->dispose();
}

bool RenderFrontend::begin_frame()
{
    if( !_backend->begin_frame() )
        return false;

    _ub_views.clear();
    _uniform_buffer._position.store(0);
    return true;
}

struct ClearView : public FrameCommand
{
    ClearOption option;
    math::Color color;
    float depth;
    uint32_t stencil;

    void dispatch(RenderBackend& backend)
    {
        backend.clear(option, color, depth, stencil);
    }
};

void RenderFrontend::clear(
    ClearOption option, const math::Color& color, float depth, uint32_t stencil)
{
    auto cv = _submit->make<ClearView>();
    cv->option = option;
    cv->color = color;
    cv->depth = depth;
    cv->stencil = stencil;
}

void RenderFrontend::submit(const RenderDrawCall& drawcall)
{
    _drawcalls.push_back(drawcall);
}

void RenderFrontend::flush()
{
    for( size_t i = 0; i < _submit->_packet_tail; i++ )
    {
        _submit->_packets[i]->dispatch(*_backend);
    }

    for( size_t i = 0; i < _drawcalls.size(); i++ )
    {
        auto& dc = _drawcalls[i];
        if( dc.num <= 0 )
            continue;
        
        _backend->set_program(dc.program);
        _backend->set_vertex_buffer(dc.buffer_vertex);
        _backend->set_index_buffer(dc.buffer_index);

        if( auto state = _states.fetch(dc.state) )
        {
            _backend->set_scissor_test(
                state->scissor.enable,
                state->scissor.area);

            _backend->set_front_face(
                state->cull.winding);

            _backend->set_cull_face(
                state->cull.enable,
                state->cull.face);

            _backend->set_depth_test(
                state->depth.enable,
                state->depth.compare);

            _backend->set_depth_write(
                state->depth_write.enable,
                state->depth_write.bias_slope_scaled,
                state->depth_write.bias_constant);

            _backend->set_color_blend(
                state->blend.enable,
                state->blend.equation,
                state->blend.source_factor,
                state->blend.destination_factor);

            _backend->set_color_write(
                state->color_write);

            _backend->set_stencil_test(
                state->stencil.enable,
                state->stencil.compare,
                state->stencil.reference,
                state->stencil.mask);

            _backend->set_stencil_write(
                state->stencil_write.sfail,
                state->stencil_write.dpfail,
                state->stencil_write.dppass,
                state->stencil_write.mask);
        }

        if( auto shared_uniforms = _ub_views.fetch(dc.shared_uniforms) )
        {
            uint32_t end = shared_uniforms->first + shared_uniforms->used;
            for( uint32_t i = shared_uniforms->first; i < end; i++ )
            {
                _backend->update_program_uniform(dc.program,
                    _uniform_buffer._names[i],
                    _uniform_buffer._values[i]);
            }
        }

        if( auto uniforms = _ub_views.fetch(dc.uniforms) )
        {
            uint32_t end = uniforms->first + uniforms->used;
            for( uint32_t i = uniforms->first; i < end; i++ )
            {
                _backend->update_program_uniform(dc.program,
                    _uniform_buffer._names[i],
                    _uniform_buffer._values[i]);
            }   
        }

        _backend->draw(PrimitiveType::TRIANGLES, dc.first, dc.num);
    }

    _submit->clear();
    _drawcalls.clear();
}

void RenderFrontend::end_frame()
{
    flush();
    _backend->end_frame();
}

// void RenderFrontend::end_frame()
// {
//     ENSURE(_insync.fetch_add(1) == 0);

//     while( _render != nullptr )
//         std::this_thread::yield();

//     _render = _submit;
//     _submit = _frames[0] == _submit : _frames[1] : _frames[0];

//     ENSURE(_insync.fetch_sub(1) == 1);
// }

// bool RenderFrontend::initialize()
// {
//     _dispose.store(false);
//     _insync.store(0);

//     _backend_thread = std::thread(std::bind(RenderFrontend::consume, this));
//     _frames[0] = new RenderFrame(1024, 1024*1024);
//     _frames[1] = new RenderFrame(1024, 1024*1024);
//     return true;
// }

// void RenderFrontend::dispose()
// {
//     _insync.store(1);
//     _dispose.store(true);

//     _backend_thread.join();
//     _backend_thread = nullptr;

//     delete _frames[0];
//     delete _frames[1];
//     _frames[0] = _frames[1] = _submit = _render = nullptr;
// }


NS_LEMON_GRAPHICS_END
