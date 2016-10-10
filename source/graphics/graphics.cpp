// @date 2016/08/29
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/graphics.hpp>
#include <codebase/type/enumeration.hpp>

NS_LEMON_GRAPHICS_BEGIN

static const uint8_t SIZE_OF_VERTEX_ELEMENT_FORMAT[] =
{
    1,
    1,
    2,
    2,
    2,
    4
};

static const char* ATTRIBUTE_DEFAULT_NAME[] =
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
};

VertexAttribute::VertexAttribute(
    VertexAttribute::Enum attribute,
    VertexElementFormat format,
    unsigned size,
    bool normalized) : attribute(attribute), format(format), size(size), normalized(normalized) {}

uint8_t VertexAttribute::encode(const VertexAttribute& in)
{
    ASSERT( in.size >= 1 && in.size <= 4,
        "the number of components per generic vertex attribute, must be 1, 2, 3, or 4." );

    const uint8_t format = (value(in.format) & 0xF) << 0;
    const uint8_t size = (in.size & 0x7) << 4;
    const uint8_t normalized = (in.normalized ? 1 : 0) << 7;

    return format | size | normalized;
}

VertexAttribute VertexAttribute::decode(VertexAttribute::Enum va, uint8_t in)
{
    return {
        va,
        static_cast<VertexElementFormat>(in & 0xF),
        static_cast<unsigned>((in >> 4) & 0x7),
        static_cast<bool>((in >> 7) & 1)
    };
}

const char* VertexAttribute::name(VertexAttribute::Enum va)
{
    return ATTRIBUTE_DEFAULT_NAME[value(va)];
}

VertexLayout::VertexLayout() : _stride(0)
{
    memset(_offsets, 0, sizeof(_offsets));
    memset(_attributes, invalid, sizeof(_attributes));
}

VertexAttribute VertexLayout::get_attribute(VertexAttribute::Enum va) const
{
    return VertexAttribute::decode(va, _attributes[va]);
}

VertexLayout& VertexLayout::append(const VertexAttribute& va)
{
    const size_t size = SIZE_OF_VERTEX_ELEMENT_FORMAT[value(va.format)] * va.size;
    ASSERT(((size_t)_stride + size) < (size_t)invalid,
        "failed to make layout with a max stride %d.", invalid);

    _attributes[va.attribute] = VertexAttribute::encode(va);
    _offsets[va.attribute] = _stride;
    _stride += (uint8_t)size;
    return *this;
}

VertexLayout& VertexLayout::append(size_t size)
{
    ASSERT(((size_t)_stride + size) < (size_t)invalid,
        "failed to make layout with a max stride %d.", invalid);

    _stride += (uint8_t)size;
    return *this;
}

NS_LEMON_GRAPHICS_END