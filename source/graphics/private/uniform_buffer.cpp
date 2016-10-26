// @date 2016/10/13
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/private/uniform_buffer.hpp>
#include <graphics/renderer.hpp>

NS_LEMON_GRAPHICS_BEGIN

bool UniformBufferGL::initialize()
{
    return true;
}

void UniformBufferGL::dispose()
{
    _textures.clear();
    _uniforms.clear();
}

bool UniformBufferGL::set_uniform_texture(const char* name, Handle texture)
{
   _textures[name] = texture;
    return true;
}

bool UniformBufferGL::set_uniform_1f(const char* name, const math::Vector<1, float>& value)
{
    _uniforms[name].set<math::Vector<1, float>>(value);
    return true;
}

bool UniformBufferGL::set_uniform_2f(const char* name, const math::Vector<2, float>& value)
{
    _uniforms[name].set<math::Vector<2, float>>(value);
    return true;
}

bool UniformBufferGL::set_uniform_3f(const char* name, const math::Vector<3, float>& value)
{
     _uniforms[name].set<math::Vector<3, float>>(value);
    return true;
}

bool UniformBufferGL::set_uniform_4f(const char* name, const math::Vector<4, float>& value)
{
    _uniforms[name].set<math::Vector<4, float>>(value);
    return true;
}

// OpenGL use column-major layout, so we always transpose our matrix
bool UniformBufferGL::set_uniform_2fm(const char* name, const math::Matrix<2, 2, float>& value)
{
    _uniforms[name].set<math::Matrix<2, 2, float>>(value);
    return true;
}

bool UniformBufferGL::set_uniform_3fm(const char* name, const math::Matrix<3, 3, float>& value)
{
    _uniforms[name].set<math::Matrix<3, 3, float>>(value);
    return true;
}

bool UniformBufferGL::set_uniform_4fm(const char* name, const math::Matrix<4, 4, float>& value)
{
    _uniforms[name].set<math::Matrix<4, 4, float>>(value);
    return true;
}

NS_LEMON_GRAPHICS_END