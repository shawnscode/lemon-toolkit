// @date 2016/06/16
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <flow2d/forward.hpp>
#include <flow2d/graphic/buffer.hpp>
#include <flow2d/math/color.hpp>
#include <flow2d/math/vector.hpp>
#include <flow2d/math/rect.hpp>
#include <flow2d/math/matrix.hpp>

NS_FLOW2D_BEGIN

struct VGPaint
{
    float       radius;
    float       feather;
    Vector2f    extent;
    Color       inner_color, outer_color;
    Matrix3f    transform;

    // creates and returns a linear gradient.
    // parameters from-to specify the start and end coordinates
    // of the linear gradient, icol specifies the start color and ocol the end color.
    static VGPaint liner_gradient(const Vector2f& from, const Vector2f& to, const Color& i, const Color& o);
    static VGPaint radial_gradient(const Vector2f& center, float inr, float outr, const Color& i, const Color& o);
};


enum class VGWinding : uint8_t
{
    CCW = 1,
    CW  = 2,
};

enum class VGLineCap : uint8_t
{
    BUTT    = 1,
    ROUND   = 2,
    SQUARE  = 3,
};

enum class VGLineJoin : uint8_t
{
    MITER   = 1,
    ROUND   = 2,
    BEVEL   = 3,
};

enum VGAlign
{
    ALIGN_LEFT      = 0x001, // default, align horizontally to left
    ALIGN_CENTER    = 0x002,
    ALIGN_RIGHT     = 0x004,

    ALIGN_TOP       = 0x008,
    ALIGN_MIDDLE    = 0x010,
    ALIGN_BOTTOM    = 0x020,
    ALIGN_BASELINE  = 0x040, // default, align vertically to baseline
};

struct VGState
{
    VGPaint     fill;
    VGPaint     stroke;
    float       stroke_width;
    float       miter_limit;
    VGLineJoin  line_join;
    VGLineCap   line_cap;
    float       alpha;
    Matrix3f    transform;
};

struct VGContext
{
    static VGContext* create();

    // void begin_frame(size_t width, size_t height, float device_pixel_ratio);
    // void end_frame();

    /// PATH
    // begins a path, or resets the current path
    void begin_path();
    // creates a path from the current point back to the starting point
    void close_path();
    // moves the path to the specified point in the canvas, without creating a line
    void move_to(const Vector2f&);
    // adds a new point and creates a line to that point from the last specified
    // point in the canvas
    void line_to(const Vector2f&);
    // creates a cubic Bézier curve
    void bezier_curve_to(const Vector2f&, const Vector2f&, const Vector2f&);
    // creates a quadratic Bézier curve
    void quadratic_curve_to(const Vector2f&, const Vector2f&);

    /// DRAWING
    // fills the current drawing path
    void fill();
    // actually draws the path you have defined
    void stroke();

    /// STATE
    // saves the state of the current context
    void save();
    // returns previously saved path state and attributes
    void restore();
    // resets current render state to default values.
    void reset();

    // fill and stroke render style can be either a solid color or a
    // paint which is a gradient or a pattern. solid color is simply
    // defined as a color value, different kinds of paints can be created
    void set_fill_color(const Color&);
    void set_fill_paint(const VGPaint&);

    void set_stroke_color(const Color&);
    void set_stroke_paint(const VGPaint&);
    void set_stroke_width(float);

    // sets how the end of the line (cap) is drawn
    void set_line_cap(VGLineCap);
    // set how sharp path corners are drawn
    void set_line_join(VGLineJoin);
    // sets the miter limit of the stroke style, miter limit controls when
    // a sharp corner is beveled.
    void set_miter_limit(float);
    void set_global_alpha(float);

    /// TRANSFORMATION
    void scale(const Vector2f&);
    void rotate(float); // in rads
    void translate(const Vector2f&);
    void transform(const Matrix3f&);
    void set_transform(const Matrix3f&);

protected:
    bool initialize();
    void tesselate_bezier(const Vector2f&, const Vector2f&, const Vector2f&, const Vector2f&, int);

    std::vector<VGState>    m_states;
    std::vector<uint16_t>   m_countors;
    std::vector<Vector2f>   m_points;

    VertexBuffer<Vector2f>  m_vertex_buffer;
    IndexBuffer<uint16_t>   m_index_buffer;

    Rid                     m_program;
};

NS_FLOW2D_END