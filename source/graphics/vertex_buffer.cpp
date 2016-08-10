#include <graphics/vertex_buffer.hpp>
#include <graphics/opengl.hpp>

NS_FLOW2D_GFX_BEGIN

bool VertexBuffer::restore(unsigned count, unsigned size, bool dynamic)
{
    _count = count;
    _size = size;
    _dynamic = dynamic;
    return restore();
}

bool VertexBuffer::restore()
{
    if( _device.is_device_lost() )
    {
        LOGW("trying to restore vertex buffer while device is lost.");
        return false;
    }

    if( _count == 0 || _size == 0 )
        return true;

    if( _object == 0 )
    {
        glGenBuffers(1, &_object);
        if( !_object )
        {
            LOGW("failed to create vertex buffer object.");
            return false;
        }
    }

    if( _data_pending )
    {
        _device.set_vertex_buffer(_object);
        glBufferData(GL_ARRAY_BUFFER, _count*_size, _shadowed_data.get(), _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }

    CHECK_GL_ERROR();
    return true;
}

void VertexBuffer::release()
{
    if( _object != 0 && !_device.is_device_lost() )
    {
        if( _device.get_bound_vertex_buffer() == _object )
            _device.set_vertex_buffer(0);
        glDeleteBuffers(1, &_object);
    }

    _object = 0;
}

void VertexBuffer::bind_to_device()
{
    if( !_object || _device.is_device_lost() )
    {
        LOGW("failed to bind vertex buffer while device is lost.");
        return;
    }

    _device.set_vertex_buffer(_object);
}

void VertexBuffer::set_shadowed(bool enable)
{
    if( (_shadowed_data != nullptr) != enable )
    {
        if( enable )
        {
            _shadowed_data.reset( new unsigned char[_count*_size] );
            if( !_shadowed_data )
                LOGW("failed to create shadowed data for vertex buffer.");
        }
        else
            _shadowed_data.reset();
    }
}

bool VertexBuffer::set_data(const void* data)
{
    if( !data )
    {
        LOGW("vertex buffer data could not be null.");
        return false;
    }

    if( _shadowed_data && data != _shadowed_data.get() )
        memcpy(_shadowed_data.get(), data, _count * _size );

    if( _object && !_device.is_device_lost() )
    {
        _device.set_vertex_buffer(_object);
        glBufferData(GL_ARRAY_BUFFER, _count * _size, data, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }
    else
        _data_pending = true;

    CHECK_GL_ERROR();
    return true;
}

bool VertexBuffer::set_data_range(const void* data, unsigned start, unsigned count, bool discard)
{
    if( start == 0 && count == _count )
        return set_data(data);

    if( !data )
    {
        LOGW("vertex buffer data could not be null.");
        return false;
    }
    
    if( start + count > _count )
    {
        LOGW("out-of-range while setting new vertex buffer data.");
        return false;
    }

    if( !count )
        return true;

    if( _shadowed_data && _shadowed_data.get() + start*_size != data )
        memcpy(_shadowed_data.get() + start * _size, data, count * _size);

    if( _object && !_device.is_device_lost() )
    {
        _device.set_vertex_buffer(_object);
        if( !discard || start != 0 )
            glBufferSubData(GL_ARRAY_BUFFER, start * _size, count * _size, data);
        else
            glBufferData(GL_ARRAY_BUFFER, count*_size, data, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }
    else
        _data_pending = true;

    CHECK_GL_ERROR();
    return true;
}

NS_FLOW2D_GFX_END