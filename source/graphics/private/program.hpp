// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/graphics.hpp>
#include <graphics/private/opengl.hpp>
#include <graphics/private/texture.hpp>

#include <math/string_hash.hpp>
#include <math/vector.hpp>
#include <math/matrix.hpp>

#include <map>
#include <string>
#include <unordered_map>

NS_LEMON_GRAPHICS_BEGIN
struct ProgramGL : public Program
{
    ProgramGL(Handle handle) : Program(handle) {}
    virtual ~ProgramGL() { dispose(); }

    // initialize the OpenGL specific functionality for this buffer
    bool initialize(const char* vs, const char* ps) override;
    // specified input identifier of vertex attribute
    bool set_attribute_name(VertexAttribute::Enum, const char*) override;
    // returns true if we have uniform associated with name in program
    bool has_uniform(const char*) const override;

    // release internal video resources
    void dispose();
    // retrieves a unique id for this buffer
    GLuint get_uid() const { return _object; }
    // returns location of specified uniform
    GLint get_uniform_location(const char*, bool warning = true) const;
    // returns location of specified attribute
    GLint get_attribute_location(VertexAttribute::Enum);

protected:
    // returns location of specified attribute
    GLint get_attribute_location(const char*);

    std::string _fragment_shader;
    std::string _vertex_shader;
    GLuint _object = 0;

    GLint _attributes[VertexAttribute::kVertexAttributeCount];
    mutable std::unordered_map<math::StringHash, GLint> _uniforms;
};

NS_LEMON_GRAPHICS_END
