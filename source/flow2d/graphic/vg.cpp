// @date 2016/06/16
// @author Mao Jingkai(oammix@gmail.com)

#include <flow2d/graphic/vg.hpp>
#include <flow2d/graphic/device.hpp>
#include <tesselator.h>

NS_FLOW2D_BEGIN

#define COUNTOF(arr) (sizeof(arr) / sizeof(0[arr]))

static void set_paint(VGPaint& paint, const Color& color)
{
    paint.radius = 0.f;
    paint.feather = 1.f;
    paint.inner = color;
    paint.outer = color;
    paint.transform.identity();
}

static void reset_state(VGState& state)
{
    set_paint(state.fill, Color::WHITE);
    set_paint(state.stroke, Color::BLACK);

    state.stroke_width  = 1.f;
    state.miter_limit   = 10.f;
    state.line_join     = VGLineJoin::MITER;
    state.line_cap      = VGLineCap::BUTT;
    state.alpha         = 1.f;
    state.transform.identity();
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
    return true;
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

    auto& device = GraphicDevice::instance();
    device.bind_shader(m_program);
    device.bind_vertex_buffer(0, m_vertex_buffer.id(), 2, ElementFormat::FLOAT, m_vertex_buffer.stride(), 0);
    device.bind_index_buffer(m_index_buffer.id(), ElementFormat::UNSIGNED_SHORT, m_index_buffer.stride(), 0);
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
    set_paint(m_states.back().stroke, color);
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
    set_paint(m_states.back().fill, color);
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
    // m_states.back().transform *= make_scale(scale);
}

void VGContext::rotate(float)
{

}

void VGContext::translate(const Vector2f& translation)
{
    // m_states.back().transform *= make_translation(translation);
}

void VGContext::transform(const Matrix3f& transform)
{
    // m_states.back().transform *= transform;
}

void VGContext::set_transform(const Matrix3f& transform)
{
    m_states.back().transform = transform;
}

NS_FLOW2D_END