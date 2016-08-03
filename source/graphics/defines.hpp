// @date 2016/07/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <defines.hpp>

#define NS_FLOW2D_GFX_BEGIN namespace flow2d { namespace graphics {
#define NS_FLOW2D_GFX_END } }

NS_FLOW2D_GFX_BEGIN

const static size_t kMaxVertexAttributes = 8;
const static size_t kMaxTextures = 8;

using ResourceHandle = size_t;

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

enum class Orientation : uint8_t
{
    LANDSCAPE_LEFT = 0,
    LANDSCAPE_RIGHT,
    PORTRAIT,
    PORTRAIT_UPSIDE_DOWN
};

enum class CompareMode : uint8_t
{
    ALWAYS = 0,
    EQUAL,
    NOT_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
};

enum class StencilOp : uint8_t
{
    KEEP = 0,
    ZERO,
    REF,
    INCR,
    DECR
};

enum class CullMode : uint8_t
{
    NONE = 0,
    CCW,
    CW
};

enum class BlendMode : uint8_t
{
    REPLACE = 0,
    ADD,
    MULTIPLY,
    ALPHA,
    ADDALPHA,
    PREMULALPHA,
    INVDESTALPHA,
    SUBTRACT,
    SUBTRACTALPHA
};

struct Device;
struct GPUObject;
struct IndexBuffer;
struct VertexBuffer;

NS_FLOW2D_GFX_END