#include <graphics/private/index_buffer.hpp>
#include <graphics/private/backend.hpp>

NS_LEMON_GRAPHICS_BEGIN

size_t GL_INDEX_ELEMENT_SIZES[] =
{
    1,
    2
};

bool IndexBufferGL::initialize(const void* data, size_t size, IndexElementFormat format, MemoryUsage usage)
{
    dispose();

    glGenBuffers(1, &_object);
    if( !_object )
    {
        LOGW("failed to create index buffer object.");
        return false;
    }

    _size = size;
    _format = format;
    _usage = usage == MemoryUsage::DYNAMIC ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    return update_data(data);
}

void IndexBufferGL::dispose()
{
    if( _object != 0 )
        glDeleteBuffers(1, &_object);

    _object = 0;
}

bool IndexBufferGL::update_data(const void* data)
{
    if( !data )
    {
        LOGW("failed to update index buffer with nullptr.");
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

bool IndexBufferGL::update_data(const void* data, size_t start, size_t size, bool discard)
{
    if( start == 0 && size == _size )
        return update_data(data);

    if( data != nullptr )
    {
        LOGW("failed to update index buffer with nullptr.");
        return false;
    }

    if( start + size > _size )
    {
        LOGW("out-of-range while setting new index buffer data.");
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