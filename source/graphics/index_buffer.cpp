#include <graphics/index_buffer.hpp>
#include <graphics/opengl.hpp>

NS_LEMON_GRAPHICS_BEGIN

unsigned GL_INDEX_ELEMENT_SIZES[] =
{
    1,
    2
};

IndexBuffer::IndexBuffer(Device& device)
: GraphicsObject(device)
{
    _dynamic    = false;
    _shadowed   = false;
    _index_count = 0;
}

bool IndexBuffer::restore(const void* data,
    unsigned vcount, IndexElementFormat format, bool dynamic)
{
    if( _device.is_device_lost() )
    {
        LOGW("trying to restore index buffer while device is lost.");
        return false;
    }

    release();

    glGenBuffers(1, &_object);
    if( !_object )
    {
        LOGW("failed to create index buffer object.");
        return false;
    }

    _format         = _format;
    _index_count    = vcount;
    _dynamic        = dynamic;

    if( get_stride() == 0 || vcount == 0 )
    {
        _shadowed_data.reset();
        return true;
    }

    if( _shadowed )
    {
        _shadowed_data.reset( new uint8_t[get_size()] );
        if( !_shadowed_data )
        {
            LOGW("failed to create shadowed data for index buffer.");
            return false;
        }
    }

    CHECK_GL_ERROR();
    return update_data(data);
}

bool IndexBuffer::restore()
{
    if( !_shadowed_data )
    {
        LOGW("failed to restore index buffer without shadowed data.");
        return false;
    }

    return restore(_shadowed_data.get(), _index_count, _format, _dynamic);
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

void IndexBuffer::set_shadowed(bool shadowed)
{
    if( _shadowed != shadowed )
    {
        if( shadowed && get_size() > 0 )
        {
            _shadowed_data.reset( new uint8_t[get_size()] );
            if( !_shadowed_data )
                LOGW("failed to create shadowed data for index buffer.");
        }
        else
            _shadowed_data.reset();
    }

    _shadowed = shadowed;
}

bool IndexBuffer::update_data(const void* data)
{
    if( !data )
    {
        LOGW("failed to update index buffer with null.");
        return false;
    }

    if( _shadowed_data && data != _shadowed_data.get() )
        memcpy(_shadowed_data.get(), data, get_size() );

    if( _object && !_device.is_device_lost() )
    {
        _device.set_index_buffer(_object);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, get_size(), data, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }

    CHECK_GL_ERROR();
    return true;
}

bool IndexBuffer::update_data_range(const void* data, unsigned start, unsigned vcount, bool discard)
{
    if( start == 0 && vcount == _index_count )
        return update_data(data);

    if( !data )
    {
        LOGW("failed to update index buffer with null.");
        return false;
    }
    
    if( start + vcount > _index_count )
    {
        LOGW("out-of-range while setting new index buffer data.");
        return false;
    }

    if( !vcount )
        return true;

    unsigned stride = get_stride();
    if( _shadowed_data && _shadowed_data.get() + start*stride != data )
        memcpy(_shadowed_data.get() + start * stride, data, vcount * stride);

    if( _object && !_device.is_device_lost() )
    {
        _device.set_index_buffer(_object);
        if( !discard || start != 0 )
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, start * stride, vcount * stride, data);
        else
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, vcount * stride, data, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }

    CHECK_GL_ERROR();
    return true;
}

unsigned IndexBuffer::get_index_count() const
{
    return _index_count;
}

unsigned IndexBuffer::get_stride() const
{
    return GL_INDEX_ELEMENT_SIZES[value(_format)];
}

unsigned IndexBuffer::get_size() const
{
    return get_stride() * _index_count;
}

IndexElementFormat IndexBuffer::get_element_format() const
{
    return _format;
}

NS_LEMON_GRAPHICS_END