// @date 2016/07/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/backend.hpp>

#include <iostream>

NS_LEMON_GRAPHICS_BEGIN

// 
enum class AttributeComponentFormat : uint8_t
{
    BYTE = 0,
    UNSIGNED_BYTE,
    SHORT,
    UNSIGNED_SHORT,
    FIXED,
    FLOAT
};

// vertex attribute enumeration, correspons to vertex shader attribute
enum class VertexAttribute : uint8_t
{
    POSITION = 0,
    NORMAL,
    TANGENT,
    BITANGENT,
    COLOR_0,
    COLOR_1,
    INDICES,
    WEIGHT,
    TEXCOORD_0,
    TEXCOORD_1,
    TEXCOORD_3,
    TEXCOORD_4,
    TEXCOORD_5,
    TEXCOORD_6,
    TEXCOORD_7,
    TEXCOORD_8
};

constexpr const static size_t kVertexAttributeCount = 16;

//
struct VertexAttributeData
{
    VertexAttributeData() {}
    VertexAttributeData(VertexAttribute attribute, AttributeComponentFormat component, unsigned num, bool normalize = false)
    : attribute(attribute), component(component), num(num), normalize(normalize)
    {}

    VertexAttribute attribute = VertexAttribute::POSITION;
    // specifies the data type of each component in the array
    AttributeComponentFormat component = AttributeComponentFormat::FLOAT;
    // specifies the number of components per generic vertex attribute. must be 1, 2, 3, or 4
    unsigned num = 4;
    // specifies whether fixed-point data values should be normalized
    bool normalize = false;
};

//
struct VertexIgnoreBytes
{
    VertexIgnoreBytes(unsigned value) : value(value) {}
    unsigned value;
};

//
struct VertexAttributeLayout
{
    VertexAttributeLayout();

    // append attribute to this layout
    VertexAttributeLayout& append(VertexAttributeData&);
    VertexAttributeLayout& append(VertexIgnoreBytes&);
    VertexAttributeLayout& end();

    // returns true if this layout contains specified attribute
    bool has(VertexAttribute a) const { return _attributes[value(a)] != invalid; }
    // returns relative attribute offset from the vertex
    uint16_t get_offset(VertexAttribute a) const { return _offsets[value(a)]; }
    // returns vertex stride
    uint16_t get_stride() const { return _stride; }
    // returns the hash value of this layout
    uint32_t get_hash() const { return _hash; }
    // returns decoded attribute
    VertexAttributeData get_attribute(VertexAttribute) const;

protected:
    const static uint8_t invalid;

    uint32_t _hash;
    uint16_t _stride;
    uint16_t _offsets[kVertexAttributeCount];
    uint8_t _attributes[kVertexAttributeCount];
};

std::ostream& operator << (std::ostream&, VertexAttributeLayout&);

const char* get_attribute_name(VertexAttribute);

/* facilities for vertex attribute layout generation.
usage: make_attribute_layout(
    {VertexAttribute::POSITION, AttributeComponentFormat::FLOAT, 4},
    { 2 },
    {VertexAttribute::TEXCOORD_0, AttributeComponentFormat::FLOAT, 2} );
*/
template<typename ... T> VertexAttributeLayout make_attribute_layout(T&& ...);

//
template<typename ... T>
VertexAttributeLayout make_attribute_layout(T&& ... args)
{
    VertexAttributeLayout result;
    return make_attribute_layout(result, std::forward<T>(args)...).end();
}

template<typename T>
VertexAttributeLayout& make_attribute_layout(VertexAttributeLayout& vas, T&& arg)
{
    return vas.append(arg);
}

template<typename T1, typename T2, typename ... Args>
VertexAttributeLayout& make_attribute_layout(VertexAttributeLayout& vas, T1&& arg1, T2&& arg2, Args&& ... tails)
{
    return make_attribute_layout(vas.append(arg1), arg2, tails...);
}

NS_LEMON_GRAPHICS_END
