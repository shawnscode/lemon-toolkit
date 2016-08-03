// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/device.hpp>
#include <math/matrix.hpp>
#include <math/string_hash.hpp>

NS_FLOW2D_GFX_BEGIN

enum class ElementFormat : uint8_t
{
    BYTE = 0,
    UNSIGNED_BYTE,
    SHORT,
    UNSIGNED_SHORT,
    FIXED,
    FLOAT
};

struct VertexAttribute
{
    math::StringHash    name;
    unsigned            vbo = 0;
    unsigned            size;
    ElementFormat       format;
    bool                normalized;
    unsigned            stride;
    unsigned            offset;
};

struct Shader : public GPUObject
{
    virtual ~Shader() { dispose(); }

    void bind();

    void set_uniform1f(const char*, const math::Vector<1, float>&);
    void set_uniform2f(const char*, const math::Vector<2, float>&);
    void set_uniform3f(const char*, const math::Vector<3, float>&);
    void set_uniform4f(const char*, const math::Vector<4, float>&);

    void set_uniform2fm(const char*, const math::Matrix<2, 2, float>&);
    void set_uniform3fm(const char*, const math::Matrix<3, 3, float>&);
    void set_uniform4fm(const char*, const math::Matrix<4, 4, float>&);

    void set_vertex_attribute(const char*, unsigned, ElementFormat, bool, unsigned, unsigned);

protected:
    friend class Device;
    Shader(Device& device, const char* vs, const char* ps)
    : GPUObject(device), _vertex_shader(vs), _fragment_shader(ps) {}

    bool initialize() override;
    void dispose() override;
    int32_t get_uniform_location(const char*);

protected:
    std::string _fragment_shader, _vertex_shader;
    std::vector<VertexAttribute> _attributes;
    std::vector<std::pair<math::StringHash, int32_t>> _locations;
    unsigned _vao = 0;
};

NS_FLOW2D_GFX_END