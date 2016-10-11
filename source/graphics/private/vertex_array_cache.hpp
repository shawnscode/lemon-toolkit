// @date 2016/10/09
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/graphics.hpp>
#include <graphics/private/opengl.hpp>
#include <math/string_hash.hpp>

#include <unordered_map>

NS_LEMON_GRAPHICS_BEGIN

struct VertexArrayObjectCache
{
    VertexArrayObjectCache();

    void bind(Program::ptr, VertexBuffer::ptr);
    void unbind();

    // void free(Program::ptr);
    // void free(VertexBuffer::ptr);

protected:
    GLint get_attribute_location(GLuint, const char*);

    bool _vao_support;

    using attributes = std::unordered_map<math::StringHash, GLint>;
    std::unordered_map<GLuint, attributes> _program_attributes;
    std::unordered_map<uint64_t, GLuint> _vaos;
};

NS_LEMON_GRAPHICS_END
