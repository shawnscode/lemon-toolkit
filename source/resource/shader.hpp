// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <resource/resource.hpp>
#include <codebase/handle.hpp>
#include <math/color.hpp>

NS_LEMON_RESOURCE_BEGIN

struct Shader : public Resource
{
    using ptr = std::shared_ptr<Shader>;
    using weak_ptr = std::weak_ptr<Shader>;

    bool read(std::istream&) override;
    bool save(std::ostream&) override;

    Handle get_program_handle() const
    {
        return _program;
    }

    const std::string& get_vertex_shader() const
    {
        return _vertex;
    }

    const std::string& get_fragment_shader() const
    {
        return _fragment;
    }

protected:
    Handle _program;
    std::string _vertex;
    std::string _fragment;
};

NS_LEMON_RESOURCE_END
