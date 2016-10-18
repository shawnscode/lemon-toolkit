// @date 2016/10/13
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/graphics.hpp>
#include <graphics/private/opengl.hpp>
#include <codebase/variant.hpp>

#include <string>
#include <map>
#include <unordered_map>

NS_LEMON_GRAPHICS_BEGIN

using UniformVariable = Variant<
    math::Vector<1, float>,
    math::Vector<2, float>,
    math::Vector<3, float>,
    math::Vector<4, float>,
    math::Matrix<2, 2, float>,
    math::Matrix<3, 3, float>,
    math::Matrix<4, 4, float>>;

struct UniformBufferGL : public UniformBuffer
{
    UniformBufferGL(Renderer& renderer, Handle handle) : UniformBuffer(renderer, handle) {}
    virtual ~UniformBufferGL() { dispose(); }

    bool initialize() override;
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
    bool set_uniform_texture(const char*, Handle) override;

    std::map<std::string, Handle>& get_textures() { return _textures; }
    const std::map<std::string, Handle>& get_textures() const { return _textures; }

    std::unordered_map<std::string, UniformVariable>& get_uniforms() { return _uniforms; }
    const std::unordered_map<std::string, UniformVariable>& get_uniforms() const { return _uniforms; }

protected:
    std::map<std::string, Handle> _textures;
    std::unordered_map<std::string, UniformVariable> _uniforms;
};

NS_LEMON_GRAPHICS_END
