// @date 2016/07/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/defines.hpp>
#include <core/context.hpp>
#include <core/typeinfo.hpp>
#include <math/rect.hpp>
#include <math/color.hpp>

NS_FLOW2D_GFX_BEGIN

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

// device events
struct EvtDeviceLost {};
struct EvtDeviceRestore {};

// the GraphicsObject class is the abstract base class for resources, shaders
struct GraphicsObject
{
    GraphicsObject(Device& device);
    virtual ~GraphicsObject();

    void receive(const EvtDeviceRestore&);
    void receive(const EvtDeviceLost&);

    using receiver = std::function<void(GraphicsObject&)>;
    receiver on_device_restore = nullptr;
    receiver on_device_release = nullptr;

    virtual bool restore();
    virtual void release();

    unsigned get_graphic_object() const { return _object; }

protected:
    Device&     _device;
    unsigned    _object = 0;
};

// graphics device subsystem. manages the window device, renedering state and gpu resources
struct DeviceContext;
struct Device : public core::Subsystem
{
    SUBSYSTEM("Graphics'Device")

    Device(core::Context& c) : Subsystem(c) {}
    virtual ~Device() {}

    bool initialize() override;
    void dispose() override;

    // handle window messages, called from engine
    void process_message(void*);

    // restore OpenGL context and reinitialize state, requires an open window. returns true if successful
    bool restore_context();
    // release OpenGL context and handle the device lost of GPU resources
    void release_context();
    // reset all the graphics state to default
    void reset_cached_state();

    // set allowed screen orientations as a space-separated way
    void set_orientations(Orientation);
    // set the initial left-upper position of window
    void set_position(const math::Vector2i&);
    // create window with width, height, and options. return true if successful
    bool spawn_window(int, int, int multisample = 1, WindowOption options = WindowOption::NONE);
    // close current window and release OpenGL context
    void close_window();

    // begin frame rendering. return true if device available and can reneder
    bool begin_frame();
    // end frame rendering and swap buffers
    void end_frame();
    // clear any or all of rendertarget, depth buffer and stencil buffer
    void clear(ClearOption, const math::Color& color = {0.f, 0.f, 0.f, 0.f}, float depth = 1.f, unsigned stencil = 0);

    // set current shader program
    void set_shader(unsigned);
    // set index buffer
    void set_index_buffer(unsigned);
    // set vertex buffer
    void set_vertex_buffer(unsigned);
    // set texture
    void set_texture(unsigned, unsigned, unsigned);

    // prepare for draw call. setup corresponding frame/vertex buffer object
    void prepare_draw();
    // draw non-indexed geometry
    void draw(PrimitiveType, unsigned start, unsigned count);
    // draw indexed geometry
    void draw_index(PrimitiveType, unsigned start, unsigned count);

    // set viewport
    void set_viewport(const math::Rect2i&);
    // set blending mode
    void set_blend_mode(BlendMode);
    // set hardware culling mode
    void set_cull_mode(CullMode);

    // per-sample processing
    void set_scissor_test(bool, const math::Rect2i& scissor = {{0, 0}, {0, 0}});
    void set_stencil_test(bool, CompareMode, unsigned ref = 0, unsigned compare_mask = math::max<unsigned>());
    void set_stencil_write(StencilOp pass, StencilOp fail, StencilOp zfail, unsigned write_mask = math::max<unsigned>());
    void set_depth_test(bool, CompareMode);
    void set_depth_write(bool);
    void set_depth_bias(float slope_scaled = 0.f, float constant = 0.f);
    void set_color_write(bool);

    // restore gpu object and reinitialize state, returns a custom shared_ptr
    template<typename T> using spawn_return = typename std::enable_if<
        std::is_base_of<GraphicsObject, T>::value,
        std::shared_ptr<T>>::type;
    template<typename T, typename ... Args> spawn_return<T> spawn(Args&& ...);

    // check if we have valid window and OpenGL context
    bool is_device_lost() const;

    unsigned get_bound_vertex_buffer() const { return _bound_vbo; }
    unsigned get_bound_index_buffer() const { return _bound_ibo; }
    unsigned get_bound_shader() const { return _bound_program; }

    // return window position/size
    math::Vector2i get_window_position() const { return _position; }
    math::Vector2i get_window_size() const { return _size; }
    // return window flags
    unsigned get_window_flags() const;
    // return window internal representation
    void* get_window() const;
    // return whether application window is minimized
    bool is_minimized() const { return _minimized; }

protected:
    // window options and status
    int             _multisamples = 0;
    math::Vector2i  _size = {1, 1}, _position = {0, 0};
    Orientation     _orientation = Orientation::PORTRAIT;
    WindowOption    _options = WindowOption::NONE;
    DeviceContext*  _device = nullptr;
    int32_t         _system_frame_object = 0;
    bool            _minimized = false;

    // render states
    unsigned        _bound_fbo = 0;
    unsigned        _bound_program = 0, _bound_vbo = 0, _bound_ibo = 0;
    unsigned        _active_texunit = 0, _bound_texture = 0, _bound_textype = 0;
    BlendMode       _blend_mode;
    CullMode        _cull_mode;

    bool            _color_write;

    bool            _scissor_test;
    math::Rect2i    _scissor;

    bool            _depth_test;
    CompareMode     _depth_test_mode;
    bool            _depth_write;
    float           _depth_constant_bias;
    float           _depth_slope_scaled_bias;

    bool            _stencil_test;
    CompareMode     _stencil_test_mode;
    StencilOp       _stencil_pass_op;
    StencilOp       _stencil_fail_op;
    StencilOp       _stencil_zfail_op;
    unsigned        _stencil_ref;
    unsigned        _stencil_compare_mask;
    unsigned        _stencil_write_mask;
};

///
template<typename T, typename ... Args>
Device::spawn_return<T> Device::spawn(Args&& ... args)
{
    auto object = new (std::nothrow) T(*this);
    return std::shared_ptr<T>(object);
}

NS_FLOW2D_GFX_END

ENABLE_BITMASK_OPERATORS(lemon::graphics::WindowOption);
ENABLE_BITMASK_OPERATORS(lemon::graphics::ClearOption);
