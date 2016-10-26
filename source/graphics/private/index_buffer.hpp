// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/graphics.hpp>
#include <graphics/private/opengl.hpp>

NS_LEMON_GRAPHICS_BEGIN

struct IndexBufferGL : public IndexBuffer
{
    IndexBufferGL(Handle handle) : IndexBuffer(handle) {}
    virtual ~IndexBufferGL() { dispose(); }

    // initialize the OpenGL specific functionality for this buffer
    bool initialize(const void*, size_t, IndexElementFormat, MemoryUsage) override;
    // creates a new data store boound to the buffer, any pre-existing data is deleted
    bool update_data(const void*) override;
    // update a subset of a buffer object's data store, optionally discard pre-existing data
    bool update_data(const void*, size_t, size_t, bool discard = false) override;

    // release internal video resources
    void dispose();
    // retrieves the element format of this buffer, must be one of GL_UNSIGNED_SHORT or GL_UNSIGNED_BYTE
    IndexElementFormat get_format() const { return _format; }
    // retrieves a unique id for this vertex buffer
    size_t get_size() const { return _size; }
    // retrieves a unique id for this buffer
    GLuint get_uid() const { return _object; }

protected:
    size_t _size;
    GLuint _object;
    GLenum _usage;
    IndexElementFormat _format;
};

NS_LEMON_GRAPHICS_END
