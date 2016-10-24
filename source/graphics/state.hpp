// @date 2016/08/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <math/rect.hpp>
#include <codebase/enumeration.hpp>

NS_LEMON_GRAPHICS_BEGIN

// specify whether front- or back-facing polygons can be culled
enum class CullFace : uint8_t
{
    FRONT = 0,
    BACK,
    FRONTANDBACK
};

// define front- and back-facing polygons
enum class FrontFaceOrder : uint8_t
{
    CLOCKWISE = 0,
    COUNTERCLOCKWISE
};

// compare functions
enum class CompareEquation : uint8_t
{
    NEVER = 0,
    LESS,
    LEQUAL,
    GREATER,
    GEQUAL,
    EQUAL,
    NOTEQUAL,
    ALWAYS
};

// set front and back stencil write actions
enum class StencilWriteEquation : uint8_t
{
    KEEP = 0,
    ZERO,
    REPLACE,
    INCR,
    INCR_WRAP,
    DECR,
    DECR_WRAP,
    INVERT
};

// specifies how the red, green, blue, and alpha blending factors are computed
enum class BlendFactor : uint8_t
{
    ZERO = 0,
    ONE,
    SRC_COLOR,
    INV_SRC_COLOR,
    DST_COLOR,
    INV_DST_COLOR,
    SRC_ALPHA,
    INV_SRC_ALPHA,
    DST_ALPHA,
    INV_DST_ALPHA,
    SRC_ALPHA_SATURATE
};

// specifies how source and destination colors are combined
enum class BlendEquation : uint8_t
{
    ADD = 0,
    SUBTRACT,
    REVERSE_SUBTRACT
};

// enable and disable writing of frame buffer color components
enum class ColorMask : uint8_t
{
    RED     = 0x1,
    GREEN   = 0x2,
    BLUE    = 0x4,
    ALPHA   = 0x8,
    ALL     = 0xF
};

struct CullTestOp
{
    bool enable;
    CullFace face;
    FrontFaceOrder winding;
};

struct ScissorTestOp
{
    bool enable;
    math::Rect2i area;
};

struct DepthTestOp
{
    // enable or disable depth buffer test
    bool enable;
    // specify the value used for depth buffer comparisons and test
    CompareEquation compare;
};

struct DepthBufferWrite
{
    // enable or disable writing into the depth buffer
    bool enable;
    // set the scale and units used to calculate depth values
    float bias_slope_scaled;
    float bias_constant;
};

struct ColorBlendOp
{
    bool enable;
    // specifies how source and destination colors are combined
    BlendEquation equation;
    // specifies how the red, green, blue, and alpha blending factors are computed
    BlendFactor source_factor;
    BlendFactor destination_factor;
};

// set front and back function and reference value for stencil testing,
// reference is clamped to the range [0,2^n−1],
// where n is the number of bitplanes in the stencil buffer.
// EQ: compare(reference & mask, stencil & mask)
struct StencilTestOp
{
    bool enable;
    unsigned reference;
    unsigned mask;
    CompareEquation compare;
};

struct StencilBufferWrite
{
    // specifies the action to take when the stencil test fails
    StencilWriteEquation sfail;
    // specifies the stencil action when the stencil test passes, but the depth test fails
    StencilWriteEquation dpfail;
    // specifies the stencil action when both the stencil test and the depth test pass
    StencilWriteEquation dppass;
    // control the front and back writing of individual bits in the stencil planes
    unsigned mask;
};

// stateless render states used to address multi-thread draw call submittings
struct RenderState
{
    RenderState();

    CullTestOp cull;
    ScissorTestOp scissor;

    DepthTestOp depth;
    DepthBufferWrite depth_write;

    ColorBlendOp blend;
    ColorMask color_write;

    StencilTestOp stencil;
    StencilBufferWrite stencil_write;

    void reset();
};

NS_LEMON_GRAPHICS_END

ENABLE_BITMASK_OPERATORS(lemon::graphics::ColorMask);
