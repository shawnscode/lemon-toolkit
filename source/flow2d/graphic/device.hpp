// @date 2016/05/31
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <flow2d/forward.hpp>

NS_FLOW2D_BEGIN

using Rid = size_t;

enum class RenderObject : uint8_t
{
    INVALID         = 0,
    SHADER,
    VERTEX_BUFFER,
    INDEX_BUFFER,
    TEXTURE,
    TARGET,
};

enum class TextureFormat : uint8_t
{
    INVALID = 0,
    RGBA8,
    RGBA4,
    RGB8,
    RGB565,
    ALPHA8,
    PVR2,
    PVR4,
    ETC1,
};

enum class ElementFormat : uint8_t
{
    BYTE = 0,
    UNSIGNED_BYTE,
    SHORT,
    UNSIGNED_SHORT,
    INT,
    UNSIGNED_INT,
    FLOAT,
};

enum class UniformFormat : uint8_t
{
    INVALID = 0,
    FLOAT1,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    VECTOR_F1,
    VECTOR_F2,
    VECTOR_F3,
    VECTOR_F4,
    MATRIX_F33,
    MATRIX_F44,
};

enum class BlendFunc : uint8_t
{
    DISABLE = 0,
    ZERO,
    ONE,
    SRC_COLOR,
    ONE_MINUS_SRC_COLOR,
    SRC_ALPHA,
    ONE_MINUS_SRC_ALPHA,
    DST_ALPHA,
    ONE_MINUS_DST_ALPHA,
    DST_COLOR,
    ONE_MINUS_DST_COLOR,
    SRC_ALPHA_SATURATE,
};

enum class DepthTestFunc : uint8_t
{
    DISABLE = 0,
    LESS_EQUAL,
    LESS,
    EQUAL,
    GREATER,
    GREATER_EQUAL,
    ALWAYS,
};

enum ClearMask
{
    CLEAR_COLOR   = 0x1,
    CLEAR_DEPTH   = 0x2,
    CLEAR_STENCIL = 0x4,
};

enum class DrawMode : uint8_t
{
    TRIANGLE = 0,
    LINE,
};

enum class CullMode : uint8_t
{
    DISABLE = 0,
    FRONT,
    BACK,
};

struct GraphicDeviceInstance;
struct GraphicDevice
{
    static GraphicDevice& instance();
    static bool initialize();
    static void dispose();

    void set_viewport(int x, int y, int width, int height);
    void set_scissor(bool enable, int x=0, int y=0, int width=0, int height=0);
    void set_blend(BlendFunc src, BlendFunc dst);
    void set_depth(bool write, DepthTestFunc test);
    void set_cull(CullMode mode);
    void reset();

    void bind_shader(Rid id);
    void bind_index_buffer(Rid id, ElementFormat format, size_t stride, size_t offset);
    void bind_vertex_buffer(
        int index, Rid id, size_t n, ElementFormat format, size_t stride, size_t offset, bool normalized = false);
    void bind_texture(int index, Rid id);
    void bind_uniform(int index, UniformFormat format, const float* v);

    void clear(uint32_t mask, uint8_t, uint8_t, uint8_t, uint8_t a = 255);
    void draw(DrawMode mode, size_t from_index, size_t number_index);
    void flush();

    Rid create_shader(
        const char* vs, const char* fs,
        size_t attribute_n,
        size_t texture_n, const char** textures,
        size_t uniform_n, const char** uniforms);
    Rid create_texture(const void* data, size_t width, size_t height, TextureFormat format, int mipmap);
    Rid create_buffer(RenderObject what, const void* data, size_t size);

    void update_buffer(RenderObject what, Rid rid, const void* data, size_t size);
    void release(RenderObject what, Rid rid);

    static const Rid INVALID = 0;

protected:
    GraphicDeviceInstance* m_state;
};

NS_FLOW2D_END