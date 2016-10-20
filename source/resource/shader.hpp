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
    static Shader::ptr create(const fs::Path&, const char* vs, const char* ps);

public:
    virtual ~Shader();

    bool read(std::istream&) override;
    bool save(std::ostream&) override;
    size_t get_memory_usage() const override;

    // specified input identifier of vertex attribute
    bool set_attribute_name(graphics::VertexAttribute::Enum, const char*);
    // returns true if we have uniform associated with name in program
    bool has_uniform(const char*) const;

    // returns handle of specified graphics object
    Handle get_program_handle() const;

    // returns string representation of shaders
    const std::string& get_vertex_shader() const;
    const std::string& get_fragment_shader() const;

protected:
    graphics::Program* _program = nullptr;
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

INLINE bool Shader::set_attribute_name(graphics::VertexAttribute::Enum value, const char* name)
{
    return _program == nullptr ? false : _program->set_attribute_name(value, name);
}

INLINE bool Shader::has_uniform(const char* name) const
{
    return _program == nullptr ? false : _program->has_uniform(name);
}

INLINE Handle Shader::get_program_handle() const
{
    return _program == nullptr ? Handle() : _program->handle;
}

NS_LEMON_RESOURCE_END
