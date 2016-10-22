// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <resource/shader.hpp>
#include <resource/image.hpp>

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
    bool set_uniform_texture(const char*, Image::ptr);

    // returns handle of specified graphics object
    graphics::UniformBuffer* get_uniform_buffer() const;
    graphics::Program* get_program() const;

protected:
    graphics::UniformBuffer* _uniform = nullptr;
    Shader::ptr _shader;
    std::unordered_map<math::StringHash, Image::ptr> _references;
};

INLINE bool Material::set_uniform_texture(const char* name, Image::ptr image)
{
    if( _uniform != nullptr )
    {
        _references[name] = image;
        return _uniform->set_uniform_texture(name, *image->get_texture());
    }
    return false;
}

INLINE graphics::UniformBuffer* Material::get_uniform_buffer() const
{
    return _uniform;
}

INLINE graphics::Program* Material::get_program() const
{
    return _shader->get_program();
}

NS_LEMON_RESOURCE_END
