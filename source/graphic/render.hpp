// @date 2016/05/31
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <graphic/device.hpp>

#include <math/vector.hpp>
#include <math/rect.hpp>
#include <math/color.hpp>

NS_FLOW2D_BEGIN

// REFACTOR THIS TO SUPPORT MATERIAL BASED BATCHING

struct Vertex2f
{
    Vertex2f()
    {}

    Vertex2f(std::initializer_list<float> values)
    {
        auto size = values.size();
        auto cursor = values.begin();

        if( size >= 1 ) position[0] = *cursor++;
        if( size >= 2 ) position[1] = *cursor++;
        if( size >= 3 ) texcoord[0] = *cursor++;
        if( size >= 4 ) texcoord[1] = *cursor++;
    }

    Vertex2f(const Vector2f& position, const Vector2f& texcoord)
    : position(position), texcoord(texcoord)
    {}

    Vector2f position;
    Vector2f texcoord;
};

// a buffered graphic renderer with fixed input.
struct GraphicRender
{
    struct VertexPack
    {
        float    vx, vy; // position
        uint16_t tx, ty; // texcoord
        uint32_t diffuse, additive;
    };

    static GraphicRender& instance();
    static bool initialize(float width, float height);
    static void dispose();

    void set_resolution(float width, float height);
    bool is_visible(float x, float y) const;

    void scissor_push(float x, float y, float width, float height);
    void scissor_pop();

    // position, texcoord, diffuse, addtive
    Rid  compile(const char* vs, const char* fs, size_t tn, const char** textures, size_t un, const char** uniforms);
    void set_program(Rid rid);
    void set_blend(BlendFunc src, BlendFunc dst);
    void set_texture(int index, Rid rid);
    void set_uniform(int index, UniformFormat format, const float* v);

    void submit(size_t vsize, const Vertex2f* vertices, size_t isize, const uint16_t* indices,
        const Color& diffuse = Color::WHITE, const Color& additive = Color::TRANSPARENT);
    void submit(const Vertex2f& v1, const Vertex2f& v2, const Vertex2f& v3, const Vertex2f& v4,
        const Color& diffuse = Color::WHITE, const Color& additive = Color::TRANSPARENT);
    void flush();

protected:
    float       m_resolution_width, m_resolution_height;
    Rid         m_verts, m_indices;

    VertexPack  m_vbuffer[kGfxMaxBatchVertices];
    uint16_t    m_ibuffer[kGfxMaxBatchVertices*2];
    size_t      m_vused, m_iused;

    BlendFunc   m_blend_src, m_blend_dst;
    Rid         m_textures[kGfxMaxTextures];
    Rid         m_current_program;

    std::vector<Rect2f> m_scissors;
};

NS_FLOW2D_END