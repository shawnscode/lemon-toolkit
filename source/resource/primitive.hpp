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
    bool read(std::istream&) override { return true; }
    bool save(std::ostream&) override { return true;}

    Handle get_vertex_buffer() const { return _vertex_buffer; }
    Handle get_index_buffer() const { return _index_buffer; }
    graphics::PrimitiveType get_type() const { return _type; }

protected:
    graphics::PrimitiveType _type;
    Handle _vertex_buffer;
    Handle _index_buffer;
};

NS_LEMON_RESOURCE_END
