// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/renderer.hpp>
#include <graphics/private/context.hpp>

NS_LEMON_GRAPHICS_BEGIN

#define CTX(PROPERTY) (_context->PROPERTY)

bool Renderer::initialize()
{
    _context.reset(new (std::nothrow) RendererContext());
    if( _context == nullptr )
        return false;

    _frame_began = false;
    return true;
}

bool Renderer::restore(SDL_Window* window)
{
    return CTX(backend).initialize(window);
}

void Renderer::release()
{
    CTX(backend).dispose();
}

Handle Renderer::create_program(const char* vs, const char* ps)
{
    ASSERT(!_frame_began, "\'create_program\' could not be called during frame rendering.");

    Handle handle = CTX(programs).create(vs, ps);
    if( !handle.is_valid() )
        LOGW("failed to allocate program.");
    return handle;
}

bool Renderer::update_uniform_1f(Handle handle, const char* name, const math::Vector<1, float>& v)
{
    ASSERT(!_frame_began, "\'update_uniform\' could not be called during frame rendering.");

    Program* program = CTX(programs).get(handle);
    if( program == nullptr )
        return false;

    return program->set_uniform_1f(name, v);
}

bool Renderer::update_uniform_2f(Handle handle, const char* name, const math::Vector<2, float>& v)
{
    ASSERT(!_frame_began, "\'update_uniform\' could not be called during frame rendering.");

    Program* program = CTX(programs).get(handle);
    if( program == nullptr )
        return false;

    return program->set_uniform_2f(name, v);
}

bool Renderer::update_uniform_3f(Handle handle, const char* name, const math::Vector<3, float>& v)
{
    ASSERT(!_frame_began, "\'update_uniform\' could not be called during frame rendering.");

    Program* program = CTX(programs).get(handle);
    if( program == nullptr )
        return false;

    return program->set_uniform_3f(name, v);
}

bool Renderer::update_uniform_4f(Handle handle, const char* name, const math::Vector<4, float>& v)
{
    ASSERT(!_frame_began, "\'update_uniform\' could not be called during frame rendering.");

    Program* program = CTX(programs).get(handle);
    if( program == nullptr )
        return false;

    return program->set_uniform_4f(name, v);
}

bool Renderer::update_uniform_2fm(Handle handle, const char* name, const math::Matrix<2, 2, float>& v)
{
    ASSERT(!_frame_began, "\'update_uniform\' could not be called during frame rendering.");

    Program* program = CTX(programs).get(handle);
    if( program == nullptr )
        return false;

    return program->set_uniform_2fm(name, v);
}

bool Renderer::update_uniform_3fm(Handle handle, const char* name, const math::Matrix<3, 3, float>& v)
{
    ASSERT(!_frame_began, "\'update_uniform\' could not be called during frame rendering.");

    Program* program = CTX(programs).get(handle);
    if( program == nullptr )
        return false;

    return program->set_uniform_3fm(name, v);
}

bool Renderer::update_uniform_4fm(Handle handle, const char* name, const math::Matrix<4, 4, float>& v)
{
    ASSERT(!_frame_began, "\'update_uniform\' could not be called during frame rendering.");

    Program* program = CTX(programs).get(handle);
    if( program == nullptr )
        return false;

    return program->set_uniform_4fm(name, v);
}

bool Renderer::update_uniform_texture(Handle handle, const char* name, Handle texhandle)
{
    ASSERT(!_frame_began, "\'update_uniform\' could not be called during frame rendering.");

    // Program* program = CTX(programs).get(handle);
    // Texture2D* texture = CTX(texture2ds).get(texhandle);
    // if( program == nullptr || texture == nullptr )
    //     return false;

    // return program->set_uniform_texture(name, texture->get_handle());
    return false;
}

Handle Renderer::create_vertex_buffer(
    const void* data,
    size_t size,
    const VertexAttributeLayout& layout,
    BufferUsage usage)
{
    ASSERT(!_frame_began, "\'create_vertex_buffer\' could not be called during frame rendering.");

    Handle handle = CTX(vertex_buffers).create(data, size, layout, usage);
    if( !handle.is_valid() )
        LOGW("failed to allocate vertex buffer.");
    return handle;
}

bool Renderer::update_vertex_buffer(Handle handle, const void* data)
{
    ASSERT(!_frame_began, "\'update_vertex_buffer\' could not be called during frame rendering.");

    VertexBuffer* vb = CTX(vertex_buffers).get(handle);
    if( vb == nullptr )
        return false;

    return vb->update_data(data);
}

bool Renderer::update_vertex_buffer(Handle handle, const void* data, unsigned start, unsigned size, bool discard)
{
    ASSERT(!_frame_began, "\'update_vertex_buffer\' could not be called during frame rendering.");

    VertexBuffer* vb = CTX(vertex_buffers).get(handle);
    if( vb == nullptr )
        return false;

    return vb->update_data_range(data, start, size, discard);
}

void Renderer::free_vertex_buffer(Handle handle)
{
    ASSERT(!_frame_began, "\'free_vertex_buffer\' could not be called during frame rendering.");

    CTX(vertex_buffers).free(handle);
    CTX(vaocache).free_vertex_buffer(handle);
}

Handle Renderer::create_index_buffer(
    const void* data,
    size_t size,
    IndexElementFormat format,
    BufferUsage usage)
{
    ASSERT(!_frame_began, "\'create_index_buffer\' could not be called during frame rendering.");

    Handle handle = CTX(index_buffers).create(data, size, format, usage);
    if( !handle.is_valid() )
        LOGW("failed to allocate index buffer.");
    return handle;
}

bool Renderer::update_index_buffer(Handle handle, const void* data)
{
    ASSERT(!_frame_began, "\'update_index_buffer\' could not be called during frame rendering.");

    IndexBuffer* ib = CTX(index_buffers).get(handle);
    if( ib == nullptr )
        return false;

    return ib->update_data(data);
}

bool Renderer::update_index_buffer(Handle handle, const void* data, unsigned start, unsigned size, bool discard)
{
    ASSERT(!_frame_began, "\'update_index_buffer\' could not be called during frame rendering.");

    IndexBuffer* ib = CTX(index_buffers).get(handle);
    if( ib == nullptr )
        return false;

    return ib->update_data_range(data, start, size, discard);
}

void Renderer::free_index_buffer(Handle handle)
{
    ASSERT(!_frame_began, "\'free_index_buffer\' could not be called during frame rendering.");

    CTX(index_buffers).free(handle);
}

bool Renderer::begin_frame()
{
    ENSURE(CTX(drawcalls).size() == 0);

    if( !CTX(backend).begin_frame() )
        return false;

    _frame_began = true;
    return true;
}

void Renderer::clear(ClearOption option, const math::Color& color, float depth, unsigned stencil)
{
    ASSERT(_frame_began, "\'clear\' could only be called during rendering frame.");
    CTX(backend).clear(option, color, depth, stencil);
}

void Renderer::submit(
    RenderLayer layer, RenderState state,
    Handle program, Handle vb, Handle ib,
    uint32_t depth, uint32_t start, uint32_t num)
{
    ASSERT(_frame_began, "\'submit\' could only be called during rendering frame.");

    RenderDrawcall drawcall;
    drawcall.state = state;
    drawcall.program = program;
    drawcall.vertex_buffer = vb;
    drawcall.index_buffer = ib;
    drawcall.start = start;
    drawcall.vertex_count = num;
    drawcall.sort_value = SortValue::encode(layer, program, depth);

    {
        std::unique_lock<std::mutex> L(CTX(mutex));
        CTX(drawcalls).push_back(drawcall);
    }
}

void Renderer::flush()
{
    ASSERT(_frame_began, "\'flush\' could only be called during rendering frame.");

    // std::unique_lock<std::mutex> L(_context->frame_mutex);
    // _context->frame.sort();
    // for( auto& drawcall : _context->frame )
    // {
        // auto program = _context->programs.get(drawcall.program);
        // auto ub = _context->uniform_buffers.get(drawcall.uniform_buffer);
        // auto vb = CTX(vertex_buffers).get(drawcall.vertex_buffer);
        // auto ib = CTX(index_buffers).get(drawcall.index_buffer);

        // auto hprogram = program->get_opengl_handle();

        // if( _supportVao )
        // {
        //     program->get_vao_handle();
        // }

        // CTX(backend).set_shader(program->get_opengl_handle());
        // CTX(backend).set_vertex_attributes(vb->get_attributes());
        // CTX(backend).set_


        // ub->get_opengl_handle();
        // vb->get_opengl_handle();
        // ib->get_opengl_handle();
    // }
    // _context->frame.clear();
}

void Renderer::end_frame()
{
    ASSERT(_frame_began, "\'end_frame\' could only be called during rendering frame.");

    flush();
    CTX(backend).end_frame();
    _frame_began = false;
}

NS_LEMON_GRAPHICS_END