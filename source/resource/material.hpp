// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <resource/shader.hpp>
#include <resource/image.hpp>
#include <graphics/state.hpp>

#include <array>

NS_LEMON_RESOURCE_BEGIN

// materials are used in conjunction with rendering components used in lemon,
// they play an essential part in defining how your object is displayed.
struct Material : public Resource
{
    using ptr = std::shared_ptr<Material>;
    using weak_ptr = std::weak_ptr<Material>;

public:
    virtual ~Material();

    bool read(std::istream&) override;
    bool save(std::ostream&) override;
    bool update_video_object() override;

    size_t get_memory_usage() const override;
    size_t get_video_memory_usage() const override;

    // initialize material with shader
    bool initialize(Shader::ptr);
    // material will keep a reference to this image
    bool set_texture(const char*, Image::ptr);
    // set the uniform variable of program
    bool set_uniform_variable(const char*, const graphics::UniformVariable&);
    //
    bool set_render_state(const graphics::RenderState&);
    // returns internal shader
    Shader::ptr get_shader();

    // returns 
    Handle get_video_uniforms();
    Handle get_video_state();

protected:    
    template<size_t S> using uniform_array_t = 
        std::array<std::pair<math::StringHash, graphics::UniformVariable>, S>;

    template<size_t S> using texture_array_t =
        std::array<std::pair<math::StringHash, Image::ptr>, S>;

    Shader::ptr _shader;

    uint8_t _uniform_size = 0;
    uniform_array_t<graphics::kMaxUniformsPerMaterial> _uniforms;

    uint8_t _texture_size = 0;
    texture_array_t<graphics::kMaxTexturePerMaterial> _textures;

    bool _uniform_dirty = false;
    Handle _uniform_buffer;
    Handle _render_state;
};

INLINE Shader::ptr Material::get_shader()
{
    return _shader;
}

NS_LEMON_RESOURCE_END
