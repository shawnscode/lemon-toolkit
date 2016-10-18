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
    bool save(std::ostream&) override { return true;}

    graphics::PrimitiveType get_type() const;
    unsigned get_vertex_count() const;
    Handle get_vertex_handle() const;
    Handle get_index_handle() const;

protected:
    graphics::PrimitiveType _type;
    graphics::VertexBuffer* _vertex_buffer = nullptr;
    graphics::IndexBuffer* _index_buffer = nullptr;
};

INLINE graphics::PrimitiveType Primitive::get_type() const
{
    return _type;
}

INLINE Handle Primitive::get_index_handle() const
{
    return _index_buffer == nullptr ? Handle() : _index_buffer->handle;
}

INLINE Handle Primitive::get_vertex_handle() const
{
    return _vertex_buffer == nullptr ? Handle() : _vertex_buffer->handle;
}

INLINE unsigned Primitive::get_vertex_count() const
{
    return _vertex_buffer == nullptr ? 0 : _vertex_buffer->get_size();
}


NS_LEMON_RESOURCE_END
