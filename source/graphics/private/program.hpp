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
    ProgramGL(Renderer& renderer) : Program(renderer) {}

    bool initialize(const char* vs, const char* ps) override;
    void dispose() override;

    // set uniform vector value
    bool set_uniform_1f(const char*, const math::Vector<1, float>&) override;
    bool set_uniform_2f(const char*, const math::Vector<2, float>&) override;
    bool set_uniform_3f(const char*, const math::Vector<3, float>&) override;
    bool set_uniform_4f(const char*, const math::Vector<4, float>&) override;
    // set uniform matrix value
    bool set_uniform_2fm(const char*, const math::Matrix<2, 2, float>&) override;
    bool set_uniform_3fm(const char*, const math::Matrix<3, 3, float>&) override;
    bool set_uniform_4fm(const char*, const math::Matrix<4, 4, float>&) override;
    // set uniform texture
    bool set_uniform_texture(const char*, std::shared_ptr<Texture>) override;

    // active this program
    void bind();
    // returns opengl handle of this program
    GLuint get_handle() const { return _object; }

protected:
    GLint get_uniform_location(const char*);

    std::string _fragment_shader;
    std::string _vertex_shader;
    GLuint _object = 0;

    // std::map<name, std::pair<location, texture handle>>
    std::map<math::StringHash, std::pair<GLint, std::shared_ptr<Texture>>> _textures;
    std::unordered_map<math::StringHash, GLint> _uniforms;
};

NS_LEMON_GRAPHICS_END
