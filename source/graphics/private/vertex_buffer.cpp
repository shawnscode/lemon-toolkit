// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/private/vertex_buffer.hpp>
#include <graphics/private/backend.hpp>

NS_LEMON_GRAPHICS_BEGIN

bool VertexBufferGL::initialize(const void* data, unsigned size, const VertexLayout& layout, MemoryUsage usage)
{
    ENSURE_NOT_RENDER_PHASE;

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
    ENSURE_NOT_RENDER_PHASE;

    if( _object != 0 )
        glDeleteBuffers(1, &_object);

    _object = 0;
}

bool VertexBufferGL::update_data(const void* data)
{
    ENSURE_NOT_RENDER_PHASE;

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

bool VertexBufferGL::update_data(const void* data, unsigned start, unsigned size, bool discard)
{
    ENSURE_NOT_RENDER_PHASE;

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