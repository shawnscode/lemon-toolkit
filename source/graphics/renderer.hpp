// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>

#include <graphics/graphics.hpp>
#include <graphics/state.hpp>
#include <graphics/drawcall.hpp>

#include <codebase/type_traits.hpp>
#include <codebase/memory/indexed_pool.hpp>
#include <core/core.hpp>
#include <math/color.hpp>
#include <math/vector.hpp>
#include <math/matrix.hpp>

#include <mutex>
#include <vector>
#include <type_traits>

NS_LEMON_GRAPHICS_BEGIN

struct EvtGraphicsDisposed {};
struct EvtGraphicsInitialized {};

namespace resource
{
    template<typename T> void free();
    template<typename T, typename ... Args> T* create(Args&& ...);
}

namespace details
{
    struct Resolver
    {
        virtual ~Resolver() {}
        virtual void* create() = 0;
        virtual void* get(Handle) = 0;
        virtual void free(Handle) = 0;
    };
}

// Renderer provides sort-based draw call bucketing. this means that submission
// order doesn't necessarily match the rendering order, but on the low-level
// they will be sorted and ordered correctly.
struct RendererBackend;
struct RenderStateCache;
struct Renderer : public core::Subsystem
{
    bool initialize() override;
    void dispose() override;

    // resource manipulation should be finished before frame render phase
    template<typename T, typename ... Args> T* create(Args&& ...);
    template<typename T> T* get(Handle);
    template<typename T> void free(T*);
    template<typename T> void free(Handle);

    void set_vertex_buffer();

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
    // check if we have valid window and OpenGL context
    bool is_device_lost() const;

protected:
    static bool drawcall_compare(const RenderDrawcall&, const RenderDrawcall&);

    template<typename T> details::Resolver* resolve();
    template<typename T, typename Impl, size_t Growth>
    bool attach(const std::function<void(Handle, void*)>& dtor = nullptr);

protected:
    friend struct WindowDevice;
    // restore OpenGL context and reinitialize state, requires an open window. returns true if successful
    bool restore(SDL_Window*);
    // release OpenGL context and handle the device lost of GPU resources
    void release();

protected:
    bool _frame_began;
    unsigned _frame_drawcall;

    RendererBackend* _backend = nullptr;
    RenderStateCache* _statecache = nullptr;

    std::mutex _mutex;
    std::vector<RenderDrawcall> _drawcalls;
    std::vector<std::unique_ptr<details::Resolver>> _resolvers;
};

// implementations of templates
namespace resource
{
    template<typename T> void free(T* object)
    {
        if( auto frontend = core::get_subsystem<Renderer>() )
            frontend->free(object);
    }

    template<typename T, typename ... Args> T* create(Args&& ... args)
    {
        if( auto frontend = core::get_subsystem<Renderer>() )
            return frontend->create<T>(std::forward<Args>(args)...);
        return nullptr;
    }
}

template<typename T, typename ... Args> T* Renderer::create(Args&& ... args)
{
    auto object = static_cast<T*>(resolve<T>()->create());
    if( object && object->initialize(std::forward<Args>(args)...) ) return object;
    if( object ) resolve<T>()->free(*object);
    return nullptr;
}

template<typename T> T* Renderer::get(Handle handle)
{
    return static_cast<T*>(resolve<T>()->get(handle));
}

template<typename T> void Renderer::free(T* object)
{
    if( object != nullptr )
        Renderer::free<T>(object->handle);
}

template<typename T> void Renderer::free(Handle handle)
{
    resolve<T>()->free(handle);
}

namespace details
{
    template<typename T, size_t Growth>
    struct ResolverT : public Resolver
    {
        ResolverT(Renderer& renderer, const std::function<void(Handle, void*)>& cb)
        : _renderer(renderer), _dispose(cb) {}

        ~ResolverT()
        {
            for( auto handle : _allocator )
            {
                if( auto object = _allocator.get_t(handle) )
                {
                    if( _dispose != nullptr) _dispose(handle, object);
                    object->~T();
                }
            }
        }

        void* create()
        {
            Handle handle;
            {
                std::unique_lock<std::mutex> L(_mutex);
                handle = _allocator.malloc();
            }

            if( auto object = _allocator.get(handle) )
            {
                ::new (object) T(handle);
                return object;
            }

            return nullptr;
        }

        void* get(Handle handle)
        {
            return _allocator.get(handle);
        }

        void free(Handle handle)
        {
            if( auto object = _allocator.get_t(handle) )
            {
                if( _dispose != nullptr) _dispose(handle, object);
                object->~T();

                {
                    std::unique_lock<std::mutex> L(_mutex);
                    _allocator.free(handle);
                }
            }
        }

    protected:
        Renderer& _renderer;
        std::mutex _mutex;
        std::function<void(Handle, void*)> _dispose;
        IndexedMemoryPoolT<T, Growth> _allocator;
    };
}

template<typename T> details::Resolver* Renderer::resolve()
{
    const auto index = TypeInfo::id<GraphicsObject, T>();
    ASSERT(index < _resolvers.size() && _resolvers[index] != nullptr,
        "trying to access un-registered graphics object %s.", typeid(T).name());
    return _resolvers[index].get();
}

template<typename T, typename Impl, size_t Growth>
bool Renderer::attach(const std::function<void(Handle, void*)>& cb)
{
    const auto index = TypeInfo::id<GraphicsObject, T>();

    if( _resolvers.size() <= index )
        _resolvers.resize(index+1);

    _resolvers[index].reset(new (std::nothrow) details::ResolverT<Impl, Growth>(*this, cb));
    return _resolvers[index] != nullptr;
}

NS_LEMON_GRAPHICS_END

ENABLE_BITMASK_OPERATORS(lemon::graphics::ClearOption);
