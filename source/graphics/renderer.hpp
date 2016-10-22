// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>

#include <graphics/graphics.hpp>
#include <graphics/state.hpp>
#include <graphics/drawcall.hpp>

#include <codebase/type/type_traits.hpp>
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

    using resolver = std::function<void(Handle, void*)>;
    template<typename T, typename Impl, size_t Growth>
    bool register_graphics_object(const resolver& dtor = nullptr);

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

    std::vector<std::unique_ptr<std::mutex>> _object_mutexs;
    std::vector<resolver> _object_destructors;
    std::vector<resolver> _object_creators;
    std::vector<std::unique_ptr<IndexedMemoryPool>> _object_sets;

    std::mutex _mutex;
    std::vector<RenderDrawcall> _drawcalls;
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
    const auto index = TypeInfo::id<GraphicsObject, T>();
    ASSERT(index < _object_sets.size() && _object_sets[index] != nullptr,
        "trying to access un-registered graphics object %s.", typeid(T).name());

    auto& pool = _object_sets[index];
    Handle handle;

    {
        std::unique_lock<std::mutex> L(*_object_mutexs[index]);
        handle = pool->malloc();
    }

    if( !handle.is_valid() )
        return nullptr;

    auto object = static_cast<T*>(pool->get(handle));
    _object_creators[index](handle, object);
    if( !object->initialize(std::forward<Args>(args)...) )
    {
        {
            std::unique_lock<std::mutex> L(*_object_mutexs[index]);
            pool->free(handle);
        }
        return nullptr;
    }

    return object;
}

template<typename T> T* Renderer::get(Handle handle)
{
    const auto index = TypeInfo::id<GraphicsObject, T>();
    ASSERT(index < _object_sets.size() && _object_sets[index] != nullptr,
        "trying to access un-registered graphics object %s.", typeid(T).name());

    return static_cast<T*>(_object_sets[index]->get(handle));
}

template<typename T> void Renderer::free(T* object)
{
    if( object != nullptr )
        Renderer::free<T>(object->handle);
}

template<typename T> void Renderer::free(Handle handle)
{
    const auto index = TypeInfo::id<GraphicsObject, T>();
    ASSERT(index < _object_sets.size() && _object_sets[index] != nullptr,
        "trying to access un-registered graphics object %s.", typeid(T).name());

    auto object = _object_sets[index]->get(handle);
    if( object == nullptr )
        return;

    _object_destructors[index](handle, object);
    {
        std::unique_lock<std::mutex> L(*_object_mutexs[index]);
        _object_sets[index]->free(handle);
    }
}

template<typename T, typename Impl, size_t Growth>
bool Renderer::register_graphics_object(const resolver& dtor)
{
    const auto index = TypeInfo::id<GraphicsObject, T>();

    if( _object_sets.size() <= index )
    {
        _object_sets.resize(index+1);
        _object_destructors.resize(index+1);
        _object_creators.resize(index+1);
        _object_mutexs.resize(index+1);
    }

    _object_sets[index].reset(new (std::nothrow) IndexedMemoryPoolT<Impl, Growth>());
    _object_mutexs[index].reset(new std::mutex());

    _object_destructors[index] = dtor != nullptr ? dtor : [=](Handle, void* object)
    {
        static_cast<T*>(object)->~T();
    };

    _object_creators[index] = [=](Handle handle, void* object)
    {
        ::new(object) Impl(*this, handle);
    };

    return _object_sets[index] != nullptr;
}

NS_LEMON_GRAPHICS_END

ENABLE_BITMASK_OPERATORS(lemon::graphics::ClearOption);
