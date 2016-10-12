// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/event.hpp>

#include <codebase/type/type_traits.hpp>
#include <codebase/handle.hpp>

#include <bitset>

// a fast, type-safe c++ entity component system based on [HandleX](https://github.com/alecthomas/entityx).
NS_LEMON_CORE_BEGIN

// represents an entity handle which is made up of components.
#define SET_CHUNK_SIZE(SIZE) static const size_t chunk_size = SIZE;
#define SET_COMPONENT_NAME(NAME) static const char* name = NAME;

struct Component
{
    constexpr static const size_t chunk_size = kEntPoolChunkSize;
    constexpr static const char* const name = "Component";

    const Handle object;
    operator Handle () const { return object; }

    // static dispatch instead of virtual table
    bool initialize() { return true; }
    void dispose() {}

protected:
    Component() : object({}) {}

    // component memory is always managed by entity manager.
    void operator delete(void*) = delete;
    void operator delete[](void*) = delete;
};

using ComponentMask = std::bitset<kEntMaxComponents>;

namespace ecs
{
    // an iterator over a specified view with components of the entites in an HandleManager.
    struct iterator : public std::iterator<std::forward_iterator_tag, Handle>
    {
        iterator(Handle, ComponentMask);

        iterator operator ++ (int);
        iterator& operator ++ ();

        bool operator == (const iterator&) const;
        bool operator != (const iterator&) const;
        Handle operator * () const;

    protected:
        ComponentMask _mask;
        Handle _current;
    };

    template<typename ... Args> struct view
    {
        view();

        iterator begin() const;
        iterator end() const;

        // visit entities with specialized components
        using visitor = std::function<void(Handle, Args& ...)>;
        void visit(const visitor&);

    protected:
        ComponentMask _mask;
    };
}

// spawn a new entity identifier
Handle spawn();
template<typename T, typename ... Args> Handle spawn_with(Args&&...);
// recycle entity identifier and its components
void recycle(Handle);
// returns true if the entity identifier is current alive
bool alive(Handle);
// returns the count of current alive entities
size_t size_of_world();
// dispose all the alive objects
void reset_world();

// assign a component to object, passing through component constructor arguments
template<typename T, typename ... Args> T* add_component(Handle, Args && ...);
// retrieve a component assigned to object
template<typename T> T* get_component(Handle);
template<typename ...Args> std::tuple<Args*...> get_components(Handle);
// remove a component from object
template<typename T> void remove_component(Handle);
// check if we have specified components
template<typename T> bool has_component(Handle);
template<typename ...Args> bool has_components(Handle);
// get bitmask representation of components
ComponentMask get_components_mask(Handle);
template<typename ...Args> ComponentMask get_components_mask();

// find entities that have all of the specified components, returns a incremental iterator
ecs::view<> find_entities();
template<typename ... T> ecs::view<T...> find_entities_with();

// dispatched events during the whole life of event manager
struct EvtHandleModified
{
    explicit EvtHandleModified(Handle object) : object(object) {}
    Handle object;
};

template<typename T> struct EvtComponentAdded
{
    explicit EvtComponentAdded(Handle object, T& component)
    : object(object), component(component) {}

    Handle object;
    T&     component;
};

template<typename T> struct EvtComponentRemoved
{
    explicit EvtComponentRemoved(Handle object, T& component)
    : object(object), component(component) {}

    Handle object;
    T&     component;
};

//
// implementation of traits
namespace internal
{
    // initialize component storage and type informations
    using destructor = std::function<void(Handle, void*)>;
    bool has_component_registered(TypeInfo::index_t);
    bool register_component(TypeInfo::index_t, size_t, size_t, const destructor&);

    //
    template<typename T> bool register_component()
    {
        auto destructor = [](Handle object, void* chunk)
        {
            T* component = static_cast<T*>(chunk);
            
            lemon::core::emit<EvtComponentRemoved<T>>(object, *component);
            lemon::core::emit<EvtHandleModified>(object);
 
            component->dispose();
            component->~T();
        };
 
        const auto id = TypeInfo::id<Component, T>();
        return register_component(id, sizeof(T), T::chunk_size, destructor);
    }

    //
    void* add_component(TypeInfo::index_t, Handle);
    void* get_component(TypeInfo::index_t, Handle);

    //
    void remove_component(TypeInfo::index_t, Handle);
    bool has_component(TypeInfo::index_t, Handle);

    // returns first available entity identifier with specified mask
    Handle find_first_available(ComponentMask);
    // returns next available entity identifier with specified mask
    Handle find_next_available(Handle, ComponentMask);

    //
    template<typename T> INLINE bool has_components(Handle object)
    {
        return has_component(TypeInfo::id<Component, T>(), object);
    }

    template<typename T1, typename T2, typename ... Args> INLINE bool has_components(Handle object)
    {
        return has_components<T1>(object) && has_components<T2, Args...>(object);
    }

    template<typename T> INLINE ComponentMask get_components_mask()
    {
        ComponentMask mask;
        mask.set(TypeInfo::id<Component, T>());
        return mask;
    }

    template<typename T1, typename T2, typename ... Args> INLINE ComponentMask get_components_mask()
    {
        return get_components_mask<T1>() | get_components_mask<T2, Args ...>();
    }

    namespace test_mem
    {
        size_t size(TypeInfo::index_t);
        size_t capacity(TypeInfo::index_t);
    }
}

// INCLUDED METHODS OF ENTITY VIEW AND ITERATOR
namespace ecs
{
    INLINE iterator::iterator(Handle index, ComponentMask mask)
    : _current(index), _mask(mask)
    {}

    INLINE iterator& iterator::operator ++ ()
    {
        _current = internal::find_next_available(_current, _mask);
        return *this;
    }

    INLINE iterator iterator::operator ++ (int dummy)
    {
        auto tmp = *this;
        (*this) ++;
        return tmp;
    }

    INLINE bool iterator::operator == (const iterator& rhs) const
    {
        return _current == rhs._current && _mask == rhs._mask;
    }

    INLINE bool iterator::operator != (const iterator& rhs) const
    {
        return !(*this == rhs);
    }

    INLINE Handle iterator::operator * () const
    {
        return _current;
    }

    template<typename ... Args> view<Args...>::view()
    : _mask(get_components_mask<Args...>())
    {}

    template<typename ... Args> INLINE iterator view<Args...>::begin() const
    {
        return iterator(internal::find_first_available(_mask), _mask);
    }

    template<typename ... Args> INLINE iterator view<Args...>::end() const
    {
        return iterator(Handle(), _mask);
    }

    template<typename ... Args> INLINE void view<Args...>::visit(const visitor& cb)
    {
        for( auto cursor : *this )
            cb(cursor, *::lemon::core::get_component<Args>(cursor) ...);
    }
}

template<typename T, typename ... Args> T* add_component(Handle object, Args && ... args)
{
    const auto id = TypeInfo::id<Component, T>();
    if( !internal::has_component_registered(id) )
        internal::register_component<T>();

    ASSERT( alive(object), "invalid operation: dead entity." );
    ASSERT( !has_components<T>(object), "invalid operation: duplicated component %s.", T::name );

    auto component = static_cast<T*>(internal::add_component(id, object));
    if( component == nullptr )
        return nullptr;

    ::new(component) T(std::forward<Args>(args)...);
    const_cast<Handle&>(component->object) = object;
    emit<EvtComponentAdded<T>>(object, *component);
    emit<EvtHandleModified>(object);

    if( !component->initialize() )
    {
        internal::remove_component(id, object);
        return nullptr;
    }
    return component;
}

template<typename T, typename ... Args> INLINE Handle spawn_with(Args && ... args)
{
    auto object = spawn();
    add_component<T>(object, std::forward<Args>(args)...);
    return object;
}

template<typename T> INLINE T* get_component(Handle object)
{
    const auto id = TypeInfo::id<Component, T>();
    return static_cast<T*>(internal::get_component(id, object));
}

template<typename ... T> INLINE std::tuple<T*...> get_components(Handle object)
{
    return std::make_tuple(get_component<T>(object)...);
}

template<typename T> INLINE void remove_component(Handle object)
{
    const auto id = TypeInfo::id<Component, T>();
    internal::remove_component(id, object);
}

template<typename T> INLINE bool has_component(Handle object)
{
    const auto id = TypeInfo::id<Component, T>();
    return internal::has_component(id, object);
}

template<> INLINE bool has_components(Handle object)
{
    return true;
}

template<typename ... Args> INLINE bool has_components(Handle object)
{
    return internal::has_components<Args ...>(object);
}

template<> INLINE ComponentMask get_components_mask()
{
    return ComponentMask();
}

template<typename ... Args> INLINE ComponentMask get_components_mask()
{
    return internal::get_components_mask<Args...>();
}

INLINE ecs::view<> find_entities()
{
    return ecs::view<>();
}

template<typename ... T> INLINE ecs::view<T...> find_entities_with()
{
    return ecs::view<T...>();
}

NS_LEMON_CORE_END
