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
    // returns string representation of shaders
    const std::string& get_vertex_shader() const;
    const std::string& get_fragment_shader() const;

    // returns graphics object of program
    graphics::Program* get_program() const;

protected:
    graphics::Program* _program = nullptr;
    bool _dirty = false;
    std::string _vertex;
    std::string _fragment;
};

INLINE const std::string& Shader::get_vertex_shader() const
{
    return _vertex;
}

INLINE const std::string& Shader::get_fragment_shader() const
{
    return _fragment;
}

INLINE graphics::Program* Shader::get_program() const
{
    return _program;
}

NS_LEMON_RESOURCE_END
