// @date 2016/10/10
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>

#include <math/vector.hpp>
#include <math/matrix.hpp>

#include <codebase/handle.hpp>
#include <codebase/enumeration.hpp>

NS_LEMON_GRAPHICS_BEGIN

// BufferUsage is a hint to the video implementation as to how a buffer object
// store will be accessed, this enable video implementation to make more
// intelligent decisions that may significantly impact buffer object performance
enum class BufferUsage : uint8_t
{
    // the data store contents will be modified once and used at most a few times
    STREAM = 0,
    // the data store contents will be modified once and used many times
    STATIC,
    // the data store contents will be modified repeatedly and used many times
    DYNAMIC
};

// vertex element format
enum class VertexElementFormat : uint8_t
{
    BYTE = 0,
    UBYTE,
    SHORT,
    USHORT,
    FIXED,
    FLOAT
};

// the type of values in indices
enum class IndexElementFormat : uint8_t
{
    UBYTE = 0,
    USHORT
};

// VertexAttribute defines an generic vertex attribute data
struct VertexAttribute
{
    enum Enum : uint8_t
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
    };

    constexpr const static size_t kVertexAttributeCount = 12;

    VertexAttribute() {}
    VertexAttribute(VertexAttribute::Enum, VertexElementFormat, unsigned size = 4, bool normalized = false);

    // specifies the default attribute identifier
    VertexAttribute::Enum attribute = VertexAttribute::POSITION;
    // specifies the data type of each component in the array
    VertexElementFormat format = VertexElementFormat::FLOAT;
    // specifies the number of components per generic vertex attribute. must be 1, 2, 3, or 4
    unsigned size = 4;
    // specifies whether fixed-point data values should be normalized
    bool normalized = false;

    static uint8_t encode(const VertexAttribute&);
    static VertexAttribute decode(VertexAttribute::Enum, uint8_t);
    static const char* name(VertexAttribute::Enum);
};

// VertexLayout defines an layout of vertex structure
struct VertexLayout
{
    template<typename ... T> static VertexLayout make(T&& ...);

    VertexLayout();

    // returns true if this layout contains specified attribute
    bool has(VertexAttribute::Enum va) const { return _attributes[va] != invalid; }
    // returns relative attribute offset from the vertex
    uint8_t get_offset(VertexAttribute::Enum va) const { return _offsets[va]; }
    // returns vertex stride
    uint8_t get_stride() const { return _stride; }
    // returns decoded attribute
    VertexAttribute get_attribute(VertexAttribute::Enum va) const;

protected:
    template<typename T> static VertexLayout& recursive_make(VertexLayout&, T&&);
    template<typename T1, typename T2, typename ... Args>
    static VertexLayout& recursive_make(VertexLayout&, T1&&, T2&&, Args&& ...);

    constexpr const static uint8_t invalid = std::numeric_limits<uint8_t>::max();

    VertexLayout& append(size_t size);
    VertexLayout& append(const VertexAttribute& va);

    uint8_t _stride;
    uint8_t _offsets[VertexAttribute::kVertexAttributeCount];
    uint8_t _attributes[VertexAttribute::kVertexAttributeCount];
};

struct VertexBuffer
{
    // initialize the video specific functionality for this buffer
    virtual bool initialize(const void*, size_t, const VertexLayout&, MemoryUsage) = 0;
    // destroy vertex buffer
    virtual void dispose() = 0;
    // creates a new data store boound to the buffer, any pre-existing data is deleted
    virtual bool update_data(const void*) = 0;
    // update a subset of a buffer object's data store, optionally discard pre-existing data
    virtual bool update_data(const void*, size_t, size_t, bool discard = false) = 0;
};

struct IndexBuffer
{
    // initialize the video specific functionality for this buffer
    virtual bool initialize(const void*, size_t, IndexElementFormat, MemoryUsage) = 0;
    // destroy index buffer
    virtual void dispose() = 0;
    // creates a new data store boound to the buffer, any pre-existing data is deleted
    virtual bool update_data(const void*) = 0;
    // update a subset of a buffer object's data store, optionally discard pre-existing data
    virtual bool update_data(const void*, size_t, size_t, bool discard = false) = 0;
};

// INCLUDED IMPLEMENTATIONS of VERTEX LAYOUT
template<>
INLINE VertexLayout VertexLayout::make()
{
    return VertexLayout();
}

template<typename ... T>
INLINE VertexLayout VertexLayout::make(T&& ... args)
{
    VertexLayout layout;
    return recursive_make(layout, std::forward<T>(args)...);
}

template<typename T>
INLINE VertexLayout& VertexLayout::recursive_make(VertexLayout& layout, T&& arg)
{
    return layout.append(arg);
}

template<typename T1, typename T2, typename ... Args>
INLINE VertexLayout& VertexLayout::recursive_make(VertexLayout& layout, T1&& t1, T2&& t2, Args&& ... args)
{
    return recursive_make(layout.append(t1), t2, std::forward<Args>(args)...);
}

NS_LEMON_GRAPHICS_END
