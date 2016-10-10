// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <graphics/graphics.hpp>
#include <graphics/state.hpp>

#include <core/subsystem.hpp>
#include <math/color.hpp>
#include <math/vector.hpp>
#include <math/matrix.hpp>

NS_LEMON_GRAPHICS_BEGIN

enum class ClearOption : uint8_t
{
    NONE    = 0x0,
    COLOR   = 0x1,
    DEPTH   = 0x2,
    STENCIL = 0x4
};

enum class RenderLayer : uint16_t
{
    BACKGROUND = 1000,
    GEOMETRY = 2000,
    ALPHATEST = 2500,
    TRANSPARENCY = 3000, // transparent geometreis will be renderred from-back-to-front
    OVERLAY = 4000
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

    // resource manipulation should be finished before frame render phase
    template<typename T, typename ... Args> std::shared_ptr<T> create(Args&& ... args)
    {
        auto object = new T();
        if( object && object->initialize(args...) )
            return std::shared_ptr<T>(object);

        if( object ) delete object;
        return nullptr;
    }

    // clear and start current frame
    bool begin_frame();
    // clear any or all of rendertarget, depth buffer and stencil buffer
    void clear(ClearOption, const math::Color& color = {0.f, 0.f, 0.f, 0.f}, float depth = 1.f, unsigned stencil = 0);
    // submit primitive for rendering
    void submit(RenderLayer, RenderState, Program::ptr, VertexBuffer::ptr, IndexBuffer::ptr, uint32_t depth, uint32_t start, uint32_t num);
    // flush all cached draw calls
    void flush();
    // finish current frame and flush all cached draw calls
    void end_frame();
    // returns true if we are under frame render phase
    bool is_frame_began() const { return _frame_began; }

protected:
    template<typename T> void free(T* object) {}

protected:
    friend struct WindowDevice;
    // restore OpenGL context and reinitialize state, requires an open window. returns true if successful
    bool restore(SDL_Window*);
    // release OpenGL context and handle the device lost of GPU resources
    void release();

protected:
    bool _frame_began;
    std::unique_ptr<RendererBackend> _backend;
    std::unique_ptr<VertexArrayObjectCache> _vaocache;
};

NS_LEMON_GRAPHICS_END
