// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <resource/resource.hpp>
#include <codebase/handle.hpp>
#include <graphics/graphics.hpp>

NS_LEMON_RESOURCE_BEGIN

struct Primitive : public Resource
{
    using ptr = std::shared_ptr<Primitive>;
    using weak_ptr = std::weak_ptr<Primitive>;

    static Primitive::ptr cube();

public:
    virtual ~Primitive();

    bool read(std::istream&) override { return true; }
    bool save(std::ostream&) override { return true; }
    bool update_video_object() override;

    size_t get_memory_usage() const override;
    size_t get_video_memory_usage() const override;

    // set the vertex data and layout of this primitive
    bool initialize(const void*, const graphics::VertexLayout&, size_t);
    bool initialize(const void*, const graphics::VertexLayout&, size_t, const void*, graphics::IndexElementFormat, size_t);
    // specifies what kind of primitives to draw
    void set_primitive_type(graphics::PrimitiveType);
    // specifies the expected usage pattern of the data source
    void set_video_memory_hint(graphics::MemoryUsage);

    // returns current primitive type
    graphics::PrimitiveType get_primitive_type() const;
    // returns current memory usage
    graphics::MemoryUsage get_video_memory_hint() const;

    // returns the size of vertices
    size_t get_vertex_size() const;
    // returns the size of indices
    size_t get_index_size() const;

    // returns graphics object of vertex/index buffer
    graphics::VertexBuffer* get_vertex_buffer() const;
    graphics::IndexBuffer* get_index_buffer() const;

protected:
    size_t _vertex_size = 0;
    size_t _index_size = 0;
    graphics::VertexLayout _layout;
    std::unique_ptr<uint8_t[]> _vertices;
    graphics::IndexElementFormat _index_format;
    std::unique_ptr<uint8_t[]> _indices;
    graphics::MemoryUsage _usage = graphics::MemoryUsage::STATIC;
    graphics::PrimitiveType _type = graphics::PrimitiveType::TRIANGLES;

    graphics::VertexBuffer* _vertex_buffer = nullptr;
    graphics::IndexBuffer* _index_buffer = nullptr;
};

INLINE void Primitive::set_primitive_type(graphics::PrimitiveType type)
{
    _type = type;
}

INLINE void Primitive::set_video_memory_hint(graphics::MemoryUsage usage)
{
    _usage = usage;
}

INLINE graphics::PrimitiveType Primitive::get_primitive_type() const
{
    return _type;
}

INLINE graphics::MemoryUsage Primitive::get_video_memory_hint() const
{
    return _usage;
}

INLINE size_t Primitive::get_vertex_size() const
{
    return _vertex_size;
}

INLINE size_t Primitive::get_index_size() const
{
    return _index_size;
}

INLINE graphics::VertexBuffer* Primitive::get_vertex_buffer() const
{
    return _vertex_buffer;
}

INLINE graphics::IndexBuffer* Primitive::get_index_buffer() const
{
    return _index_buffer;
}

NS_LEMON_RESOURCE_END
