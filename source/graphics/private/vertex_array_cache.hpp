// @date 2016/10/09
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <codebase/handle.hpp>
#include <math/string_hash.hpp>
#include <graphics/private/opengl.hpp>

#include <unordered_map>

NS_LEMON_GRAPHICS_BEGIN

struct Program;
struct VertexBuffer;
struct VertexArrayObjectCache
{
    VertexArrayObjectCache();

    void bind(Handle, Program&, Handle, VertexBuffer&);
    void unbind();

    void free_vertex_buffer(Handle);
    void free_program(Handle);

protected:
    bool _vao_support;
    std::unordered_map<uint64_t, GLuint> _vaos;
};

NS_LEMON_GRAPHICS_END
