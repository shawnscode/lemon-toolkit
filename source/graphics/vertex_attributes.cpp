#include <graphics/vertex_attributes.hpp>
#include <graphics/opengl.hpp>

NS_LEMON_GRAPHICS_BEGIN

static const uint8_t SIZE_OF_COMPONENT_FORMAT[] =
{
    1,
    1,
    2,
    2,
    2,
    4
};

static const char* s_attribute_names[] =
{
    "Position",
    "Normal",
    "Tangent",
    "Bitangent",
    "Color_0",
    "Color_1",
    "Indices",
    "Weight",
    "Texcoord_0",
    "Texcoord_1",
    "Texcoord_3",
    "Texcoord_4",
    "Texcoord_5",
    "Texcoord_6",
    "Texcoord_7",
    "Texcoord_8"
};

const uint8_t VertexAttributeLayout::invalid = 0xff;

VertexAttributeLayout::VertexAttributeLayout()
{
    _stride = 0;
    memset(_attributes, 0xff, sizeof(_attributes));
    memset(_offsets, 0, sizeof(_offsets));
}

VertexAttributeLayout& VertexAttributeLayout::append(VertexAttributeData& data)
{
    ASSERT( data.num >= 1 && data.num <= 4,
        "the number of components per generic vertex attribute, must be 1, 2, 3, or 4." );

    const uint8_t type = (value(data.component) & 0xF) << 0;
    const uint8_t number = (data.num & 0x7) << 4;
    const uint8_t normalize = (data.normalize ? 1 : 0) << 7;

    _attributes[value(data.attribute)] = type | number | normalize;
    _offsets[value(data.attribute)] = _stride;
    _stride += SIZE_OF_COMPONENT_FORMAT[value(data.component)] * data.num;
    return *this;
}

VertexAttributeLayout& VertexAttributeLayout::append(VertexIgnoreBytes& data)
{
    _stride += data.value;
    return *this;
}

VertexAttributeLayout& VertexAttributeLayout::end()
{
    // todo: calculation hash value
    return *this;
}

VertexAttributeData VertexAttributeLayout::get_attribute(VertexAttribute index) const
{
    const uint8_t encode = _attributes[value(index)];
    return {
        index,
        static_cast<AttributeComponentFormat>(encode & 0xF),
        static_cast<unsigned>((encode >> 4) & 0x7),
        static_cast<bool>((encode >> 7) & 1)
    };
}

std::ostream& operator << (std::ostream& out, VertexAttributeLayout& attributes)
{
    out << "VertexAttributeLayout(";
    for( uint8_t i = 0; i < kVertexAttributeCount; i++ )
    {
        if( attributes.has((VertexAttribute)i) )
            out << get_attribute_name((VertexAttribute)i) << ", ";
    }
    return out << ")";
}

const char * get_attribute_name(VertexAttribute va)
{
    return s_attribute_names[value(va)];
}

// VertexBuffer::VertexBuffer(Backend& device)
// : GraphicsObject(device)
// {
//     _dynamic    = false;
//     _shadowed   = false;
//     _vertex_count       = 0;
// }

// bool VertexBuffer::restore(const void* data,
//     unsigned vcount, const vertex_attributes& attributes, bool dynamic)
// {
//     if( _device.is_device_lost() )
//     {
//         LOGW("trying to restore vertex buffer while device is lost.");
//         return false;
//     }

//     release();

//     glGenBuffers(1, &_object);
//     if( !_object )
//     {
//         LOGW("failed to create vertex buffer object.");
//         return false;
//     }

//     _attributes     = attributes;
//     _vertex_count   = vcount;
//     _dynamic        = dynamic;

//     unsigned offset = 0;
//     for( auto& attr : _attributes )
//     {
//         attr.offset = offset;
//         offset += attr.get_stride();
//     }

//     if( get_stride() == 0 || vcount == 0 )
//     {
//         _shadowed_data.reset();
//         return true;
//     }

//     if( _shadowed )
//     {
//         _shadowed_data.reset( new uint8_t[get_size()] );
//         if( !_shadowed_data )
//         {
//             LOGW("failed to create shadowed data for vertex buffer.");
//             return false;
//         }
//     }

//     CHECK_GL_ERROR();
//     return update_data(data);
// }

// bool VertexBuffer::restore()
// {
//     if( !_shadowed_data )
//     {
//         LOGW("failed to restore vertex buffer without shadowed data.");
//         return false;
//     }

//     return restore(_shadowed_data.get(), _vertex_count, _attributes, _dynamic);
// }

// void VertexBuffer::release()
// {
//     if( _object != 0 && !_device.is_device_lost() )
//     {
//         if( _device.get_bound_vertex_buffer() == _object )
//             _device.set_vertex_buffer(0);
//         glDeleteBuffers(1, &_object);
//     }

//     _object = 0;
// }

// void VertexBuffer::set_shadowed(bool shadowed)
// {
//     if( _shadowed != shadowed )
//     {
//         if( shadowed && get_size() > 0 )
//         {
//             _shadowed_data.reset( new uint8_t[get_size()] );
//             if( !_shadowed_data )
//                 LOGW("failed to create shadowed data for vertex buffer.");
//         }
//         else
//             _shadowed_data.reset();
//     }

//     _shadowed = shadowed;
// }

// bool VertexBuffer::update_data(const void* data)
// {
//     if( !data )
//     {
//         LOGW("failed to update vertex buffer with null.");
//         return false;
//     }

//     if( _shadowed_data && data != _shadowed_data.get() )
//         memcpy(_shadowed_data.get(), data, get_size() );

//     if( _object && !_device.is_device_lost() )
//     {
//         _device.set_vertex_buffer(_object);
//         glBufferData(GL_ARRAY_BUFFER, get_size(), data, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
//     }

//     CHECK_GL_ERROR();
//     return true;
// }

// bool VertexBuffer::update_data_range(const void* data, unsigned start, unsigned vcount, bool discard)
// {
//     if( start == 0 && vcount == _vertex_count )
//         return update_data(data);

//     if( !data )
//     {
//         LOGW("failed to update vertex buffer with null.");
//         return false;
//     }
    
//     if( start + vcount > _vertex_count )
//     {
//         LOGW("out-of-range while setting new vertex buffer data.");
//         return false;
//     }

//     if( !vcount )
//         return true;

//     unsigned stride = get_stride();
//     if( _shadowed_data && _shadowed_data.get() + start*stride != data )
//         memcpy(_shadowed_data.get() + start * stride, data, vcount * stride);

//     if( _object && !_device.is_device_lost() )
//     {
//         _device.set_vertex_buffer(_object);
//         if( !discard || start != 0 )
//             glBufferSubData(GL_ARRAY_BUFFER, start * stride, vcount * stride, data);
//         else
//             glBufferData(GL_ARRAY_BUFFER, vcount * stride, data, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
//     }

//     CHECK_GL_ERROR();
//     return true;
// }

// VertexAttribute VertexBuffer::get_attribute_at(unsigned index) const
// {
//     ASSERT( index < _attributes.get_size(), "invalid attribute.");
//     return _attributes[index];
// }

// unsigned VertexBuffer::get_attribute_size() const
// {
//     return _attributes.get_size();
// }

// unsigned VertexBuffer::get_vertex_count() const
// {
//     return _vertex_count;
// }

// unsigned VertexBuffer::get_stride() const
// {
//     unsigned stride = 0;
//     for( auto attr : _attributes )
//         stride += attr.get_stride();
//     return stride;
// }

// unsigned VertexBuffer::get_size() const
// {
//     return get_stride() * _vertex_count;
// }

NS_LEMON_GRAPHICS_END