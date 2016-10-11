// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>

#include <graphics/graphics.hpp>
#include <graphics/state.hpp>
#include <graphics/drawcall.hpp>

#include <core/subsystem.hpp>
#include <math/color.hpp>
#include <math/vector.hpp>
#include <math/matrix.hpp>

#include <mutex>
#include <vector>
#include <type_traits>

NS_LEMON_GRAPHICS_BEGIN

enum class ClearOption : uint8_t
{
    NONE    = 0x0,
    COLOR   = 0x1,
    DEPTH   = 0x2,
    STENCIL = 0x4
};

// Renderer provides sort-based draw call bucketing. this means that submission
// order doesn't necessarily match the rendering order, but on the low-level
// they will be sorted and ordered correctly.
struct RendererBackend;
struct VertexArrayObjectCache;
struct Renderer : public core::Subsystem
{
    SUBSYSTEM("lemon::graphics::Renderer")

    bool initialize() override;
    void dispose() override;

    // resource manipulation should be finished before frame render phase
    Program::ptr create_program(const char* vs, const char* ps);
    Texture::ptr create_texture(const void*, TextureFormat, TexturePixelFormat, unsigned, unsigned, MemoryUsage);
    VertexBuffer::ptr create_vertex_buffer(const void*, unsigned, const VertexLayout&, MemoryUsage);
    IndexBuffer::ptr create_index_buffer(const void*, unsigned, IndexElementFormat, MemoryUsage);

    // clear and start current frame
    bool begin_frame();
    // clear any or all of rendertarget, depth buffer and stencil buffer
    void clear(ClearOption, const math::Color& color = {0.f, 0.f, 0.f, 0.f}, float depth = 1.f, unsigned stencil = 0);
    // submit primitive for rendering
    void submit(RenderLayer, uint32_t depth, RenderDrawcall&);
    // flush all cached draw calls
    void flush();
    // end current frame and returns the count of drawcall
    unsigned end_frame();
    // returns true if we are under frame render phase
    bool is_frame_began() const { return _frame_began; }

protected:
    static bool drawcall_compare(const RenderDrawcall&, const RenderDrawcall&);

protected:
    friend struct WindowDevice;
    // restore OpenGL context and reinitialize state, requires an open window. returns true if successful
    bool restore(SDL_Window*);
    // release OpenGL context and handle the device lost of GPU resources
    void release();

protected:
    bool _frame_began;
    unsigned _frame_drawcall;

    RendererBackend* _backend;
    VertexArrayObjectCache* _vaocache;

    std::mutex _mutex;
    std::vector<RenderDrawcall> _drawcalls;
};

NS_LEMON_GRAPHICS_END

ENABLE_BITMASK_OPERATORS(lemon::graphics::ClearOption);
