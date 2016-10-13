// @date 2016/10/09
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/graphics.hpp>
#include <graphics/private/opengl.hpp>
#include <math/string_hash.hpp>

#include <unordered_map>

NS_LEMON_GRAPHICS_BEGIN

struct RenderStateCache
{
    RenderStateCache(Renderer&);

    void begin_frame();
    void bind_program(Handle program);
    void bind_uniform_buffer(Handle program, Handle uniform);
    void bind_vertex_buffer(Handle program, Handle vb);
    void end_frame();

    void free_program(Handle);
    void free_vertex_buffer(Handle);

protected:
    Renderer& _renderer;

    bool _vao_support;
    std::unordered_map<std::pair<Handle, Handle>, GLuint> _vertex_array_objects;

    Handle _active_program;
    std::unordered_map<Handle, Handle> _active_uniforms;
};

NS_LEMON_GRAPHICS_END
