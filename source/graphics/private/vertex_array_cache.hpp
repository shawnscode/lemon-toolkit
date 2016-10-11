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

    void free(ProgramGL&);
    void free(VertexBufferGL&);

protected:
    bool _vao_support;
    std::unordered_map<uint64_t, GLuint> _vaos;
};

NS_LEMON_GRAPHICS_END
