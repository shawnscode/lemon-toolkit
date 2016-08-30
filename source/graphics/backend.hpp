// @date 2016/07/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/render_state.hpp>
#include <core/context.hpp>
#include <math/color.hpp>

NS_LEMON_GRAPHICS_BEGIN

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

enum class RasterizationMode : uint8_t
{
    POINT,
    LINE,
    FILL
};

enum class Orientation : uint8_t
{
    LANDSCAPE_LEFT = 0,
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

// device events
struct EvtBackendLost {};
struct EvtBackendRestore {};

// the GraphicsObject class is the abstract base class for resources, shaders
struct GraphicsObject
{
    GraphicsObject(Backend& device);
    virtual ~GraphicsObject();

    void receive(const EvtBackendRestore&);
    void receive(const EvtBackendLost&);

    using receiver = std::function<void(GraphicsObject&)>;
    receiver on_device_restore = nullptr;
    receiver on_device_release = nullptr;

    virtual bool restore();
    virtual void release();

    unsigned get_graphic_object() const { return _object; }

protected:
    Backend&     _device;
    unsigned    _object = 0;
};

// graphics device subsystem. manages the window device, renedering state and gpu resources
struct BackendContext;
struct Backend : public core::Subsystem
{
    SUBSYSTEM("Graphics'Backend")

    Backend(core::Context& c) : Subsystem(c) {}
    virtual ~Backend() {}

    bool initialize() override;
    void dispose() override;

    // create window handle and main OpenGL context
    bool restore_window(int, int, int multisample = 1, WindowOption options = WindowOption::NONE);
    void release_window();
    // handle window messages, called from engine
    void process_window_message(void*);

    // set allowed screen orientations as a space-separated way
    void set_orientations(Orientation);
    Orientation get_orientations() const { return _orientation; }
    // set the initial left-upper position of window
    void set_window_position(const math::Vector2i&);
    math::Vector2i get_window_position() const { return _position; }
    // set the window size if fullscreen not enable
    void set_window_size(const math::Vector2i&);
    math::Vector2i get_window_size() const { return _size; }

    // return whether application window is minimized
    bool is_minimized() const { return _minimized; }
    // return window internal representation
    void* get_window_object() const;
    // return window flags
    unsigned get_window_flags() const;

    // restore OpenGL context and reinitialize state, requires an open window. returns true if successful
    bool restore_context();
    // release OpenGL context and handle the device lost of GPU resources
    void release_context();
    // reset all the graphics state to default
    void reset_cached_state();

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

    // specify whether front- or back-facing polygons can be culled
    void set_cull_face(bool, CullFace);
    // define front- and back-facing polygons
    void set_front_face(FrontFaceOrder);
    // define the scissor box
    void set_scissor_test(bool, const math::Rect2i& scissor = {{0, 0}, {0, 0}});
    // set front and back function and reference value for stencil testing
    void set_stencil_test(bool, CompareEquation, unsigned reference, unsigned mask);
    // set front and back stencil write actions
    void set_stencil_write(StencilWriteEquation sfail, StencilWriteEquation dpfail, StencilWriteEquation dppass, unsigned mask);
    // specify the value used for depth buffer comparisons
    void set_depth_test(bool, CompareEquation);
    // enable or disable writing into the depth buffer with bias
    void set_depth_write(bool, float slope_scaled = 0.f, float constant = 0.f);
    // set blending mode
    void set_color_blend(bool, BlendEquation, BlendFactor, BlendFactor);
    // enable and disable writing of frame buffer color components
    void set_color_write(ColorMask);
    // set the viewport
    void set_viewport(const math::Rect2i&);

    // prepare for draw call. setup corresponding frame/vertex buffer object
    void prepare_draw();
    // draw non-indexed geometry
    void draw(PrimitiveType, unsigned start, unsigned count);
    // draw indexed geometry
    void draw_index(PrimitiveType, unsigned start, unsigned count);

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

protected:
    // window options and status
    int             _multisamples = 0;
    math::Vector2i  _size = {1, 1}, _position = {0, 0};
    Orientation     _orientation = Orientation::PORTRAIT;
    WindowOption    _options = WindowOption::NONE;
    BackendContext*  _device = nullptr;
    int32_t         _system_frame_object = 0;
    bool            _minimized = false;

    // render states
    RenderState     _render_state;
    math::Rect2i    _viewport;

    unsigned        _bound_fbo = 0;
    unsigned        _bound_program = 0, _bound_vbo = 0, _bound_ibo = 0;
    unsigned        _active_texunit = 0, _bound_texture = 0, _bound_textype = 0;
};

///
template<typename T, typename ... Args>
Backend::spawn_return<T> Backend::spawn(Args&& ... args)
{
    auto object = new (std::nothrow) T(*this);
    return std::shared_ptr<T>(object);
}

NS_LEMON_GRAPHICS_END

ENABLE_BITMASK_OPERATORS(lemon::graphics::WindowOption);
ENABLE_BITMASK_OPERATORS(lemon::graphics::ClearOption);
