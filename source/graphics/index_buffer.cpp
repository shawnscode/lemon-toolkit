#include <graphics/index_buffer.hpp>
#include <graphics/opengl.hpp>

NS_FLOW2D_GFX_BEGIN

bool IndexBuffer::restore(unsigned count, unsigned size, bool dynamic)
{
    _count = count;
    _size = size;
    _dynamic = dynamic;
    return restore();
}

bool IndexBuffer::restore()
{
    if( _device.is_device_lost() )
    {
        LOGW("trying to restore index buffer while device is lost.");
        return false;
    }

    release();

    if( _count == 0 || _size == 0 )
        return false;

    glGenBuffers(1, &_object);
    if( !_object )
    {
        LOGW("failed to create index buffer object.");
        return false;
    }

    if( _shadowed_data )
    {
        _device.set_index_buffer(_object);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _count*_size, _shadowed_data.get(), _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }

    CHECK_GL_ERROR();
    return true;
}

void IndexBuffer::release()
{
    if( _object != 0 && !_device.is_device_lost() )
    {
        if( _device.get_bound_index_buffer() == _object )
            _device.set_index_buffer(0);
        glDeleteBuffers(1, &_object);
    }

    _object = 0;
}

void IndexBuffer::bind_to_device()
{
    if( !_object || _device.is_device_lost() )
    {
        LOGW("failed to bind index buffer while device is lost.");
        return;
    }

    _device.set_index_buffer(_object);
}

void IndexBuffer::set_shadowed(bool enable)
{
    if( (_shadowed_data != nullptr) != enable )
    {
        if( enable )
        {
            _shadowed_data.reset( new unsigned char[_count*_size] );
            if( !_shadowed_data )
                LOGW("failed to create shadowed data for index buffer.");
        }
        else
            _shadowed_data.reset();
    }
}

bool IndexBuffer::set_data(const void* data)
{
    if( !data )
    {
        LOGW("index buffer data could not be null.");
        return false;
    }

    if( _shadowed_data && data != _shadowed_data.get() )
        memcpy(_shadowed_data.get(), data, _count * _size );

    if( _object && !_device.is_device_lost() )
    {
        _device.set_index_buffer(_object);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _count * _size, data, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }

    CHECK_GL_ERROR();
    return true;
}

bool IndexBuffer::set_data_range(const void* data, unsigned start, unsigned count, bool discard)
{
    if( start == 0 && count == _count )
        return set_data(data);

    if( !data )
    {
        LOGW("index buffer data could not be null.");
        return false;
    }
    
    if( start + count > _count )
    {
        LOGW("out-of-range while setting new index buffer data.");
        return false;
    }

    if( !count )
        return true;

    if( _shadowed_data && _shadowed_data.get() + start*_size != data )
        memcpy(_shadowed_data.get() + start * _size, data, count * _size);

    if( _object && !_device.is_device_lost() )
    {
        _device.set_index_buffer(_object);
        if( !discard || start != 0 )
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, start * _size, count * _size, data);
        else
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, count*_size, data, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }

    CHECK_GL_ERROR();
    return true;
}

NS_FLOW2D_GFX_END