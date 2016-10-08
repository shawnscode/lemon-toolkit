// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/frontend.hpp>
#include <graphics/private/context.hpp>

NS_LEMON_GRAPHICS_BEGIN

bool Frontend::initialize()
{
    _context.reset(new (std::nothrow) FrontendContext());
    if( _context == nullptr )
        return false;

    return true;
}

Handle Frontend::create_vertex_buffer(
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

bool Frontend::update_vertex_buffer(Handle handle, const void* data)
{
    VertexBuffer* vb = _context->vertex_buffers.get(handle);
    if( vb == nullptr )
        return false;

    return vb->update_data(data);
}

bool Frontend::update_vertex_buffer(Handle handle, const void* data, unsigned start, unsigned size, bool discard)
{
    VertexBuffer* vb = _context->vertex_buffers.get(handle);
    if( vb == nullptr )
        return false;

    return vb->update_data_range(data, start, size, discard);
}

void Frontend::free_vertex_buffer(Handle handle)
{
    _context->vertex_buffers.free(handle);
}

Handle Frontend::create_index_buffer(
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

bool Frontend::update_index_buffer(Handle handle, const void* data)
{
    IndexBuffer* ib = _context->index_buffers.get(handle);
    if( ib == nullptr )
        return false;

    return ib->update_data(data);
}

bool Frontend::update_index_buffer(Handle handle, const void* data, unsigned start, unsigned size, bool discard)
{
    IndexBuffer* ib = _context->index_buffers.get(handle);
    if( ib == nullptr )
        return false;

    return ib->update_data_range(data, start, size, discard);
}

void Frontend::free_index_buffer(Handle handle)
{
    _context->index_buffers.free(handle);
}


// IndexBufferHandle Frontend::create_vertex_buffer(
//     const uint8_t* data,
//     size_t size,
//     const VertexAttributeLayout& layout,
//     BufferUsage usage)
// {
//     ASSERT_MAIN_THREAD("resource manipulation could only be done in main thread.");

//     VertexBufferHandle handle;
//     handle.index = _context->vertex_buffer_handle.allocate();

//     if( is_valid(handle) )
//     {
//         // _vertexBuffers[handle.index]._
//         _commands->write(FrontendCommand::CreateVertexBuffer);
//         _commands->write(handle);
//         _commands->write(size);
//         _commands->write(data, size);
//         _commands->write(usage);
//     }
//     else
//     {
//         LOGW("failed to allocate vertex buffer handle.");
//     }

//     return handle;
// }

NS_LEMON_GRAPHICS_END