#include <graphics/private/vertex_buffer.hpp>

NS_LEMON_GRAPHICS_BEGIN

VertexBuffer::VertexBuffer()
{
}

bool VertexBuffer::initialize(const void* data, unsigned size, const VertexAttributeLayout& layout, BufferUsage usage)
{
    glGenBuffers(1, &_buffer);
    if( !_buffer )
    {
        LOGW("failed to create vertex buffer object.");
        return false;
    }

    _size = size;
    _attributes = layout;
    _usage = usage == BufferUsage::DYNAMIC_DRAW ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    _buffer = 0;
    return update_data(data);
}

void VertexBuffer::dispose()
{
    if( _buffer != 0 )
        glDeleteBuffers(1, &_buffer);

    _buffer = 0;
}

bool VertexBuffer::update_data(const void* data)
{
    if( !data )
    {
        LOGW("failed to update vertex buffer with nullptr.");
        return false;
    }

    if( _buffer != 0 )
    {
        glBindBuffer(GL_ARRAY_BUFFER, _buffer);
        glBufferData(GL_ARRAY_BUFFER, _size*_attributes.get_stride(), data, _usage);
    }

    CHECK_GL_ERROR();
    return true;
}

bool VertexBuffer::update_data_range(const void* data, unsigned start, unsigned size, bool discard)
{
    if( start == 0 && size == _size )
        return update_data(data);

    if( data != nullptr )
    {
        LOGW("failed to update vertex buffer with nullptr.");
        return false;
    }

    if( start + size > _size )
    {
        LOGW("out-of-range while setting new vertex buffer data.");
        return false;
    }

    if( !size )
        return true;

    if( _buffer != 0 )
    {
        glBindBuffer(GL_ARRAY_BUFFER, _buffer);

        uint16_t stride = _attributes.get_stride();
        if( !discard || start != 0 )
            glBufferSubData(GL_ARRAY_BUFFER, start * stride, size * stride, data);
        else
            glBufferData(GL_ARRAY_BUFFER, size*stride, data, _usage);
    }

    CHECK_GL_ERROR();
    return true;
}

NS_LEMON_GRAPHICS_END