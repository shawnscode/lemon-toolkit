// @date 2016/07/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <defines.hpp>

#define NS_FLOW2D_GFX_BEGIN namespace flow2d { namespace graphics {
#define NS_FLOW2D_GFX_END } }

NS_FLOW2D_GFX_BEGIN

using ResourceHandle = size_t;

// vertex/index buffer lock state
enum class LockState
{
    NONE,
    HARDWARE,
    SHADOW,
    SCRATCH
};

// arbitrary vertex declaration element datatypes
enum class VertexElementType
{
    INT,
    FLOAT,
    VECTOR2,
    VECTOR3,
    VECTOR4,
    UBYTE4,
    UBYTE4_NORM
};

// arbitrary vertex declaration element semantics
enum class VertexElementSemantic
{
    POSITION,
    NORMAL,
    BINORMAL,
    TANGENT,
    TEXCOORD,
    COLOR,
    BLENDWEIGHTS,
    BLENDINDICES,
    OBJECTINDEX
};

struct Resource;
struct Shader;
struct IndexBuffer;
struct VertexBuffer;
struct GraphicsEngine;

NS_FLOW2D_GFX_END