// @date 2016/06/16
// @author Mao Jingkai(oammix@gmail.com)

#include <flow2d/graphic/vg.hpp>
#include <flow2d/graphic/device.hpp>
#include <flow2d/graphic/vg_shader.inl>

extern "C" {
#include <tesselator.h>
}

NS_FLOW2D_BEGIN

#define COUNTOF(arr) (sizeof(arr) / sizeof(0[arr]))

static void reset_state(VGState& state)
{
    state.fill.as_color(Color::WHITE).with_empty();
    state.stroke.as_color(Color::BLACK).with_empty();

    state.stroke_width  = 1.f;
    state.miter_limit   = 10.f;
    state.line_join     = VGLineJoin::MITER;
    state.line_cap      = VGLineCap::BUTT;
    state.alpha         = 1.f;
    state.transform.identity();
}

enum PaintFirstPass
{
    PFP_SOLID_COLOR     = 0,
    PFP_LINEAR_GRADIENT = 1,
    PFP_RADIAL_GRADIENT = 2,
    PFP_TEXTURE         = 3,
};

enum PaintSecondPass
{
    PSP_EMPTY   = 0x00,
    PSP_GRAY    = 0x10,
};

VGPaint& VGPaint::as_color(const Color& i)
{
    memset(this, 0, sizeof(VGPaint));

    inner_color = i;

    operations = PFP_SOLID_COLOR | (operations & 0xF0);
    return *this;
}

VGPaint& VGPaint::as_linear_gradient(const Vector2f& from, const Vector2f& to, const Color& i, const Color& o)
{
    memset(this, 0, sizeof(VGPaint));

    Vector2f extent = to - from;
    float len = extent.length();

    if( len > 0.0001f )
    {
        extent /= len;
    }
    else
    {
        len = 1;
        extent = { 0.f, 1.f };
    }

    float inv_len = 1/len;

    transform = {
        inv_len, 0, -from[0] * inv_len,
        0, inv_len, -from[1] * inv_len,
    };

    radius = extent[0];
    feather = extent[1];

    inner_color = i;
    outer_color = o;

    operations = PFP_LINEAR_GRADIENT | (operations & 0xF0);
    return *this;
}

VGPaint& VGPaint::as_radial_gradient(const Vector2f& center, float inr, float outr, const Color& i, const Color& o)
{
    ASSERT( outr >= inr, "invalid parameters: outr should be always greater than inr." );
    memset(this, 0, sizeof(VGPaint));

    transform.identity();
    transform(0, 2) = -center[0];
    transform(1, 2) = -center[1];

    radius  = inr;
    feather = outr - inr;

    inner_color = i;
    outer_color = o;

    operations = PFP_RADIAL_GRADIENT | (operations & 0xF0);
    return *this;
}

VGPaint& VGPaint::with_gray()
{
    operations = PSP_GRAY | (operations & 0xF);
    return *this;
}

VGPaint& VGPaint::with_empty()
{
    operations = PSP_EMPTY | (operations & 0xF);
    return *this;
}

VGContext* VGContext::create()
{
    auto ctx = new (std::nothrow) VGContext();
    if( ctx && ctx->initialize() ) return ctx;
    if( ctx ) delete ctx;
    return ctx;
}

bool VGContext::initialize()
{
    m_states.resize(1);
    reset_state(m_states[m_states.size()-1]);

    auto& device = GraphicDevice::instance();
    m_program = device.create_shader(vg_vertex, vg_fragment,
        1,
        0, nullptr,
        COUNTOF(vg_uniforms), vg_uniforms);

    m_screen_size = kVector2fZero;

    return true;
}

void VGContext::begin_frame(const Vector2f& screen)
{
    m_screen_size = screen;
}

void VGContext::end_frame()
{
}

void VGContext::begin_path()
{
    m_points.clear();
    m_countors.clear();
}

void VGContext::move_to(const Vector2f& position)
{
    m_countors.push_back(m_points.size());
    m_points.push_back(position);
}

void VGContext::line_to(const Vector2f& position)
{
    if( m_countors.size() == 0 )
        return;

    m_points.push_back(position);
}

// reference: http://antigrain.com/research/adaptive_bezier/
void VGContext::tesselate_bezier(const Vector2f& from, const Vector2f& c1, const Vector2f& c2, const Vector2f& to, int level)
{
    if( level > 10 )
        return;

    auto p12    = (from + c1) * 0.5f;
    auto p23    = (c1 + c2) * 0.5f;
    auto p34    = (c2 + to) * 0.5f;

    auto d14    = to - from;
    auto d2     = std::abs( (c1[0] - to[0]) * d14[1] - (c1[1] - to[1]) * d14[0] );
    auto d3     = std::abs( (c2[0] - to[0]) * d14[1] - (c2[1] - to[1]) * d14[0] );

    // if curve is flat enough
    if( (d2 + d3) * (d2 + d3) < kGfxTesselateTolerance * dot(d14, d14) )
    {
        m_points.push_back(to);
        return;
    }

    auto p123   = (p12 + p23) * 0.5f;
    auto p234   = (p23 + p34) * 0.5f;
    auto p1234  = (p123 + p234) * 0.5f;

    tesselate_bezier(from, p12, p123, p1234, level+1);
    tesselate_bezier(p1234, p234, p34, to, level+1);
}

void VGContext::bezier_curve_to(const Vector2f& to, const Vector2f& c1, const Vector2f& c2)
{
    if( m_countors.size() == 0 )
        return;

    tesselate_bezier(m_points.back(), c1, c2, to, 0);
}

void VGContext::quadratic_curve_to(const Vector2f& to, const Vector2f& c)
{
    if( m_countors.size() == 0 )
        return;

    auto from = m_points.back();
    auto c1 = from  + 2.0f/3.0f * (c-from);
    auto c2 = to    + 2.0f/3.0f * (c-to);
    bezier_curve_to(to, c1, c2);
}

void VGContext::close_path()
{
    if( m_countors.size() == 0 )
        return;

    move_to(m_points[m_countors.back()]);
}

void VGContext::fill()
{
    if( m_countors.size() == 0 || m_points.size() == 0 )
        return;

    auto T = tessNewTess(nullptr);
    if( !T )
    {
        LOGW("failed to create tesselator.");
        return;
    }

    for( size_t i=m_countors.size(); i>=1; i-- )
    {
        auto start_pos = m_countors[i-1];
        auto end_pos = i == m_countors.size() ? m_points.size() : m_countors[i];
        tessAddContour(T, 2, m_points.data()+start_pos, sizeof(Vector2f), end_pos-start_pos);
    }

    const int nvp = 6;

    if( !tessTesselate(T, TESS_WINDING_NONZERO, TESS_POLYGONS, nvp, 2, 0) )
    {
        tessDeleteTess(T);
        LOGW("failed to tesselate this vector graphics.");
        return;
    }

    auto nv         = tessGetVertexCount(T);
    auto vertices   = tessGetVertices(T);
    auto ne         = tessGetElementCount(T);
    auto elements   = tessGetElements(T);

    m_vertex_buffer.data.resize(nv);
    for( size_t i=0; i<nv; i++ )
        m_vertex_buffer.data[i] = {vertices[i*2], vertices[i*2+1]};

    m_index_buffer.data.clear();
    for( size_t i=0; i<ne; i++ )
    {
        const int* p = &elements[i*nvp];
        for( size_t j=2; j < nvp && p[j] != TESS_UNDEF; j++ )
        {
            m_index_buffer.data.push_back(p[0]);
            m_index_buffer.data.push_back(p[j-1]);
            m_index_buffer.data.push_back(p[j]);
        }
    }

    tessDeleteTess(T);

    m_index_buffer.force_update();
    m_vertex_buffer.force_update();

    Matrix3f transform = make_ortho(0.f, m_screen_size[0], 0.f, m_screen_size[1]) * m_states.back().transform;

    auto& device = GraphicDevice::instance();
    device.bind_shader(m_program);
    device.bind_vertex_buffer(0, m_vertex_buffer.id(), 2, ElementFormat::FLOAT, m_vertex_buffer.stride(), 0);
    device.bind_index_buffer(m_index_buffer.id(), ElementFormat::UNSIGNED_SHORT, m_index_buffer.stride(), 0);

    auto& paint = m_states.back().fill;
    // transform, convert to colum major
    device.bind_uniform(0, UniformFormat::MATRIX_F33, &transpose(transform)[0]);
    // paint transform
    device.bind_uniform(1, UniformFormat::MATRIX_F33, &transpose(paint.transform)[0]);
    // inner color
    device.bind_uniform(2, UniformFormat::VECTOR_F4, (float*)&paint.inner_color);
    // outer color
    device.bind_uniform(3, UniformFormat::VECTOR_F4, (float*)&paint.outer_color);
    // radius
    device.bind_uniform(4, UniformFormat::FLOAT1, &paint.radius);
    // feather
    device.bind_uniform(5, UniformFormat::FLOAT1, &paint.feather);
    // operations
    device.bind_uniform(6, UniformFormat::INT1, (float*)&paint.operations);

    LOGW("%d", paint.operations);

    device.draw(DrawMode::TRIANGLE, 0, m_index_buffer.data.size());
}

void VGContext::stroke()
{
}

void VGContext::save()
{
    m_states.resize(m_states.size()+1);
    m_states[m_states.size()-1] = m_states[m_states.size()-2];
}

void VGContext::restore()
{
    if( m_states.size() > 1 ) m_states.pop_back();
    else reset_state(m_states[0]);
}

void VGContext::reset()
{
    reset_state(m_states.back());
}

void VGContext::set_stroke_color(const Color& color)
{
    m_states.back().stroke.as_color(color).with_empty();
}

void VGContext::set_stroke_paint(const VGPaint& paint)
{
    m_states.back().stroke = paint;
    m_states.back().stroke.transform *= m_states.back().transform;
}

void VGContext::set_stroke_width(float width)
{
    m_states.back().stroke_width = width;
}

void VGContext::set_fill_color(const Color& color)
{
    m_states.back().fill.as_color(color).with_empty();
}

void VGContext::set_fill_paint(const VGPaint& paint)
{
    m_states.back().fill = paint;
    m_states.back().fill.transform *= m_states.back().transform;
}

void VGContext::set_line_cap(VGLineCap cap)
{
    m_states.back().line_cap = cap;
}

void VGContext::set_line_join(VGLineJoin join)
{
    m_states.back().line_join = join;
}

void VGContext::set_miter_limit(float miter)
{
    m_states.back().miter_limit = miter;
}

void VGContext::set_global_alpha(float alpha)
{
    m_states.back().alpha = alpha;
}

void VGContext::scale(const Vector2f& scale)
{
    transform(hlift(make_scale(scale)));
}

void VGContext::rotate(float)
{
}

void VGContext::translate(const Vector2f& translation)
{
    transform(make_translation(translation));
}

void VGContext::transform(const Matrix3f& transform)
{
    m_states.back().transform = transform * m_states.back().transform;
}

void VGContext::set_transform(const Matrix3f& transform)
{
    m_states.back().transform = transform;
}

NS_FLOW2D_END