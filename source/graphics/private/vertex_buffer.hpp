// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/graphics.hpp>
#include <graphics/private/opengl.hpp>

NS_LEMON_GRAPHICS_BEGIN

struct VertexBufferGL : public VertexBuffer
{
    VertexBufferGL(Renderer& renderer, Handle handle) : VertexBuffer(renderer, handle) {}
    virtual ~VertexBufferGL() { dispose(); }

    // initialize the OpenGL specific functionality for this buffer
    bool initialize(const void*, unsigned, const VertexLayout&, MemoryUsage) override;
    // release graphics resource and state
    void dispose() override;
    // update the whole buffer with data
    bool update_data(const void*) override;
    // update the buffer in range with data, optionally discard old buffer
    bool update_data(const void*, unsigned, unsigned, bool discard = false) override;
    // returns size of vertices
    unsigned get_size() const override { return _size; }
    // returns vertex layout of this buffer
    const VertexLayout& get_layout() const override { return _attributes; }
    // retrieves a unique id for this vertex buffer
    GLuint get_handle() const { return _object; }

protected:
    unsigned _size;
    GLuint _object;
    GLenum _usage;
    VertexLayout _attributes;
};

NS_LEMON_GRAPHICS_END
