// @date 2016/07/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/backend.hpp>
#include <core/compact.hpp>

NS_LEMON_GRAPHICS_BEGIN

enum class ElementFormat : uint8_t
{
    BYTE = 0,
    UBYTE,
    SHORT,
    USHORT,
    FIXED,
    FLOAT,
};

struct VertexAttribute
{
    VertexAttribute() {}
    VertexAttribute(unsigned, ElementFormat, bool normalized = false);

    unsigned        size = 0;
    ElementFormat   format;
    bool            normalized;
    unsigned        offset;

    unsigned        get_stride() const;
};

struct VertexBuffer : public GraphicsObject
{
    using vertex_attributes = core::CompactVector<VertexAttribute, kMaxVertexAttributes>;
    using ptr = std::shared_ptr<VertexBuffer>;
    using weak_ptr = std::weak_ptr<VertexBuffer>;

    VertexBuffer(Backend&);
    virtual ~VertexBuffer() { release(); }

    bool restore(const void*, unsigned vcount, const vertex_attributes&, bool dynamic = false);
    bool restore() override;
    void release() override;

    // enable shadowing data in cpu memory
    void set_shadowed(bool);
    // set all data in the buffer
    bool update_data(const void*);
    // set a data range in the buffer, optionally discard data outside the range
    bool update_data_range(const void*, unsigned, unsigned, bool discard = false);

    VertexAttribute get_attribute_at(unsigned) const;
    unsigned        get_attribute_size() const;
    unsigned        get_vertex_count() const;
    unsigned        get_stride() const;
    unsigned        get_size() const;

protected:
    bool _dynamic;
    unsigned _vertex_count;
    vertex_attributes _attributes;

    // shadow data stored in system memory
    bool _shadowed;
    std::unique_ptr<uint8_t[]> _shadowed_data;
};

NS_LEMON_GRAPHICS_END
