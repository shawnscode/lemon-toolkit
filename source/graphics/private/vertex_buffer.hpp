// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/frontend.hpp>
#include <graphics/private/opengl.hpp>

NS_LEMON_GRAPHICS_BEGIN

struct VertexBuffer
{
    VertexBuffer();

    bool initialize(const void*, unsigned, const VertexAttributeLayout&, BufferUsage);
    void dispose();

    bool update_data(const void*);
    bool update_data_range(const void*, unsigned, unsigned, bool discard = false);

    GLuint get_buffer() const { return _buffer; }
    const VertexAttributeLayout& get_attributes() const { return _attributes; }
    unsigned get_size() const { return _size; }

protected:
    unsigned _size;
    GLuint _buffer;
    GLenum _usage;
    VertexAttributeLayout _attributes;
};

NS_LEMON_GRAPHICS_END
