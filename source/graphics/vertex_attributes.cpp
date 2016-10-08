#include <graphics/vertex_attributes.hpp>

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

NS_LEMON_GRAPHICS_END