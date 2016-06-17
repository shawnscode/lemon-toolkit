// @date 2016/05/31
// @author Mao Jingkai(oammix@gmail.com)

#include <flow2d/graphic/render.hpp>

NS_FLOW2D_BEGIN

static GraphicRender* s_render = nullptr;

GraphicRender& GraphicRender::instance()
{
    ENSURE( s_render != nullptr );
    return *s_render;
}

bool GraphicRender::initialize(float width, float height)
{
    ENSURE( s_render == nullptr );

    s_render = new (std::nothrow) GraphicRender();
    if( s_render == nullptr ) return false;

    auto& device = GraphicDevice::instance();

    s_render->set_resolution(width, height);

    s_render->m_verts = device.create_buffer(
        RenderObject::VERTEX_BUFFER,
        nullptr,
        kGfxMaxBatchVertices*sizeof(VertexPack));

    s_render->m_indices = device.create_buffer(
        RenderObject::INDEX_BUFFER,
        nullptr,
        kGfxMaxBatchVertices*sizeof(uint16_t)*2);

    s_render->m_vused = s_render->m_iused = 0;
    s_render->m_blend_src = BlendFunc::ONE;
    s_render->m_blend_dst = BlendFunc::ONE_MINUS_SRC_ALPHA;

    s_render->m_current_program = GraphicDevice::INVALID;
    for( auto i=0; i<kGfxMaxTextures; i++ )
        s_render->m_textures[i] = GraphicDevice::INVALID;
    return true;
}

void GraphicRender::set_resolution(float width, float height)
{
    m_resolution_width = width;
    m_resolution_height = height;
}

bool GraphicRender::is_visible(float x, float y) const
{
    return x >= 0 && x < m_resolution_width && y >= 0 && y < m_resolution_height;
}

void GraphicRender::scissor_push(float x, float y, float width, float height)
{
    m_scissors.push_back(Rect2f{ x, y, width, height});
    auto& device = GraphicDevice::instance();
    device.set_scissor(true, x, y, width, height);
}

void GraphicRender::scissor_pop()
{
    if( m_scissors.size() > 0 )
        m_scissors.pop_back();

    auto& device = GraphicDevice::instance();
    if( m_scissors.size() > 1 )
    {
        auto& scissor = m_scissors.back();
        auto position = scissor.position();
        auto size = scissor.size();
        device.set_scissor(true, position[0], position[1], size[0], size[1]);
    }
    else
    {
        device.set_scissor(false, 0, 0, 0, 0);
    }
}

Rid GraphicRender::compile(const char* vs, const char* fs,
    size_t texture_n, const char** textures, size_t uniform_n, const char** uniforms)
{
    auto& device = GraphicDevice::instance();
    return device.create_shader(vs, fs, 4, texture_n, textures, uniform_n, uniforms);
}

void GraphicRender::set_blend(BlendFunc src, BlendFunc dst)
{
    if( m_blend_src != src || m_blend_dst != dst )
        flush();

    m_blend_src = src;
    m_blend_dst = dst;
}

void GraphicRender::set_texture(int index, Rid rid)
{
    ENSURE( index >= 0 && index < kGfxMaxTextures );
    if( m_textures[index] != rid )
        flush();

    m_textures[index] = rid;
}

void GraphicRender::set_uniform(int index, UniformFormat format, const float* v)
{
    flush();

    auto& device = GraphicDevice::instance();
    device.bind_uniform(index, format, v);
}

void GraphicRender::set_program(Rid rid)
{
    if( m_current_program != rid )
        flush();

    m_current_program = rid;
}

static uint16_t clamp_float_to_u16(float t)
{
    if( t > 1.0f ) return std::numeric_limits<uint16_t>::max();
    if( t <= 0.0f ) return 0;
    return static_cast<uint16_t>(t * std::numeric_limits<uint16_t>::max());
}

void GraphicRender::submit(size_t vsize, const Vertex2f* vertices, size_t isize, const uint16_t* indices,
    const Color& diffuse, const Color& additive)
{
    if( vsize <= 0 || isize <= 0 )
        return;

    ENSURE( vsize < kGfxMaxBatchVertices && isize < kGfxMaxBatchVertices*2 );
    if( m_vused >= (kGfxMaxBatchVertices-vsize) || m_iused >= (kGfxMaxBatchVertices*2-isize) )
        flush();

    for( auto i=0; i<isize; i++ )
        m_ibuffer[m_iused++] = m_vused + indices[i];

    for( auto i=0; i<vsize; i++ )
    {
        auto& v = vertices[i];
        m_vbuffer[m_vused++] = GraphicRender::VertexPack {
            v.position[0],
            v.position[1],
            // v.texcoord[0],
            // v.texcoord[1],
            clamp_float_to_u16(v.texcoord[0]),
            clamp_float_to_u16(v.texcoord[1]),
            diffuse.to_uint32(),
            additive.to_uint32()
        };
    }
}

void GraphicRender::submit(const Vertex2f& p1, const Vertex2f& p2, const Vertex2f& p3, const Vertex2f& p4,
    const Color& diffuse, const Color& additive)
{
    Vertex2f buf[4] = { p1, p2, p3, p4 };
    uint16_t indices[6] = { 0, 1, 2, 0, 2, 3 };
    submit(4, buf, 6, indices, diffuse, additive);
}

void GraphicRender::flush()
{
    if( m_iused > 0 && m_current_program != GraphicDevice::INVALID )
    {
        auto& device = GraphicDevice::instance();
        device.set_blend(m_blend_src, m_blend_dst);
        device.bind_shader(m_current_program);

        device.update_buffer(RenderObject::VERTEX_BUFFER, m_verts, m_vbuffer, m_vused*sizeof(VertexPack));
        device.update_buffer(RenderObject::INDEX_BUFFER, m_indices, m_ibuffer, m_iused*sizeof(uint16_t));

        const auto stride = sizeof(VertexPack);
        auto offset = 0;

        device.bind_index_buffer(m_indices, ElementFormat::UNSIGNED_SHORT, 0, 0);
        // position
        device.bind_vertex_buffer(0, m_verts, 2, ElementFormat::FLOAT, stride, offset);
        // texcoord
        offset += sizeof(float) * 2;
        device.bind_vertex_buffer(1, m_verts, 2, ElementFormat::UNSIGNED_SHORT, stride, offset, true);
        // diffuse color
        offset += sizeof(uint16_t) * 2;
        device.bind_vertex_buffer(2, m_verts, 4, ElementFormat::UNSIGNED_BYTE, stride, offset, true);
        // additive color
        offset += sizeof(uint8_t) * 4;
        device.bind_vertex_buffer(3, m_verts, 4, ElementFormat::UNSIGNED_BYTE, stride, offset, true);

        for( auto i=0; i<kGfxMaxTextures; i++ )
        {
            if( m_textures[i] != GraphicDevice::INVALID )
                device.bind_texture(i, m_textures[i]);
        }

        device.draw(DrawMode::TRIANGLE, 0, m_iused);
    }

    m_vused = m_iused = 0;
}

NS_FLOW2D_END