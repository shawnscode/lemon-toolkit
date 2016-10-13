// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>

#include <graphics/graphics.hpp>
#include <graphics/state.hpp>
#include <graphics/drawcall.hpp>

#include <codebase/type/type_traits.hpp>
#include <codebase/memory/indexed_pool.hpp>
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
    Renderer();
    virtual ~Renderer();

    bool initialize() override;
    void dispose() override;

    // resource manipulation should be finished before frame render phase
    template<typename T, typename ... Args> Handle create(Args&& ...);
    template<typename T> T* get(Handle);
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

    using destructor = std::function<void(void*)>;
    template<typename T, typename Impl, size_t Growth>
    bool register_graphics_object(const destructor& dtor = nullptr);


protected:
    friend struct WindowDevice;
    // restore OpenGL context and reinitialize state, requires an open window. returns true if successful
    bool restore(SDL_Window*);
    // release OpenGL context and handle the device lost of GPU resources
    void release();

protected:
    bool _frame_began;
    unsigned _frame_drawcall;

    std::unique_ptr<RendererBackend> _backend;
    std::unique_ptr<VertexArrayObjectCache> _vaocache;

    std::vector<std::unique_ptr<std::mutex>> _object_mutexs;
    std::vector<std::function<void(void*)>> _object_destructors;
    std::vector<std::function<void(void*)>> _object_creators;
    std::vector<std::unique_ptr<IndexedMemoryPool>> _object_sets;

    std::mutex _mutex;
    std::vector<RenderDrawcall> _drawcalls;
};

// implementations of templates
template<typename T, typename ... Args> Handle Renderer::create(Args&& ... args)
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

    auto object = static_cast<T*>(pool->get(handle));
    if( object == nullptr )
    {
        return Handle();
    }

    _object_creators[index](object);
    if( !object->initialize(std::forward<Args>(args)...) )
    {
        {
            std::unique_lock<std::mutex> L(*_object_mutexs[index]);
            pool->free(handle);
        }
        return Handle();
    }

    return handle;
}

template<typename T> T* Renderer::get(Handle handle)
{
    const auto index = TypeInfo::id<GraphicsObject, T>();
    ASSERT(index < _object_sets.size() && _object_sets[index] != nullptr,
        "trying to access un-registered graphics object %s.", typeid(T).name());

    return static_cast<T*>(_object_sets[index]->get(handle));
}

template<typename T> void Renderer::free(Handle handle)
{
    const auto index = TypeInfo::id<GraphicsObject, T>();
    ASSERT(index < _object_sets.size() && _object_sets[index] != nullptr,
        "trying to access un-registered graphics object %s.", typeid(T).name());

    auto object = _object_sets[index]->get(handle);
    if( object == nullptr )
        return;

    _object_destructors[index](object);
    {
        std::unique_lock<std::mutex> L(*_object_mutexs[index]);
        _object_sets[index]->free(handle);
    }
}

template<typename T, typename Impl, size_t Growth>
bool Renderer::register_graphics_object(const destructor& dtor)
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

    _object_destructors[index] = dtor != nullptr ? dtor : [=](void* object)
    {
        static_cast<T*>(object)->~T();
    };

    _object_creators[index] = [=](void* object)
    {
        ::new(object) Impl(*this);
    };

    return _object_sets[index] != nullptr;
}

NS_LEMON_GRAPHICS_END

ENABLE_BITMASK_OPERATORS(lemon::graphics::ClearOption);
