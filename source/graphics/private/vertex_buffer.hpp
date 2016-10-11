// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/graphics.hpp>
#include <graphics/private/opengl.hpp>

NS_LEMON_GRAPHICS_BEGIN

struct VertexBufferGL : public VertexBuffer
{
    VertexBufferGL(Renderer& renderer) : VertexBuffer(renderer) {}
    ~VertexBufferGL() { dispose(); }

    bool initialize(const void*, unsigned, const VertexLayout&, MemoryUsage) override;
    void dispose() override;

    //
    bool update_data(const void*) override;
    bool update_data(const void*, unsigned, unsigned, bool discard = false) override;

    //
    GLuint get_handle() const { return _object; }
    const VertexLayout& get_layout() const { return _attributes; }
    unsigned get_size() const { return _size; }

protected:
    unsigned _size;
    GLuint _object;
    GLenum _usage;
    VertexLayout _attributes;
};

NS_LEMON_GRAPHICS_END
