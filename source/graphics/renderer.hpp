// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/vertex_attributes.hpp>
#include <codebase/handle.hpp>
#include <core/subsystem.hpp>
#include <math/vector.hpp>
#include <math/matrix.hpp>

NS_LEMON_GRAPHICS_BEGIN

// the expected usage pattern of the data store
enum class BufferUsage : uint8_t
{
    // the data store contents will be modified once and used at most a few times
    STREAM_DRAW = 0,
    // the data store contents will be modified once and used many times
    STATIC_DRAW,
    // the data store contents will be modified repeatedly and used many times
    DYNAMIC_DRAW
};

enum class IndexElementFormat : uint8_t
{
    UBYTE = 0,
    USHORT
};

enum class RenderLayer : uint16_t
{
    BACKGROUND = 1000,
    GEOMETRY = 2000,
    ALPHATEST = 2500,
    TRANSPARENCY = 3000,
    OVERLAY = 4000
};

struct RendererContext;
struct Renderer : public core::Subsystem
{
    SUBSYSTEM("lemon::graphics::Renderer")

    bool initialize() override;

    // create program with vertex/pixel shader
    Handle create_program(const char*, const char*);
    // free program handle
    void free_program(Handle);

    // create uniform buffer associated with program
    Handle create_uniform_buffer(Handle);
    // set shader uniform parameter for draw primitive
    void update_uniform_1f(Handle, const char*, const math::Vector<1, float>&);
    void update_uniform_2f(Handle, const char*, const math::Vector<2, float>&);
    void update_uniform_3f(Handle, const char*, const math::Vector<3, float>&);
    void update_uniform_4f(Handle, const char*, const math::Vector<4, float>&);
    void update_uniform_2fm(Handle, const char*, const math::Matrix<2, 2, float>&);
    void update_uniform_3fm(Handle, const char*, const math::Matrix<3, 3, float>&);
    void update_uniform_4fm(Handle, const char*, const math::Matrix<4, 4, float>&);
    void update_uniform_texture(Handle, const char*, Handle);
    // free uniform buffer handle
    void free_uniform_buffer(Handle);

    // create vertex buffer
    Handle create_vertex_buffer(const void*, size_t, const VertexAttributeLayout&, BufferUsage);
    // set all data in the vertex buffer, returns true if update successful
    bool update_vertex_buffer(Handle, const void*);
    // set a data range in the buffer, optionally discard data outside the range
    bool update_vertex_buffer(Handle, const void*, unsigned, unsigned, bool discard = false);
    // free vertex buffer handle
    void free_vertex_buffer(Handle);

    // create index buffer
    Handle create_index_buffer(const void*, size_t, IndexElementFormat, BufferUsage);
    // set all data in the index buffer, returns true if update successful
    bool update_index_buffer(Handle, const void*);
    // set a data range in the buffer, optionally discard data outside the range
    bool update_index_buffer(Handle, const void*, unsigned, unsigned, bool discard = false);
    // free index buffer handle
    void free_index_buffer(Handle);

    // clear and start current frame
    bool begin_frame();
    // clear any or all of rendertarget, depth buffer and stencil buffer
    void clear(ClearOption, const math::Color& color = {0.f, 0.f, 0.f, 0.f}, float depth = 1.f, unsigned stencil = 0);
    // submit preparations for draw primitive
    void set_vertex_buffer(Handle);
    void set_index_buffer(Handle);
    // submit primitive for rendering.
    void submit(RenderLayer, RenderState, Handle program, Handle uniform, uint32_t depth, uint32_t start, uint32_t num);
    // flush all cached draw calls
    void flush();
    // finish current frame and flush all cached draw calls
    void end_frame();

protected:
    friend struct WindowDevice;
    // restore OpenGL context and reinitialize state, requires an open window. returns true if successful
    bool restore(SDL_Window*);
    // release OpenGL context and handle the device lost of GPU resources
    void release();

protected:
    std::unique_ptr<RendererContext> _context;
};

NS_LEMON_GRAPHICS_END
