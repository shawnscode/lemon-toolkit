#include <graphics/private/index_buffer.hpp>
#include <graphics/private/backend.hpp>

NS_LEMON_GRAPHICS_BEGIN

unsigned GL_INDEX_ELEMENT_SIZES[] =
{
    1,
    2
};

bool IndexBufferGL::initialize(const void* data, unsigned size, IndexElementFormat format, MemoryUsage usage)
{
    ENSURE_NOT_RENDER_PHASE;

    dispose();

    glGenBuffers(1, &_object);
    if( !_object )
    {
        LOGW("failed to create vertex buffer object.");
        return false;
    }

    _size = size;
    _format = format;
    _usage = usage == MemoryUsage::DYNAMIC ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    return update_data(data);
}

void IndexBufferGL::dispose()
{
    ENSURE_NOT_RENDER_PHASE;

    if( _object != 0 )
        glDeleteBuffers(1, &_object);

    _object = 0;
}

void IndexBufferGL::bind()
{
    if( _object != 0 )
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _object);
}

bool IndexBufferGL::update_data(const void* data)
{
    ENSURE_NOT_RENDER_PHASE;

    if( !data )
    {
        LOGW("failed to update vertex buffer with nullptr.");
        return false;
    }

    if( _object != 0 )
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _object);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _size*GL_INDEX_ELEMENT_SIZES[value(_format)], data, _usage);
    }

    CHECK_GL_ERROR();
    return true;
}

bool IndexBufferGL::update_data(const void* data, unsigned start, unsigned size, bool discard)
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
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _object);

        uint16_t stride = GL_INDEX_ELEMENT_SIZES[value(_format)];
        if( !discard || start != 0 )
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, start * stride, size * stride, data);
        else
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, size*stride, data, _usage);
    }

    CHECK_GL_ERROR();
    return true;
}

NS_LEMON_GRAPHICS_END