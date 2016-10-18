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
    ProgramGL(Renderer& renderer, Handle handle) : Program(renderer, handle) {}
    virtual ~ProgramGL() { dispose(); }

    bool initialize(const char* vs, const char* ps) override;
    void dispose() override;

    // set attribute name
    bool set_attribute_name(VertexAttribute::Enum, const char*) override;
    // returns opengl handle of this program
    GLuint get_handle() const { return _object; }
    // returns location of specified uniform
    GLint get_uniform_location(const char*);
    // returns location of specified attribute
    GLint get_attribute_location(VertexAttribute::Enum);

protected:
    // returns location of specified attribute
    GLint get_attribute_location(const char*);

    std::string _fragment_shader;
    std::string _vertex_shader;
    GLuint _object = 0;

    GLint _attributes[VertexAttribute::kVertexAttributeCount];
    std::unordered_map<math::StringHash, GLint> _uniforms;
};

NS_LEMON_GRAPHICS_END
