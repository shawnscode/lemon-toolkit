// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#include <resource/material.hpp>
#include <graphics/frontend.hpp>

#include <regex>
#include <iostream>

NS_LEMON_RESOURCE_BEGIN

Material::~Material()
{
    if( auto frontend = core::get_subsystem<graphics::RenderFrontend>() )
        frontend->free_render_state(_render_state);
}

bool Material::read(std::istream& in)
{
    _shader = Shader::color();
    return update_video_object();
}

bool Material::save(std::ostream& out)
{
    return true;
}

bool Material::update_video_object()
{
    auto frontend = core::get_subsystem<graphics::RenderFrontend>();
    _render_state = frontend->create_render_state(graphics::RenderState());
    return _render_state.is_valid();
}

size_t Material::get_memory_usage() const
{
    auto size = sizeof(Material);
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

    for( size_t i = 0; i < _texture_size; i++ )
        _textures[i].second = nullptr;

    _shader = shader;
    _uniform_dirty = false;
    _uniform_buffer.invalidate();
    _uniform_size = 0;
    _texture_size = 0;
    return true;
}

bool Material::set_uniform_variable(const char* name, const graphics::UniformVariable& v)
{
    if( !_shader || !_shader->has_uniform_variable(name) )
        return false;

    _uniform_dirty = true;

    auto hash = math::StringHash(name);
    for( size_t i = 0; i < _uniform_size; i++ )
    {
        if( _uniforms[i].first == hash )
        {
            _uniforms[i].second = v;
            return true;
        }
    }

    ASSERT( _uniform_size < graphics::kMaxUniformsPerMaterial,
        "too many unifoms(%d) per material.", graphics::kMaxUniformsPerMaterial );

    _uniforms[_uniform_size].first = hash;
    _uniforms[_uniform_size++].second = v;
    return true;
}

bool Material::set_texture(const char* name, Image::ptr image)
{
    auto hash = math::StringHash(name);
    for( size_t i = 0; i < _texture_size; i++ )
    {
        if( _textures[i].first == hash )
        {
            _textures[i].second = image;
        }
    }

    ASSERT( _texture_size < graphics::kMaxTexturePerMaterial,
        "too many textures(%d) per material.", graphics::kMaxTexturePerMaterial );

    _textures[_texture_size].first = hash;
    _textures[_texture_size++].second = image;
    
    graphics::UniformVariable v;
    v.set<Handle>(image->get_video_uid());
    return set_uniform_variable(name, v);
}

bool Material::set_render_state(const graphics::RenderState& state)
{
    if( auto frontend = core::get_subsystem<graphics::RenderFrontend>() )
    {
        frontend->update_render_state(_render_state, state);
        return true;
    }

    return false;
}

Handle Material::get_video_uniforms()
{
    auto frontend = core::get_subsystem<graphics::RenderFrontend>();
    if( !frontend->is_uniform_buffer_alive(_uniform_buffer) || _uniform_dirty )
    {
        _uniform_buffer = frontend->allocate_uniform_buffer(_uniform_size);
        for( size_t i = 0; i < _uniform_size; i++ )
        {
            auto& pair = _uniforms[i];
            frontend->update_uniform_buffer(_uniform_buffer, pair.first, pair.second);
        }
    }

    return _uniform_buffer;
}

Handle Material::get_video_state()
{
    return _render_state;
}

NS_LEMON_RESOURCE_END