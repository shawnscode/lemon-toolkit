// @date 2016/10/10
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/graphics.hpp>
#include <graphics/details/uniform.hpp>

NS_LEMON_GRAPHICS_BEGIN

bool UniformBufferView::set_uniform_1f(const char* name, const math::Vector<1, float>& value)
{
    if( auto uniform = find(name) )
    {
        uniform->value.set<math::Vector<1, float>>(value);
        return true;
    }

    return false;
}

bool UniformBufferView::set_uniform_2f(const char* name, const math::Vector<2, float>& value)
{
    if( auto uniform = find(name) )
    {
        uniform->value.set<math::Vector<2, float>>(value);
        return true;
    }

    return false;
}

bool UniformBufferView::set_uniform_3f(const char* name, const math::Vector<3, float>& value)
{
    if( auto uniform = find(name) )
    {
        uniform->value.set<math::Vector<3, float>>(value);
        return true;
    }

    return false;
}

bool UniformBufferView::set_uniform_4f(const char* name, const math::Vector<4, float>& value)
{
    if( auto uniform = find(name) )
    {
        uniform->value.set<math::Vector<4, float>>(value);
        return true;
    }

    return false;
}

bool UniformBufferView::set_uniform_2fm(const char* name, const math::Matrix<2, 2, float>& value)
{
    if( auto uniform = find(name) )
    {
        uniform->value.set<math::Matrix<2, 2, float>>(value);
        return true;
    }

    return false;
}

bool UniformBufferView::set_uniform_3fm(const char* name, const math::Matrix<3, 3, float>& value)
{
    if( auto uniform = find(name) )
    {
        uniform->value.set<math::Matrix<3, 3, float>>(value);
        return true;
    }

    return false;
}

bool UniformBufferView::set_uniform_4fm(const char* name, const math::Matrix<4, 4, float>& value)
{
    if( auto uniform = find(name) )
    {
        uniform->value.set<math::Matrix<4, 4, float>>(value);
        return true;
    }

    return false;
}

bool UniformBufferView::set_uniform_texture(const char* name, Handle value)
{
    if( auto uniform = find(name) )
    {
        uniform->value.set<Handle>(value);
        return true;
    }

    return false;
}

Uniform* UniformBufferView::find(const char* name)
{
    for( auto i = 0; i < _tail; i ++ )
        if( _uniforms[i]->name == name )
            return _uniforms[i];

    if( _tail < _size )
    {
        _uniforms[_tail]->name = name;
        return _uniforms[_tail++];
    }

    return nullptr;
}


NS_LEMON_GRAPHICS_END
