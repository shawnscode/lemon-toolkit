// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/message.hpp>

#include <codebase/type/type_traits.hpp>
#include <codebase/memory/indexed_pool.hpp>

#include <bitset>

// a fast, type-safe, data-driven c++ entity component system based on
// [HandleX](https://github.com/alecthomas/entityx).
NS_LEMON_CORE_BEGIN

using ComponentMask = std::bitset<kEntMaxComponents>;

// entity is defined by the components it consists of.
struct Entity
{
    Entity(EntityComponentSystem& world) : _world(world) {}

    template<typename ... Args> static ComponentMask get_components_mask();

    // assign a component to object, passing through component constructor arguments
    template<typename T, typename ... Args> T* add_component(Args&& ...);

    // retrieve a component assigned to object
    template<typename T> T* get_component();
    template<typename ... Args> std::tuple<Args*...> get_components();

    // remove a component from object
    template<typename T> void remove_component();

    // check if we have specified components
    template<typename T> bool has_component();
    template<typename ... Args> bool has_components();

    // get bitmask representation of handled components
    ComponentMask get_components_mask();

protected:
    friend class EntityComponentSystem;

    template<typename T> static ComponentMask get_components_mask_recursive();
    template<typename T1, typename T2, typename ... Args> static ComponentMask get_components_mask_recursive();

    template<typename T> bool has_components_recursive();
    template<typename T1, typename T2, typename ... Args> bool has_components_recursive();

    // references to owner
    EntityComponentSystem& _world;
    // bitmask of components associated with each entity
    // the index into the vector is the Handle::Uid
    ComponentMask _mask;
    // a sparse array which keeps all the handles of component
    Handle _components[kEntMaxComponents];
};

//
struct Component
{
    // static dispatch instead of virtual table
    bool initialize() { return true; }
    void dispose() {}

    //
    const Handle handle;
    operator Handle () const { return handle; }

protected:
    Component() : handle({}) {}

    // component memory is always managed by entity manager.
    void operator delete(void*) = delete;
    void operator delete[](void*) = delete;  
};

// 
struct EntityComponentSystem
{
    // an iterator over a specified view with components of the entites in an HandleManager.
    struct iterator : public std::iterator<std::forward_iterator_tag, Handle>
    {
        iterator(EntityComponentSystem& world, ComponentMask mask, ReuseableHandleSet::iterator current)
        : _world(world), _mask(mask), _current(current) {}

        iterator operator ++ (int);
        iterator& operator ++ ();

        bool operator == (const iterator&) const;
        bool operator != (const iterator&) const;
        Handle operator * () const;

    protected:
        EntityComponentSystem& _world;
        ComponentMask _mask;
        ReuseableHandleSet::iterator _current;
    };

    struct view
    {
        view(EntityComponentSystem& world, ComponentMask mask)
        : _world(world), _mask(mask) {}

        iterator begin() const;
        iterator end() const;

    protected:
        EntityComponentSystem& _world;
        ComponentMask _mask;
    };

    template<typename ... Args> struct view_t : public view
    {
        view_t(EntityComponentSystem&);
        // visit entities with specialized components
        void visit(const std::function<void(Handle, Args& ...)>&);
    };

    // initialize entity component system
    bool initialize();
    // dispose all the alive objects
    void dispose();

    // spawn a new entity and returns handle as identifier
    Handle create();
    // returns Entity associated with handle
    Entity* get(Handle);
    // recycle components and associated to this entity, and invalidate handle
    void free(Handle);
    // recycle all entities and components
    void free_all();
    // returns true if the entity handle is current alive
    bool is_valid(Handle) const;
    // returns the count of alive entities
    size_t size() const;

    // find entities that have all of the specified components, returns a incremental iterator
    view_t<> find_entities();
    template<typename ... T> view_t<T...> find_entities_with();

protected:
    friend class Entity;

    template<typename T> IndexedMemoryPool* get_component_pool();
    template<typename T, typename ... Args> Handle add_component(Args&& ...);
    template<typename T> T* get_component(Handle);
    template<typename T> void remove_component(Handle);

protected:
    IndexedMemoryPoolT<Entity, kEntPoolChunkSize> _entities;
    std::vector<IndexedMemoryPool*> _component_sets;
    std::vector<std::function<void(Handle, void*)>> _destructors;
};

// dispatched events during the whole life of event manager
struct EvtEntityModified
{
    explicit EvtEntityModified(Handle object) : object(object) {}
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

// IMPLEMENTATIONS of ENTITY COMPONENT SYSTEM
INLINE EntityComponentSystem::iterator& EntityComponentSystem::iterator::operator++()
{  
    ++_current;
    for( ; _current != _world._entities.end(); ++_current )
    {
        if( (_world._entities.get_t(*_current)->_mask & _mask) == _mask )
        {
            break;
        }
    }
    return *this;
}

INLINE EntityComponentSystem::iterator EntityComponentSystem::iterator::operator++(int _dummy)
{
    auto it = *this;
    ++(*this);
    return it;
}

INLINE bool EntityComponentSystem::iterator::operator == (const iterator& rhs) const
{
    return &_world == &rhs._world && _mask == rhs._mask && _current == rhs._current;
}

INLINE bool EntityComponentSystem::iterator::operator != (const iterator& rhs) const
{
    return !(*this == rhs);
}

INLINE Handle EntityComponentSystem::iterator::operator * () const
{
    return *_current;
}

INLINE EntityComponentSystem::iterator EntityComponentSystem::view::begin() const
{
    for( auto it = _world._entities.begin(); it != _world._entities.end(); it++ )
    {
        if( (_world._entities.get_t(*it)->_mask & _mask) == _mask )
            return iterator(_world, _mask, it);
    }

    return iterator(_world, _mask, _world._entities.end());
}

INLINE EntityComponentSystem::iterator EntityComponentSystem::view::end() const
{
    return iterator(_world, _mask, _world._entities.end());
}

template<typename ... Args> EntityComponentSystem::view_t<Args...>::view_t(EntityComponentSystem& world)
: view(world, Entity::get_components_mask<Args...>())
{}

template<typename ... Args>
void EntityComponentSystem::view_t<Args...>::visit(const std::function<void(Handle, Args&...)>& cb)
{
    for( auto handle : *this )
    {
        auto entity = _world.get(handle);
        cb(handle, *entity->template get_component<Args>()...);
    }
}

INLINE bool EntityComponentSystem::is_valid(Handle handle) const
{
    return _entities.is_valid(handle);
}

INLINE size_t EntityComponentSystem::size() const
{
    return _entities.size();
}

INLINE EntityComponentSystem::view_t<> EntityComponentSystem::find_entities()
{
    return view_t<>(*this);
}

template<typename ... Args> EntityComponentSystem::view_t<Args...> EntityComponentSystem::find_entities_with()
{
    return view_t<Args...>(*this);
}

template<typename T> IndexedMemoryPool* EntityComponentSystem::get_component_pool()
{
    const TypeInfo::index_t id = TypeInfo::id<Component, T>();

    if( _component_sets.size() <= id )
    {
        _component_sets.resize(id+1, nullptr);
        _destructors.resize(id+1);
    }

    if( _component_sets[id] == nullptr )
    {
        _component_sets[id] = new (std::nothrow) IndexedMemoryPoolT<T, kEntPoolChunkSize>();

        _destructors[id] = [=](Handle handle, void* raw)
        {
            auto component = static_cast<T*>(raw);
            emit<EvtEntityModified>(handle);
            emit<EvtComponentRemoved<T>>(handle, *component);

            component->dispose();
            component->~T();
        };
    }

    return _component_sets[id];
}

template<typename T, typename ... Args> Handle EntityComponentSystem::add_component(Args&& ... args)
{
    auto pool = get_component_pool<T>();

    auto handle = pool->malloc();
    auto component = static_cast<T*>(pool->get(handle));
    if( component == nullptr )
        return handle;

    ::new (component) T(std::forward<Args>(args)...);
    component->initialize();
    const_cast<Handle&>(component->handle) = handle;
    emit<EvtEntityModified>(handle);
    emit<EvtComponentAdded<T>>(handle, *component);
    return handle;
}

template<typename T> T* EntityComponentSystem::get_component(Handle handle)
{
    return static_cast<T*>(get_component_pool<T>()->get(handle));
}

template<typename T> void EntityComponentSystem::remove_component(Handle handle)
{
    auto pool = get_component_pool<T>();
    auto component = static_cast<T*>(pool->get(handle));
    if( component == nullptr )
        return;

    const TypeInfo::index_t id = TypeInfo::id<Component, T>();
    _destructors[id](handle, component);
    pool->free(handle);
}

template<> INLINE ComponentMask Entity::get_components_mask()
{
    return ComponentMask();
}

template<typename ... Args> ComponentMask Entity::get_components_mask()
{
    return get_components_mask_recursive<Args...>();
}

template<typename T> ComponentMask Entity::get_components_mask_recursive()
{
    ComponentMask mask;
    mask.set(TypeInfo::id<Component, T>());
    return mask;
}

template<typename T1, typename T2, typename ... Args> ComponentMask Entity::get_components_mask_recursive()
{
    return get_components_mask_recursive<T1>() | get_components_mask_recursive<T2, Args...>();
}

template<typename T, typename ... Args> T* Entity::add_component(Args&& ... args)
{
    const TypeInfo::index_t id = TypeInfo::id<Component, T>();
    ASSERT( !has_component<T>(), "duplicated component %s.", typeid(T).name());

    auto handle = _world.add_component<T>(std::forward<Args>(args)...);
    if( !handle.is_valid() )
        return nullptr;

    _components[id] = handle;
    _mask.set(id);
    return _world.get_component<T>(handle);
}

template<typename T> T* Entity::get_component()
{
    const TypeInfo::index_t id = TypeInfo::id<Component, T>();

    return _world.get_component<T>(_components[id]);
}

template<typename ... Args> std::tuple<Args*...> Entity::get_components()
{
    return std::make_tuple(get_component<Args>()...);
}

template<typename T> void Entity::remove_component()
{
    const TypeInfo::index_t id = TypeInfo::id<Component, T>();

    _world.remove_component<T>(_components[id]);
    _components[id].invalidate();
    _mask.reset(id);
}

template<typename T> bool Entity::has_component()
{
    const TypeInfo::index_t id = TypeInfo::id<Component, T>();

    return _mask.test(id);
}

template<> INLINE bool Entity::has_components()
{
    return true;
}

template<typename ... Args> bool Entity::has_components()
{
    return has_components_recursive<Args...>();
}

template<typename T> bool Entity::has_components_recursive()
{
    return has_component<T>();
}

template<typename T1, typename T2, typename ... Args> bool Entity::has_components_recursive()
{
    return has_component<T1>() && has_components_recursive<T2, Args...>();
}

INLINE ComponentMask Entity::get_components_mask()
{
    return _mask;
}

NS_LEMON_CORE_END
