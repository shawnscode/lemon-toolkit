// @date 2016/06/16
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <graphic/buffer.hpp>
#include <math/color.hpp>
#include <math/vector.hpp>
#include <math/rect.hpp>
#include <math/matrix.hpp>

NS_FLOW2D_BEGIN

struct CanvasPaint
{
    friend class Canvas;

protected:
    float       radius;
    float       feather;
    Vector2f    extent;
    Color       inner_color, outer_color;
    Matrix3f    transform;
    int         operations;

public:
    CanvasPaint& as_color(const Color& i);
    // parameters from-to specify the start and end coordinates
    // of the linear gradient, icol specifies the start color and ocol the end color.
    CanvasPaint& as_linear_gradient(const Vector2f& from, const Vector2f& to, const Color& i, const Color& o);
    CanvasPaint& as_radial_gradient(const Vector2f& center, float inr, float outr, const Color& i, const Color& o);
    CanvasPaint& with_gray();
    CanvasPaint& with_empty();
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
    CanvasPaint     fill;
    CanvasPaint     stroke;
    float       stroke_width;
    float       miter_limit;
    VGLineJoin  line_join;
    VGLineCap   line_cap;
    float       alpha;
    Rect2f      scissor;
    Matrix3f    transform;
};

// a vector graphic drawing utilities, which provides a similiar api and effect as html canvas.
// its could be used for prototype development.
struct Canvas
{
    static Canvas* create();

    void begin_frame(const Matrix3f&);
    void end_frame();

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
    void stroke(); // todo

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
    void set_fill_paint(const CanvasPaint&);

    void set_stroke_color(const Color&);
    void set_stroke_paint(const CanvasPaint&);
    void set_stroke_width(float);

    // sets how the end of the line (cap) is drawn
    void set_line_cap(VGLineCap);
    // set how sharp path corners are drawn
    void set_line_join(VGLineJoin);
    // sets the miter limit of the stroke style, miter limit controls when
    // a sharp corner is beveled.
    void set_miter_limit(float);
    void set_global_alpha(float);

    /// SCISSORING
    // scissoring allows you to clip the rendering into a rectangle.
    // set the current scissor rectangle
    void set_scissor(const Rect2f&);
    // intersects current scissor rectangle with the specified rectangle.
    void intersect_scissor(const Rect2f&);
    // reset and disable scissoring
    void reset_scissor();

    /// TRANSFORMATION
    void identity();
    void scale(const Vector2f&);
    void rotate(float radians, const Vector2f& anchor = {0.f, 0.f});
    void translate(const Vector2f&);
    void transform(const Matrix3f&);
    void set_transform(const Matrix3f&);

protected:
    bool initialize();
    void tesselate_bezier(const Vector2f&, const Vector2f&, const Vector2f&, const Vector2f&, int);

    Matrix3f                m_ortho;

    std::vector<VGState>    m_states;
    std::vector<uint16_t>   m_countors;
    std::vector<Vector2f>   m_points;

    VertexBuffer<Vector2f>  m_vertex_buffer;
    IndexBuffer<uint16_t>   m_index_buffer;

    Rid                     m_program;
};

NS_FLOW2D_END