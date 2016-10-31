// @date 2016/10/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <graphics/graphics.hpp>
#include <codebase/handle.hpp>

NS_LEMON_GRAPHICS_BEGIN

struct RenderDrawCall
{
    // the unique handle of shader program we are applying in drawing
    Handle program;
    // the unique handle of render state instance
    Handle state;
    // the unique handle of vertices and indices buffer
    Handle buffer_vertex;
    Handle buffer_index;
    // offsets to identifies the uniforms buffer used by this draw call
    Handle uniforms;
    // its common to have some shared uniforms such like view and projection matrix
    Handle shared_uniforms;
    // specifies the offset and count of indices or vertices to be drawed
    uint16_t first, num;
};

NS_LEMON_GRAPHICS_END
