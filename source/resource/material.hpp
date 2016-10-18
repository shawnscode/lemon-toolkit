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

    enum class BuildinUniform : uint8_t
    {
        PROJECTION = 0,
        VIEW,
        MODEL,

        VIEW_POS,
        LIGHT_POS,
        LIGHT_COLOR
    };

    static const char* name(BuildinUniform);

    static Material::ptr create(const fs::Path&, Shader::ptr);

public:
    virtual ~Material();

    bool read(std::istream&) override;
    bool save(std::ostream&) override;

    // specified input identifier of vertex attribute
    bool set_attribute_name(graphics::VertexAttribute::Enum, const char*);
    // returns true if we have uniform associated with name in program
    bool has_uniform(const char*) const;
    // set uniform vector valuet
    bool set_uniform_1f(const char*, const math::Vector<1, float>&);
    bool set_uniform_2f(const char*, const math::Vector<2, float>&);
    bool set_uniform_3f(const char*, const math::Vector<3, float>&);
    bool set_uniform_4f(const char*, const math::Vector<4, float>&);
    // set uniform matrix value
    bool set_uniform_2fm(const char*, const math::Matrix<2, 2, float>&);
    bool set_uniform_3fm(const char*, const math::Matrix<3, 3, float>&);
    bool set_uniform_4fm(const char*, const math::Matrix<4, 4, float>&);
    // set uniform texture
    bool set_uniform_texture(const char*, Image::ptr);

    // returns handle of specified graphics object
    Handle get_program_handle() const;
    Handle get_uniform_handle() const;

protected:
    graphics::UniformBuffer* _uniform = nullptr;
    Shader::ptr _shader;
    std::vector<Image::ptr> _images;
};

INLINE bool Material::set_attribute_name(graphics::VertexAttribute::Enum value, const char* name)
{
    return _shader == nullptr ? false : _shader->set_attribute_name(value, name);
}

INLINE bool Material::has_uniform(const char* name) const
{
    return _shader == nullptr ? false : _shader->has_uniform(name);
}

INLINE bool Material::set_uniform_1f(const char* name, const math::Vector<1, float>& value)
{
    return _uniform == nullptr ? false : _uniform->set_uniform_1f(name, value);
}

INLINE bool Material::set_uniform_2f(const char* name, const math::Vector<2, float>& value)
{
    return _uniform == nullptr ? false : _uniform->set_uniform_2f(name, value);
}

INLINE bool Material::set_uniform_3f(const char* name, const math::Vector<3, float>& value)
{
    return _uniform == nullptr ? false : _uniform->set_uniform_3f(name, value);
}

INLINE bool Material::set_uniform_4f(const char* name, const math::Vector<4, float>& value)
{
    return _uniform == nullptr ? false : _uniform->set_uniform_4f(name, value);
}

INLINE bool Material::set_uniform_2fm(const char* name, const math::Matrix<2, 2, float>& value)
{
    return _uniform == nullptr ? false : _uniform->set_uniform_2fm(name, value);
}

INLINE bool Material::set_uniform_3fm(const char* name, const math::Matrix<3, 3, float>& value)
{
    return _uniform == nullptr ? false : _uniform->set_uniform_3fm(name, value);
}

INLINE bool Material::set_uniform_4fm(const char* name, const math::Matrix<4, 4, float>& value)
{
    return _uniform == nullptr ? false : _uniform->set_uniform_4fm(name, value);
}

INLINE bool Material::set_uniform_texture(const char* name, Image::ptr image)
{
    auto found = std::find(_images.begin(), _images.end(), image);
    if( found == _images.end() )
        _images.push_back(image);

    return _uniform == nullptr ? false : _uniform->set_uniform_texture(name, image->get_texture_handle());
}

INLINE Handle Material::get_program_handle() const
{
    return _shader == nullptr ? Handle() : _shader->get_program_handle();
}

INLINE Handle Material::get_uniform_handle() const
{
    return _uniform == nullptr ? Handle() : _uniform->handle;
}

NS_LEMON_RESOURCE_END
