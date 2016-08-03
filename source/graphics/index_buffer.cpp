#include <graphics/index_buffer.hpp>
#include <graphics/opengl.hpp>

NS_FLOW2D_GFX_BEGIN

bool IndexBuffer::initialize()
{
    ENSURE( !_device.is_device_lost() );

    if( _count == 0 || _size == 0 )
        return true;

    if( !_handle )
        glGenBuffers(1, &_handle);

    if( !_handle )
    {
        LOGW("failed to create vertex buffer object.");
        return false;
    }

    _device.set_index_buffer(_handle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _count*_size, 0, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    return true;
}

void IndexBuffer::dispose()
{
    if( _device.is_device_lost() )
        _handle = 0;

    if( _handle != 0 )
    {
        if( _device.get_bound_index_buffer() == _handle )
            _device.set_index_buffer(0);
        glDeleteBuffers(1, &_handle);
        _handle = 0;
    }
}

void IndexBuffer::on_device_restore()
{
    initialize();
    if( _data_pending ) set_data(_shadowed_data.get());
}

void IndexBuffer::set_shadowed(bool enable)
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

bool IndexBuffer::set_data(const void* data)
{
    if( !data )
    {
        LOGW("vertex buffer data could not be null.");
        return false;
    }

    if( _device.is_device_lost() && !_shadowed_data )
    {
        LOGW("vertex data assignment without shadow enable while device is lost.");
        return false;
    }

    if( _shadowed_data && data != _shadowed_data.get() )
        memcpy(_shadowed_data.get(), data, _count * _size );

    if( _handle && !_device.is_device_lost() )
    {
        _device.set_index_buffer(_handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _count * _size, data, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }
    else
        _data_pending = true;

    return true;
}

bool IndexBuffer::set_data_range(const void* data, unsigned start, unsigned count, bool discard)
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

    if( _handle && !_device.is_device_lost() )
    {
        _device.set_index_buffer(_handle);
        if( !discard || start != 0 )
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, start * _size, count * _size, data);
        else
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, count*_size, data, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }
    else
        _data_pending = true;

    return true;
}

NS_FLOW2D_GFX_END