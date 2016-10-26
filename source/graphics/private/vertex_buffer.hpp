// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/graphics.hpp>
#include <graphics/private/opengl.hpp>

NS_LEMON_GRAPHICS_BEGIN

struct VertexBufferGL : public VertexBuffer
{
    VertexBufferGL(Handle handle) : VertexBuffer(handle) {}
    virtual ~VertexBufferGL() { dispose(); }

    // initialize the OpenGL specific functionality for this buffer
    bool initialize(const void*, size_t, const VertexLayout&, MemoryUsage) override;
    // creates a new data store boound to the buffer, any pre-existing data is deleted
    bool update_data(const void*) override;
    // update a subset of a buffer object's data store, optionally discard pre-existing data
    bool update_data(const void*, size_t, size_t, bool discard = false) override;

    // release internal video resources
    void dispose();
    // returns size of vertices
    size_t get_size() const { return _size; }
    // returns vertex layout of this buffer
    const VertexLayout& get_layout() const { return _attributes; }
    // retrieves a unique id for this buffer
    GLuint get_uid() const { return _object; }

protected:
    size_t _size;
    GLuint _object;
    GLenum _usage;
    VertexLayout _attributes;
};

NS_LEMON_GRAPHICS_END
