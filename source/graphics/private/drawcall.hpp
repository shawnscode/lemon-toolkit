// @date 2016/10/09
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>

#include <graphics/renderer.hpp>
#include <graphics/state.hpp>

#include <algorithm>

NS_LEMON_GRAPHICS_BEGIN

enum class SortValueMask : uint64_t
{
    PROGRAM = 0x000000000000FFFF,
    // sort opaque geometry front-to-back to aid z-culling,
    // sort translucent geometry back-to-front for proper draw
    DEPTH = 0x0000FFFFFFFF0000,
    // opaque, alpha-test, transparent
    TRANSPARENCY = 0x00FF00000000000000,
    // skybox layer, world layer, effects layer, HUD, etc.
    LAYER = 0xFF00000000000000
};

enum class SortValueShift : uint8_t
{
    PROGRAM = 0,
    DEPTH = 16,
    TRANSPARENCY = 48,
    LAYER = 56
};

struct SortValue
{
    uint8_t layer;
    uint8_t transparency;
    uint16_t program;
    uint32_t depth;

    static uint32_t post_depth_process(uint8_t transparency, uint32_t depth)
    {
        return (transparency > 0 ? ~depth : depth);
    }

    static uint64_t encode(SortValue in)
    {
        uint32_t dt = post_depth_process(in.transparency, in.depth);

        const uint64_t layer = ((uint64_t)in.layer) << value(SortValueShift::LAYER);
        const uint64_t transparency = ((uint64_t)in.transparency) << value(SortValueShift::TRANSPARENCY);
        const uint64_t depth = ((uint64_t)dt) << value(SortValueShift::DEPTH);
        const uint64_t program = ((uint64_t)in.program) << value(SortValueShift::PROGRAM);

        return layer | transparency | depth | program;
    }

    static uint64_t encode(RenderLayer layer, Handle program, uint32_t depth)
    {
        SortValue in;
        in.layer = value(layer);
        in.transparency = layer == RenderLayer::TRANSPARENCY ? 1 : 0;
        in.program = program.get_index();
        in.depth = depth;

        return encode(in);
    }

    static SortValue decode(uint64_t in)
    {
        SortValue out;
        out.layer = (in & value(SortValueMask::LAYER)) >> value(SortValueShift::LAYER);
        out.transparency = (in & value(SortValueMask::TRANSPARENCY)) >> value(SortValueShift::TRANSPARENCY);
        out.program = (in & value(SortValueMask::PROGRAM)) >> value(SortValueShift::PROGRAM);
        out.depth = (in & value(SortValueMask::DEPTH)) >> value(SortValueShift::DEPTH);
        out.depth = post_depth_process(out.transparency, out.depth);

        return out;
    }
};

struct RenderDrawcall
{
    RenderState state;
    Handle program;
    Handle vertex_buffer;
    Handle index_buffer;
    uint32_t start;
    uint32_t vertex_count;
    uint64_t sort_value;
};

// struct RenderFrame
// {
//     std::vector<RenderDraw>::iterator begin()
//     {
//         return _drawcalls.begin();
//     }

//     std::vector<RenderDraw>::iterator end()
//     {
//         return _drawcalls.end();
//     }

//     void clear()
//     {
//         _drawcalls.clear();
//     }

//     void push_back(const RenderDraw& dc)
//     {
//         _drawcalls.push_back(dc);
//     }

//     void sort()
//     {
//         std::sort(_drawcalls.begin(), _drawcalls.end(), compare);
//     }

// protected:
//     static bool compare(const RenderDraw& lhs, const RenderDraw& rhs)
//     {
//         return lhs.sort_value < rhs.sort_value;
//     }

//     std::vector<RenderDraw> _drawcalls;
// };

NS_LEMON_GRAPHICS_END