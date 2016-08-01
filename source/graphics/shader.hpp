#pragma once

#include <graphics/device.hpp>
#include <math/matrix.hpp>
#include <math/string_hash.hpp>

NS_FLOW2D_GFX_BEGIN

// ShaderVariation
// Material

struct Shader : public GPUObject
{


    void set_uniform1f(const char*, const math::Vector<1, float>&);
    void set_uniform2f(const char*, const math::Vector<2, float>&);
    void set_uniform3f(const char*, const math::Vector<3, float>&);
    void set_uniform4f(const char*, const math::Vector<4, float>&);

    void set_uniform1fv(const char*, const math::Vector<1, float>&);
    void set_uniform2fv(const char*, const math::Vector<2, float>&);
    void set_uniform3fv(const char*, const math::Vector<3, float>&);
    void set_uniform4fv(const char*, const math::Vector<4, float>&);

    void set_uniform3fm(const char*, const math::Matrix<3, 3, float>&);
    void set_uniform4fm(const char*, const math::Matrix<4, 4, float>&);

    void set_uniform_texture(const char*, Texture*);

protected:
    std::pair<StringHash, Texture*> _textures[kMaxTextures];
};

NS_FLOW2D_GFX_END