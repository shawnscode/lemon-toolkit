// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/private/vertex_buffer.hpp>
#include <graphics/private/backend.hpp>

NS_LEMON_GRAPHICS_BEGIN

bool VertexBufferGL::initialize(const void* data, size_t size, const VertexLayout& layout, MemoryUsage usage)
{
    dispose();

    glGenBuffers(1, &_object);
    if( _object == 0 )
    {
        LOGW("failed to create vertex buffer object.");
        return false;
    }

    _size = size;
    _attributes = layout;
    _usage = usage == MemoryUsage::DYNAMIC ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    return update_data(data);
}

void VertexBufferGL::dispose()
{
    if( _object != 0 )
        glDeleteBuffers(1, &_object);

    _object = 0;
}

bool VertexBufferGL::update_data(const void* data)
{
    if( !data )
    {
        LOGW("failed to update vertex buffer with nullptr.");
        return false;
    }

    if( _object != 0 )
    {
        glBindBuffer(GL_ARRAY_BUFFER, _object);
        glBufferData(GL_ARRAY_BUFFER, _size*_attributes.get_stride(), data, _usage);
    }

    CHECK_GL_ERROR();
    return true;
}

bool VertexBufferGL::update_data(const void* data, size_t start, size_t size, bool discard)
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

    if( _object != 0 )
    {
        glBindBuffer(GL_ARRAY_BUFFER, _object);
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