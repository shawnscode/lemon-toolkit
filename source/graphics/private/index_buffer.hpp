// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/frontend.hpp>
#include <graphics/private/opengl.hpp>

NS_LEMON_GRAPHICS_BEGIN

struct IndexBuffer
{
    IndexBuffer();

    bool initialize(const void*, unsigned, IndexElementFormat, BufferUsage);
    void dispose();

    bool update_data(const void*);
    bool update_data_range(const void*, unsigned, unsigned, bool discard = false);

    GLuint get_buffer() const { return _buffer; }
    IndexElementFormat get_element_format() const { return _format; }
    unsigned get_size() const { return _size; }

protected:
    unsigned _size;
    GLuint _buffer;
    GLenum _usage;
    IndexElementFormat _format;
};

NS_LEMON_GRAPHICS_END
