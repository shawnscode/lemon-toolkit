// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <resource/resource.hpp>
#include <graphics/graphics.hpp>

NS_LEMON_RESOURCE_BEGIN

struct Shader : public Resource
{
    using ptr = std::shared_ptr<Shader>;
    using weak_ptr = std::weak_ptr<Shader>;

    static Shader::ptr color();

public:
    virtual ~Shader();

    bool read(std::istream&) override;
    bool save(std::ostream&) override;
    bool update_video_object() override;

    size_t get_memory_usage() const override;
    size_t get_video_memory_usage() const override;

    // initialize shader with vertex/fragment text
    bool initialize(const char* vs, const char* fs);

    const std::string& get_vertex_shader() const;
    const std::string& get_fragment_shader() const;

    bool has_uniform_variable(const char* name) const;

    Handle get_video_uid() const;

protected:
    void scan_uniforms(const std::string&);

    Handle _program;
    bool _dirty = false;
    std::string _vertex;
    std::string _fragment;
    std::vector<std::string> _uniforms;
};

INLINE const std::string& Shader::get_vertex_shader() const
{
    return _vertex;
}

INLINE const std::string& Shader::get_fragment_shader() const
{
    return _fragment;
}

INLINE Handle Shader::get_video_uid() const
{
    return _program;
}

INLINE bool Shader::has_uniform_variable(const char* name) const
{
    for( size_t i = 0; i < _uniforms.size(); i++ )
    {
        if( _uniforms[i] == name )
            return true;
    }

    return false;
}

NS_LEMON_RESOURCE_END
