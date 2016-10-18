// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#include <resource/material.hpp>
#include <graphics/renderer.hpp>

#include <regex>
#include <iostream>

NS_LEMON_RESOURCE_BEGIN

static const char* BUILDIN_UNIFORM_NAME[] =
{
    "ProjectionMatrix",
    "ViewMatrix",
    "ModelMatrix",
    "ViewPos",
    "LightPos",
    "LightColor",
};

const char* Material::name(Material::BuildinUniform uni)
{
    return BUILDIN_UNIFORM_NAME[value(uni)];
}

Material::ptr Material::create(const fs::Path& name, Shader::ptr shader)
{
    if( shader == nullptr )
        return nullptr;

    auto cache = core::get_subsystem<ResourceCache>();
    if( cache->is_exist(name) )
        return cache->get<Material>(name);

    auto frontend = core::get_subsystem<graphics::Renderer>();
    auto material = Material::ptr(new (std::nothrow) Material);
    material->_shader = shader;
    material->_uniform = frontend->create<graphics::UniformBuffer>();

    cache->add(name, material);
    return material;
}

Material::~Material()
{
    if( core::details::status() == core::details::Status::RUNNING )
        core::get_subsystem<graphics::Renderer>()->free(_uniform);
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

NS_LEMON_RESOURCE_END