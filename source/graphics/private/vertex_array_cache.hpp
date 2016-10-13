// @date 2016/10/09
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/graphics.hpp>
#include <graphics/private/opengl.hpp>
#include <math/string_hash.hpp>

#include <unordered_map>

NS_LEMON_GRAPHICS_BEGIN

struct ProgramGL;
struct VertexBufferGL;

struct VertexArrayObjectCache
{
    VertexArrayObjectCache();

    void bind(ProgramGL&, VertexBufferGL&);
    void unbind();

    // void bind_uniform_buffer(Renderer&, Handle program, Handle uniform);

    void free(ProgramGL&);
    void free(VertexBufferGL&);

protected:
    bool _vao_support;
    std::unordered_map<uint64_t, GLuint> _vaos;

    // std::unordered_map<Handle, Handle> _
};

struct RenderDrawcallCache
{
    RenderDrawcallCache(Renderer&);

    void begin_frame();
    void bind_vertex_buffer(Handle program, Handle vb);
    void bind_uniform_buffer(Handle program, Handle uniform);
    void end_frame();

protected:
    Renderer& _renderer;

    std::unordered_map<Handle, Handle> _active_uniforms;
};

NS_LEMON_GRAPHICS_END
