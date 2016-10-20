// @date 2016/10/10
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <math/vector.hpp>
#include <math/matrix.hpp>

#include <codebase/handle.hpp>
#include <codebase/type/enumeration.hpp>

#include <limits>
#include <memory>

NS_LEMON_GRAPHICS_BEGIN

#define ENSURE_NOT_RENDER_PHASE \
    ASSERT(!_renderer.is_frame_began(), "can not manipulate resource during frame render phase.");

#define DECLARE_GRAPHICS_OBJECT(T) \
    using ptr = std::shared_ptr<T>; \
    using weak_ptr = std::weak_ptr<T>; \
    T(Renderer& renderer, Handle handle) : GraphicsObject(renderer, handle) {} \
    virtual ~T() {}

// GraphicsObject is the base class of all the graphic card resources
struct GraphicsObject
{
    GraphicsObject(Renderer& renderer, Handle handle) : _renderer(renderer), handle(handle) {}
    virtual ~GraphicsObject() {}

    // returns handle to this graphics object
    const Handle handle;

protected:
    Renderer& _renderer;
};

// the expected usage pattern of the data store
enum class MemoryUsage : uint8_t
{
    // the data store contents will be modified once and used at most a few times
    STREAM = 0,
    // the data store contents will be modified once and used many times
    STATIC,
    // the data store contents will be modified repeatedly and used many times
    DYNAMIC
};

// clear options
enum class ClearOption : uint8_t
{
    NONE    = 0x0,
    COLOR   = 0x1,
    DEPTH   = 0x2,
    STENCIL = 0x4
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

// specifies what kind of primitives to render.
enum class PrimitiveType : uint8_t
{
    POINTS = 0,
    LINES,
    LINE_LOOP,
    LINE_STRIP,
    TRIANGLES,
    TRIANGLE_STRIP,
    TRIANGLE_FAN,
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

struct VertexBuffer : public GraphicsObject
{
    DECLARE_GRAPHICS_OBJECT(VertexBuffer);

    virtual bool initialize(const void*, unsigned, const VertexLayout&, MemoryUsage) = 0;
    virtual void dispose() = 0;

    virtual bool update_data(const void*) = 0;
    virtual bool update_data(const void*, unsigned, unsigned, bool discard = false) = 0;

    virtual unsigned get_size() const = 0;
    virtual const VertexLayout& get_layout() const = 0;
};

// index element format
enum class IndexElementFormat : uint8_t
{
    UBYTE = 0,
    USHORT
};

struct IndexBuffer : GraphicsObject
{
    DECLARE_GRAPHICS_OBJECT(IndexBuffer);

    virtual bool initialize(const void*, unsigned, IndexElementFormat, MemoryUsage) = 0;
    virtual void dispose() = 0;

    virtual bool update_data(const void*) = 0;
    virtual bool update_data(const void*, unsigned, unsigned, bool discard = false) = 0;
};

//
enum class TextureFilterMode : uint8_t
{
    NEAREST = 0,
    LINEAR,
    TRILINEAR,
    ANISOTROPIC
};

enum class TextureAddressMode : uint8_t
{
    WRAP = 0,
    MIRROR,
    CLAMP,
    BORDER
};

enum class TextureCoordinate : uint8_t
{
    U = 0,
    V,
    W
};

enum class TextureFormat : uint8_t
{
    ALPHA = 0,
    RGB,
    RGBA,
    LUMINANCE,
    LUMINANCE_ALPHA
};

enum class TexturePixelFormat : uint8_t
{
    UBYTE = 0,
    USHORT565,
    USHORT4444,
    USHORT5551
};

struct Texture : public GraphicsObject
{
    DECLARE_GRAPHICS_OBJECT(Texture);

    virtual bool initialize(const void*, TextureFormat, TexturePixelFormat, unsigned, unsigned, MemoryUsage) = 0;
    virtual void dispose() = 0;

    // set number of requested mip levels
    virtual void set_mipmap(bool) = 0;
    // set filtering mode
    virtual void set_filter_mode(TextureFilterMode) = 0;
    // set adddress mode
    virtual void set_address_mode(TextureCoordinate, TextureAddressMode) = 0;
    // update the changed parameters to device
    virtual void update_parameters(bool force = false) = 0;
};

//
struct UniformBuffer : public GraphicsObject
{
    DECLARE_GRAPHICS_OBJECT(UniformBuffer);

    virtual bool initialize() = 0;
    virtual void dispose() = 0;

    // set uniform vector value
    virtual bool set_uniform_1f(const char*, const math::Vector<1, float>&) = 0;
    virtual bool set_uniform_2f(const char*, const math::Vector<2, float>&) = 0;
    virtual bool set_uniform_3f(const char*, const math::Vector<3, float>&) = 0;
    virtual bool set_uniform_4f(const char*, const math::Vector<4, float>&) = 0;
    // set uniform matrix value
    virtual bool set_uniform_2fm(const char*, const math::Matrix<2, 2, float>&) = 0;
    virtual bool set_uniform_3fm(const char*, const math::Matrix<3, 3, float>&) = 0;
    virtual bool set_uniform_4fm(const char*, const math::Matrix<4, 4, float>&) = 0;
    // set uniform texture
    virtual bool set_uniform_texture(const char*, Handle) = 0;
};

//
struct Program : public GraphicsObject
{
    DECLARE_GRAPHICS_OBJECT(Program);

    virtual bool initialize(const char* vs, const char* ps) = 0;
    virtual void dispose() = 0;

    // specified input identifier of vertex attribute
    virtual bool set_attribute_name(VertexAttribute::Enum, const char*) = 0;
    // returns true if we have uniform associated with name in program
    virtual bool has_uniform(const char*) const = 0;
};

// INCLUDED IMPLEMENTATIONS
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
