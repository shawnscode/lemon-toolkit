// @date 2016/07/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/defines.hpp>
#include <graphics/color.hpp>
#include <core/context.hpp>
#include <core/typeinfo.hpp>
#include <math/rect.hpp>

NS_FLOW2D_GFX_BEGIN

// struct Resource
// {
//     // return the resource's device handle
//     ResourceHandle get_handle() const { return _handle; }

// protected:
//     friend class Device;
//     ResourceHandle _handle;
// };

// using ResourceHandle    = size_t;
// using ResourceWeakPtr   = std::weak_ptr<Resource>;
// using ResourcePtr       = std::shared_ptr<Resource>;

enum class PrimitiveType : uint8_t
{
    TRIANGLE_LIST = 0,
    TRIANGLE_STRIP,
    TRIANGLE_FAN,
    LINE_LIST,
    LINE_STRIP,
    POINT_LIST,
};

enum class Orientation : uint8_t
{
    LANDSCAPE_LEFT,
    LANDSCAPE_RIGHT,
    PORTRAIT,
    PORTRAIT_UPSIDE_DOWN
};

enum class WindowOption : uint16_t
{
    NONE        = 0x0,
    FULLSCREEN  = 0x1,
    BORDERLESS  = 0x2,
    RESIZABLE   = 0x4,
    HIGHDPI     = 0x8,
    VSYNC       = 0x10,
    TRIPLEBUF   = 0x20,
};

enum class ClearOption : uint8_t
{
    NONE    = 0x0,
    COLOR   = 0x1,
    DEPTH   = 0x2,
    STENCIL = 0x4
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

enum class FillMode : uint8_t
{
    SOLID = 0,
    WIREFRAME,
    POINT
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

// graphics device subsystem. manages the window device, renedering state and gpu resources
struct DeviceContext;
struct Device : core::Subsystem
{
    SUBSYSTEM("Graphics'Device")

    Device(core::Context& c) : Subsystem(c) {}
    virtual ~Device() {}

    bool initialize() override;
    void dispose() override;

    // restore resources and reinitialize state, requires an open window. returns true if successful
    bool restore();
    // release/clear GPU objects and optionally close the window
    void release(bool, bool close_window = false);

    // set allowed screen orientations as a space-separated way
    void set_orientations(Orientation);
    // set the initial left-upper position of window
    void set_position(const math::Vector2i&);
    // create window with width, height, and options. return true if successful
    bool set_window(int, int, int multisample = 1, WindowOption options = WindowOption::NONE);
    // close the window
    void close_window();

    // begin frame rendering. return true if device available and can reneder
    bool begin_frame();
    // end frame rendering and swap buffers
    void end_frame();
    // clear any or all of rendertarget, depth buffer and stencil buffer
    void clear(ClearOption, const Color& color = {0.f, 0.f, 0.f, 0.f}, float depth = 1.f, unsigned stencil = 0);

    // set viewport
    void set_viewport(const math::Rect2i&);
    // set blend mode
    void set_blend_mode(BlendMode);
    // set depth test/write/bias
    void set_depth_write(bool);
    void set_depth_test(CompareMode);
    void set_depth_bias(float, float);
    // set cull/fill mode
    void set_cull_mode(CullMode);
    void set_fill_mode(FillMode);
    // set scissor test
    void set_scissor_test(bool, const math::Rect2i&);
    // set color writable
    void set_color_write(bool);
    // set stencil test
    void set_stencil_test(bool,
        CompareMode mode = CompareMode::ALWAYS,
        StencilOp pass = StencilOp::KEEP,
        StencilOp fail = StencilOp::KEEP,
        StencilOp zfail = StencilOp::KEEP,
        unsigned ref = 0,
        unsigned compare_mask = math::max<unsigned>(),
        unsigned write_mask = math::max<unsigned>());

    // restore gpu object and reinitialize state, returns a custom shared_ptr
    template<typename T, typename ... Args> std::shared_ptr<T> spawn(Args&& ...);
    // set vertex buffer
    void set_vertex_buffer(VertexBuffer*);
    // set index buffer
    void set_index_buffer(IndexBuffer*);
    // set shader
    void set_shader(Shader*);

    // draw non-indexed geometry
    void draw(PrimitiveType, unsigned start, unsigned count);
    // draw indexed geometry
    void draw_index(PrimitiveType, unsigned start, unsigned count);

protected:
    // window options and status
    int             _multisamples = 0;
    math::Vector2i  _size = {1, 1}, _position = {0, 0};
    Orientation     _orientation = Orientation::PORTRAIT;
    WindowOption    _options = WindowOption::NONE;

    DeviceContext*  _device = nullptr;
};

// high-level rendering subsystem, manages drawing of views
struct Renderer
{

};

struct GUIRenderer
{

};

struct Material
{

};

NS_FLOW2D_GFX_END

ENABLE_BITMASK_OPERATORS(flow2d::graphics::WindowOption);
ENABLE_BITMASK_OPERATORS(flow2d::graphics::ClearOption);

