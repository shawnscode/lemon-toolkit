// @date 2016/10/08
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/frontend.hpp>
#include <graphics/private/opengl.hpp>

#include <string>
#include <unordered_map>

NS_LEMON_GRAPHICS_BEGIN

struct UniformBuffer
{
    // struct 

    // update uniform vector value
    // void update_uniform1f(const char*, const math::Vector<1, float>&);
    // void update_uniform2f(const char*, const math::Vector<2, float>&);
    // void update_uniform3f(const char*, const math::Vector<3, float>&);
    // void update_uniform4f(const char*, const math::Vector<4, float>&);
    // // update uniform matrix value
    // void update_uniform2fm(const char*, const math::Matrix<2, 2, float>&);
    // void update_uniform3fm(const char*, const math::Matrix<3, 3, float>&);
    // void update_uniform4fm(const char*, const math::Matrix<4, 4, float>&);



protected:
    // std::unordered_map<std::string, std::unique_ptr<uint8_t[]>> _uniforms;
};

NS_LEMON_GRAPHICS_END
