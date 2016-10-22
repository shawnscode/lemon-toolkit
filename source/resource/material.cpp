// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#include <resource/material.hpp>
#include <graphics/renderer.hpp>

#include <regex>
#include <iostream>

NS_LEMON_RESOURCE_BEGIN

Material::~Material()
{
    graphics::resource::free(_uniform);
}

bool Material::read(std::istream& in)
{
    auto frontend = core::get_subsystem<graphics::Renderer>();
    ENSURE(frontend != nullptr);

    _shader = Shader::color();
    _uniform = frontend->create<graphics::UniformBuffer>();
    return true;
}

bool Material::save(std::ostream& out)
{
    return true;
}

bool Material::update_video_object()
{
    graphics::resource::free(_uniform);
    _uniform = graphics::resource::create<graphics::UniformBuffer>();
    return _uniform != nullptr;
}

size_t Material::get_memory_usage() const
{
    auto size = sizeof(Material);

    if( _uniform != nullptr )
        size += sizeof(graphics::UniformBuffer);

    size += (_references.size() * sizeof(Image::ptr));
    return size;
}

size_t Material::get_video_memory_usage() const
{
    return 0;
}

bool Material::initialize(Shader::ptr shader)
{
    if( shader == nullptr )
    {
        LOGW("failed to create material without shader.");
        return false;
    }

    _shader = shader;
    _references.clear();
    return true;
}

NS_LEMON_RESOURCE_END