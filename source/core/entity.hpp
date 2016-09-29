// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/event.hpp>
#include <codebase/type/typeinfo.hpp>

#include <bitset>

// a fast, type-safe c++ entity component system based on [EntityX](https://github.com/alecthomas/entityx).
NS_LEMON_CORE_BEGIN

// represents an entity handle which is made up of components.
struct Entity
{
    using index_type = uint16_t; // supports 60k+ entities and versions.
    const static index_type invalid = std::numeric_limits<index_type>::max();

    Entity() = default;
    Entity(const Entity&) = default;
    Entity(index_type index, index_type version) : _index(index), _version(version) {}
    Entity& operator = (const Entity&) = default;

    bool operator == (const Entity&) const;
    bool operator != (const Entity&) const;
    bool operator <  (const Entity&) const;

    index_type get_index() const;
    index_type get_version() const;

    // invalidate entity handle, disassociating it from entity manager.
    void invalidate();

private:
    index_type _index = invalid;
    index_type _version = invalid;
};

#define SET_CHUNK_SIZE(SIZE) static const size_t chunk_size = SIZE;
#define SET_COMPONENT_NAME(NAME) static const char* name = NAME;

struct Component
{
    constexpr static const size_t chunk_size = kEntPoolChunkSize;
    constexpr static const char* const name = "Component";

    const Entity object;
    operator Entity () const { return object; }

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
    // an iterator over a specified view with components of the entites in an EntityManager.
    struct iterator : public std::iterator<std::forward_iterator_tag, Entity>
    {
        iterator(Entity::index_type, ComponentMask);

        iterator operator ++ (int);
        iterator& operator ++ ();

        bool operator == (const iterator&) const;
        bool operator != (const iterator&) const;
        Entity operator * () const;

    protected:
        ComponentMask       _mask;
        Entity::index_type  _index;
    };

    template<typename ... Args> struct view
    {
        view();

        iterator begin() const;
        iterator end() const;

        // visit entities with specialized components
        using visitor = std::function<void(Entity, Args& ...)>;
        void visit(const visitor&);

    protected:
        ComponentMask _mask;
    };
}

// spawn a new entity identifier
Entity spawn();
template<typename T, typename ... Args> Entity spawn_with(Args&&...);
// recycle entity identifier and its components
void recycle(Entity);
// returns true if the entity identifier is current alive
bool alive(Entity);
// returns the count of current alive entities
size_t size_of_world();
// dispose all the alive objects
void reset_world();

// assign a component to object, passing through component constructor arguments
template<typename T, typename ... Args> T* add_component(Entity, Args && ...);
// retrieve a component assigned to object
template<typename T> T* get_component(Entity);
template<typename ...Args> std::tuple<Args*...> get_components(Entity);
// remove a component from object
template<typename T> void remove_component(Entity);
// check if we have specified components
template<typename T> bool has_component(Entity);
template<typename ...Args> bool has_components(Entity);
// get bitmask representation of components
ComponentMask get_components_mask(Entity);
template<typename ...Args> ComponentMask get_components_mask();

// find entities that have all of the specified components, returns a incremental iterator
ecs::view<> find_entities();
template<typename ... T> ecs::view<T...> find_entities_with();

// dispatched events during the whole life of event manager
struct EvtEntityModified
{
    explicit EvtEntityModified(Entity object) : object(object) {}
    Entity object;
};

template<typename T> struct EvtComponentAdded
{
    explicit EvtComponentAdded(Entity object, T& component)
    : object(object), component(component) {}

    Entity object;
    T&     component;
};

template<typename T> struct EvtComponentRemoved
{
    explicit EvtComponentRemoved(Entity object, T& component)
    : object(object), component(component) {}

    Entity object;
    T&     component;
};

//
// implementation of traits
namespace internal
{
    // initialize component storage and type informations
    using destructor = std::function<void(Entity, void*)>;
    bool has_component_registered(TypeInfo::index_type);
    bool register_component(TypeInfo::index_type, size_t, size_t, const destructor&);

    //
    template<typename T> bool register_component()
   {
        auto destructor = [](Entity object, void* chunk)
        {
            T* component = static_cast<T*>(chunk);
            
            lemon::core::emit<EvtComponentRemoved<T>>(object, *component);
            lemon::core::emit<EvtEntityModified>(object);
 
            component->dispose();
            component->~T();
        };
 
        const auto id = TypeInfo::id<Component, T>();
        return register_component(id, sizeof(T), T::chunk_size, destructor);
   }

    //
    void* add_component(TypeInfo::index_type, Entity);
    void* get_component(TypeInfo::index_type, Entity);

    //
    void remove_component(TypeInfo::index_type, Entity);
    bool has_component(TypeInfo::index_type, Entity);

    //
    Entity::index_type find_next_available(Entity::index_type, ComponentMask, bool self = false);
    Entity get(Entity::index_type);

    //
    template<typename T> INLINE bool has_components(Entity object)
    {
        return has_component(TypeInfo::id<Component, T>(), object);
    }

    template<typename T1, typename T2, typename ... Args> INLINE bool has_components(Entity object)
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
        size_t size(TypeInfo::index_type);
        size_t capacity(TypeInfo::index_type);
    }
}

// INCLUDED METHODS OF ENTITY
INLINE bool Entity::operator == (const Entity& rh) const
{
    return _index == rh._index && _version == rh._version;
}

INLINE bool Entity::operator != (const Entity& rh) const
{
    return !(*this == rh);
}

INLINE bool Entity::operator < (const Entity& rh) const
{
    return _version == rh._version ? _index < rh._index : _version < rh._version;
}

INLINE Entity::index_type Entity::get_index() const
{
    return _index;
}

INLINE Entity::index_type Entity::get_version() const
{
    return _version;
}

INLINE void Entity::invalidate()
{
    _index = invalid;
    _version = invalid;
}

// INCLUDED METHODS OF ENTITY VIEW AND ITERATOR
namespace ecs
{
    INLINE iterator::iterator(Entity::index_type index, ComponentMask mask)
    : _index(index), _mask(mask)
    {
        _index = internal::find_next_available(_index, _mask, true);
    }

    INLINE iterator& iterator::operator ++ ()
    {
        _index = internal::find_next_available(_index, _mask);
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
        return _index == rhs._index && _mask == rhs._mask;
    }

    INLINE bool iterator::operator != (const iterator& rhs) const
    {
        return !(*this == rhs);
    }

    INLINE Entity iterator::operator * () const
    {
        return internal::get(_index);
    }

    template<typename ... Args> view<Args...>::view()
    : _mask(get_components_mask<Args...>())
    {}

    template<typename ... Args> INLINE iterator view<Args...>::begin() const
    {
        return iterator(0, _mask);
    }

    template<typename ... Args> INLINE iterator view<Args...>::end() const
    {
        return iterator(Entity::invalid, _mask);
    }

    template<typename ... Args> INLINE void view<Args...>::visit(const visitor& cb)
    {
        for( auto cursor : *this )
            cb(cursor, *::lemon::core::get_component<Args>(cursor) ...);
    }
}

template<typename T, typename ... Args> T* add_component(Entity object, Args && ... args)
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
    const_cast<Entity&>(component->object) = object;
    emit<EvtComponentAdded<T>>(object, *component);
    emit<EvtEntityModified>(object);

    if( !component->initialize() )
    {
        internal::remove_component(id, object);
        return nullptr;
    }
    return component;
}

template<typename T, typename ... Args> INLINE Entity spawn_with(Args && ... args)
{
    auto object = spawn();
    add_component<T>(object, std::forward<Args>(args)...);
    return object;
}

template<typename T> INLINE T* get_component(Entity object)
{
    const auto id = TypeInfo::id<Component, T>();
    return static_cast<T*>(internal::get_component(id, object));
}

template<typename ... T> INLINE std::tuple<T*...> get_components(Entity object)
{
    return std::make_tuple(get_component<T>(object)...);
}

template<typename T> INLINE void remove_component(Entity object)
{
    const auto id = TypeInfo::id<Component, T>();
    internal::remove_component(id, object);
}

template<typename T> INLINE bool has_component(Entity object)
{
    const auto id = TypeInfo::id<Component, T>();
    return internal::has_component(id, object);
}

template<> INLINE bool has_components(Entity object)
{
    return true;
}

template<typename ... Args> INLINE bool has_components(Entity object)
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

namespace std
{
    template<> struct hash<lemon::core::Entity>
    {
        std::size_t operator() (const lemon::core::Entity& entity) const
        {
            return static_cast<std::size_t>((size_t)entity.get_index() ^ (size_t)entity.get_version());
        }
    };
}
