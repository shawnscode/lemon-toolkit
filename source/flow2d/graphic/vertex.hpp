// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <flow2d/forward.hpp>
#include <flow2d/math/vector.hpp>

NS_FLOW2D_BEGIN

struct Vertex2f
{
    Vertex2f()
    {}

    Vertex2f(std::initializer_list<float> values)
    {
        auto size = values.size();
        auto cursor = values.begin();

        if( size >= 1 ) position[0] = *cursor++;
        if( size >= 2 ) position[1] = *cursor++;
        if( size >= 3 ) texcoord[0] = *cursor++;
        if( size >= 4 ) texcoord[1] = *cursor++;
    }

    Vertex2f(const Vector2f& position, const Vector2f& texcoord)
    : position(position), texcoord(texcoord)
    {}

    Vector2f position;
    Vector2f texcoord;
};

NS_FLOW2D_END