// @date 2016/10/26
// @author Mao Jingkai(oammix@gmail.com)

#include <core/task.hpp>

#include <graphics/frontend.hpp>
#include <graphics/backend/backend.hpp>
#include <graphics/backend/task.hpp>
#include <graphics/backend/frame.hpp>

NS_LEMON_GRAPHICS_BEGIN

bool RenderFrontend::initialize()
{
    _frames[0] = new RenderFrame(1024, 1024*1024);
    _frames[1] = new RenderFrame(1024, 1024*1024);

    _draw = nullptr;
    _submit = _frames[0];
    _backend.reset(new RenderBackend());
    return true;
}

void RenderFrontend::dispose()
{
    if( auto task = core::get_subsystem<core::TaskSystem>() )
        task->wait(_paint);

    for( size_t i = 0; i < 2; i ++ )
    {
        if( _frames[i] != nullptr )
        {
            delete _frames[i];
            _frames[i] = nullptr;
        }
    }

    _submit = nullptr;
    _draw = nullptr;
    _backend.reset();
}

Handle RenderFrontend::create_vertex_buffer(
    const void* data, size_t size, const VertexLayout& layout, BufferUsage usage)
{
    if( auto handle = _vb_handles.create() )
    {
        auto cvb = _submit->create_task<CreateVertexBuffer>();
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

void RenderFrontend::update_vertex_buffer(
    Handle handle, uint16_t start, const void* data, size_t size)
{
    if( _vb_handles.is_alive(handle) )
    {
        auto uvb = _submit->create_task<UpdateVertexBuffer>();
        uvb->handle = handle;
        uvb->start = start;
        uvb->size = size;
        uvb->data = _submit->allocate(size);
        memcpy(uvb->data, data, size);
    }
}

void RenderFrontend::free_vertex_buffer(Handle handle)
{
    if( _vb_handles.free(handle) )
    {
        auto fvb = _submit->create_task<FreeVertexBuffer>();
        fvb->handle = handle;
    }
}

Handle RenderFrontend::create_index_buffer(
    const void* data, size_t size, IndexElementFormat format, BufferUsage usage)
{
    if( auto handle = _ib_handles.create() )
    {
        auto cib = _submit->create_task<CreateIndexBuffer>();
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

void RenderFrontend::update_index_buffer(
    Handle handle, uint16_t start, const void* data, size_t size)
{
    if( _ib_handles.is_alive(handle) )
    {
        auto uvb = _submit->create_task<UpdateIndexBuffer>();
        uvb->handle = handle;
        uvb->start = start;
        uvb->size = size;
        uvb->data = _submit->allocate(size);
        memcpy(uvb->data, data, size);
    }
}

void RenderFrontend::free_index_buffer(Handle handle)
{
    if( _ib_handles.free(handle) )
    {
        auto fvb = _submit->create_task<FreeIndexBuffer>();
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

Handle RenderFrontend::create_program(const char* vs, const char* fs)
{
    if( auto handle = _material_handles.create() )
    {
        auto cp = _submit->create_task<CreateProgram>();
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

void RenderFrontend::create_program_uniform(
    Handle handle, const char* name)
{
    if( _material_handles.is_alive(handle) )
    {
        auto cpu = _submit->create_task<CreateProgramUniform>();
        cpu->handle = handle;

        auto len = strlen(name);
        cpu->name = (char*)_submit->allocate(len+1);
        strncpy(cpu->name, name, len);
    }
}

void RenderFrontend::create_program_attribute(
    Handle handle, VertexAttribute::Enum va, const char* name)
{
    if( _material_handles.is_alive(handle) )
    {
        auto cpa = _submit->create_task<CreateProgramAttribute>();
        cpa->handle = handle;
        cpa->attribute = va;

        auto len = strlen(name);
        cpa->name = (char*)_submit->allocate(len+1);
        strncpy(cpa->name, name, len);
    }
}

void RenderFrontend::free_program(Handle handle)
{
    if( _material_handles.free(handle) )
    {
        auto fp = _submit->create_task<FreeProgram>();
        fp->handle = handle;
    }
}

Handle RenderFrontend::create_texture(
    const void* data,
    TextureFormat format, TexturePixelFormat pixel_format,
    uint16_t width, uint16_t height,
    BufferUsage usage)
{
    if( auto handle = _texture_handles.create() )
    {
        auto cib = _submit->create_task<CreateTexture>();
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

void RenderFrontend::update_texture_mipmap(Handle handle, bool mipmap)
{
    if( _texture_handles.is_alive(handle) )
    {
        auto utm = _submit->create_task<UpdateTextureMipmap>();
        utm->handle = handle;
        utm->mipmap = mipmap;
    }
}

void RenderFrontend::update_texture_address_mode(
    Handle handle, TextureCoordinate coordinate, TextureAddressMode wrap)
{
    if( _texture_handles.is_alive(handle) )
    {
        auto uta = _submit->create_task<UpdateTextureAddress>();
        uta->handle = handle;
        uta->coordinate = coordinate;
        uta->wrap = wrap;
    }
}

void RenderFrontend::update_texture_filter_mode(
    Handle handle, TextureFilterMode filter)
{
    if( _texture_handles.is_alive(handle) )
    {
        auto utf = _submit->create_task<UpdateTextureFilter>();
        utf->handle = handle;
        utf->filter = filter;
    }
}

void RenderFrontend::free_texture(Handle handle)
{
    if( _texture_handles.free(handle) )
    {
        auto ft = _submit->create_task<FreeTexture>();
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
    if( _backend->is_device_lost() )
        return false;

    _ub_views.clear();
    _uniform_buffer._position.store(0);
    return true;
}

void RenderFrontend::clear(
    ClearOption option, const math::Color& color, float depth, uint32_t stencil)
{
    auto cv = _submit->create_task<ClearView>();
    cv->option = option;
    cv->color = color;
    cv->depth = depth;
    cv->stencil = stencil;
}

void RenderFrontend::submit(const RenderDrawCall& drawcall)
{
    _submit->submit(drawcall);
}

void RenderFrontend::flush()
{
    auto task = core::get_subsystem<core::TaskSystem>();
    task->wait(_paint);

    ENSURE(_draw == nullptr);
    _draw = _submit;
    _submit = _submit == _frames[0] ? _frames[1] : _frames[0];

    _paint = task->create("graphics.draw", &RenderFrontend::draw, this);
    task->run(_paint);
}

void RenderFrontend::end_frame()
{
    flush();
}

void RenderFrontend::draw()
{
    if( _backend->begin_frame() )
    {
        for( size_t i = 0; i < _draw->_packet_tail; i++ )
        {
            _draw->_packets[i]->dispatch(*_backend);
        }

        for( auto dc : _draw->_drawcalls )
        {
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
    }

    _backend->end_frame();
    _draw->clear();
    _draw = nullptr;
}

NS_LEMON_GRAPHICS_END
