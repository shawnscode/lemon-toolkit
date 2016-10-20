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

    bool initialize(const void*, unsigned, const VertexLayout&, MemoryUsage) override;
    void dispose() override;

    //
    bool update_data(const void*) override;
    bool update_data(const void*, unsigned, unsigned, bool discard = false) override;
    unsigned get_size() const override { return _size; }
    const VertexLayout& get_layout() const override { return _attributes; }

    //
    GLuint get_handle() const { return _object; }

protected:
    unsigned _size;
    GLuint _object;
    GLenum _usage;
    VertexLayout _attributes;
};

NS_LEMON_GRAPHICS_END
