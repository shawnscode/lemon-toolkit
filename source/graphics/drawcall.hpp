// @date 2016/10/09
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <graphics/graphics.hpp>
#include <graphics/state.hpp>

NS_LEMON_GRAPHICS_BEGIN

// specifies in which kind of order the drawcall will be rendered.
enum class RenderLayer : uint16_t
{
    BACKGROUND = 1000,
    GEOMETRY = 2000,
    ALPHATEST = 2500,
    // transparent geometreis will be renderred from-back-to-front
    TRANSPARENCY = 3000,
    OVERLAY = 4000
};

// specifies what kind of primitives to render.
enum class PrimitiveType : uint8_t
{
    POINTS = 0,
    LINES,
    LINE_LOOP,
    LINE_STRIP,
    TRIANGLES,
    TRIANGLE_STRIP,
    TRIANGLE_FAN,
};

// SortValue
struct SortValue
{
    uint8_t layer;
    uint8_t transparency;
    uint32_t depth;
    uint16_t program;

    static uint32_t post_depth_process(uint8_t transparency, uint32_t depth);
    static uint64_t encode(SortValue in);
    static uint64_t encode(RenderLayer layer, uint32_t depth, uint16_t program);
    static SortValue decode(uint64_t in);
};

// RenderDrawcall
struct RenderDrawcall
{
    // stateless render state
    RenderState state;
    // graphics resources we need to make a draw call
    Handle program;
    Handle uniform_buffer;
    Handle vertex_buffer;
    Handle index_buffer;
    // specifies what kind of primitive to render
    PrimitiveType primitive = PrimitiveType::TRIANGLES;
    // specifies the starting index in the enabled arrays
    unsigned first = 0;
    // specifies the number of indices or vertices to be rendered
    unsigned count = 0;

protected:
    friend class Renderer;
    // a packing value which is then used for sorting
    uint64_t sort;
};

NS_LEMON_GRAPHICS_END