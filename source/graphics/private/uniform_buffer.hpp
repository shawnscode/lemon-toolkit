// @date 2016/10/08
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/renderer.hpp>
#include <graphics/private/opengl.hpp>

#include <string>
#include <unordered_map>

NS_LEMON_GRAPHICS_BEGIN

struct UniformBuffer
{
    using uniform_t = std::pair<const char*, const uint8_t*>;

    struct iterator
    {

    };

    void clear();

    // set uniform vector value
    // void set_uniform1f(const char*, const math::Vector<1, float>&);
    // void set_uniform2f(const char*, const math::Vector<2, float>&);
    // void set_uniform3f(const char*, const math::Vector<3, float>&);
    // void set_uniform4f(const char*, const math::Vector<4, float>&);
    // // set uniform matrix value
    // void set_uniform2fm(const char*, const math::Matrix<2, 2, float>&);
    // void set_uniform3fm(const char*, const math::Matrix<3, 3, float>&);
    // void set_uniform4fm(const char*, const math::Matrix<4, 4, float>&);

protected:
    std::vector<uint8_t> _buffer;
};

NS_LEMON_GRAPHICS_END
