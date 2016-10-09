// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/renderer.hpp>
#include <graphics/private/context.hpp>

NS_LEMON_GRAPHICS_BEGIN

bool Renderer::initialize()
{
    _context.reset(new (std::nothrow) RendererContext());
    if( _context == nullptr )
        return false;

    return true;
}

Handle Renderer::create_vertex_buffer(
    const void* data,
    size_t size,
    const VertexAttributeLayout& layout,
    BufferUsage usage)
{
    Handle handle = _context->vertex_buffers.create(data, size, layout, usage);
    if( !handle.is_valid() )
        LOGW("failed to allocate vertex buffer.");
    return handle;
}

bool Renderer::update_vertex_buffer(Handle handle, const void* data)
{
    VertexBuffer* vb = _context->vertex_buffers.get(handle);
    if( vb == nullptr )
        return false;

    return vb->update_data(data);
}

bool Renderer::update_vertex_buffer(Handle handle, const void* data, unsigned start, unsigned size, bool discard)
{
    VertexBuffer* vb = _context->vertex_buffers.get(handle);
    if( vb == nullptr )
        return false;

    return vb->update_data_range(data, start, size, discard);
}

void Renderer::free_vertex_buffer(Handle handle)
{
    _context->vertex_buffers.free(handle);
}

Handle Renderer::create_index_buffer(
    const void* data,
    size_t size,
    IndexElementFormat format,
    BufferUsage usage)
{
    Handle handle = _context->index_buffers.create(data, size, format, usage);
    if( !handle.is_valid() )
        LOGW("failed to allocate index buffer.");
    return handle;
}

bool Renderer::update_index_buffer(Handle handle, const void* data)
{
    IndexBuffer* ib = _context->index_buffers.get(handle);
    if( ib == nullptr )
        return false;

    return ib->update_data(data);
}

bool Renderer::update_index_buffer(Handle handle, const void* data, unsigned start, unsigned size, bool discard)
{
    IndexBuffer* ib = _context->index_buffers.get(handle);
    if( ib == nullptr )
        return false;

    return ib->update_data_range(data, start, size, discard);
}

void Renderer::free_index_buffer(Handle handle)
{
    _context->index_buffers.free(handle);
}

bool Renderer::begin_frame()
{
    _context->frame.clear();
    return true;
}

void Renderer::submit(RenderLayer layer, RenderState state, Handle program, Handle uniform, uint32_t depth, uint32_t start, uint32_t num)
{
    // RenderDraw drawcall;
    // drawcall.program = program;
    // drawcall.uniform_buffer = uniform;
    // drawcall.vertex_buffer = vb;
    // drawcall.index_buffer = ib;
    // drawcall.state = state;
    // drawcall.sort_value = SortValue::encode(layer, program, depth);

    // {
    //     std::unique_lock<std::mutex> L(_context->frame_mutex);
    //     _context->frame.push_back(drawcall);
    // }
}

void Renderer::flush()
{
    auto backend = core::get_subsystem<Backend>();

    std::unique_lock<std::mutex> L(_context->frame_mutex);
    _context->frame.sort();
    for( auto& drawcall : _context->frame )
    {
        // auto program = _context->programs.get(drawcall.program);
        // auto ub = _context->uniform_buffers.get(drawcall.uniform_buffer);
        // auto vb = _context->vertex_buffers.get(drawcall.vertex_buffer);
        // auto ib = _context->index_buffers.get(drawcall.index_buffer);

        // auto hprogram = program->get_opengl_handle();

        // if( _supportVao )
        // {
        //     program->get_vao_handle();
        // }

        // backend->set_shader(program->get_opengl_handle());
        // backend->set_vertex_attributes(vb->get_attributes());
        // backend->set_


        // ub->get_opengl_handle();
        // vb->get_opengl_handle();
        // ib->get_opengl_handle();
    }
    _context->frame.clear();
}

void Renderer::end_frame()
{
    flush();
}

NS_LEMON_GRAPHICS_END