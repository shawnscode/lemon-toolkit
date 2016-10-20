// @date 2016/10/09
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/drawcall.hpp>

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

uint32_t SortValue::post_depth_process(uint8_t transparency, uint32_t depth)
{
    return (transparency > 0 ? ~depth : depth);
}

uint64_t SortValue::encode(SortValue in)
{
    uint32_t dt = post_depth_process(in.transparency, in.depth);

    const uint64_t layer = ((uint64_t)in.layer) << value(SortValueShift::LAYER);
    const uint64_t transparency = ((uint64_t)in.transparency) << value(SortValueShift::TRANSPARENCY);
    const uint64_t depth = ((uint64_t)dt) << value(SortValueShift::DEPTH);
    const uint64_t program = ((uint64_t)in.program) << value(SortValueShift::PROGRAM);

    return layer | transparency | depth | program;
}

uint64_t SortValue::encode(RenderLayer layer, uint32_t depth, uint16_t program)
{
    SortValue in;
    in.layer = value(layer);
    in.transparency = layer == RenderLayer::TRANSPARENCY ? 1 : 0;
    in.program = program;
    in.depth = depth;

    return encode(in);
}

SortValue SortValue::decode(uint64_t in)
{
    SortValue out;
    out.layer = (in & value(SortValueMask::LAYER)) >> value(SortValueShift::LAYER);
    out.transparency = (in & value(SortValueMask::TRANSPARENCY)) >> value(SortValueShift::TRANSPARENCY);
    out.program = (in & value(SortValueMask::PROGRAM)) >> value(SortValueShift::PROGRAM);
    out.depth = (in & value(SortValueMask::DEPTH)) >> value(SortValueShift::DEPTH);
    out.depth = post_depth_process(out.transparency, out.depth);

    return out;
}

static const char* BUILDIN_UNIFORM_NAME[] =
{
    "lm_ProjectionMatrix",
    "lm_ViewMatrix",
    "lm_ModelMatrix",
    "lm_NormalMatrix",
    "lm_ViewPos",
};

const char* BuildinUniforms::name(BuildinUniforms::Enum uni)
{
    return BUILDIN_UNIFORM_NAME[value(uni)];
}

NS_LEMON_GRAPHICS_END