// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <core/subsystem.hpp>

#include <codebase/type_traits.hpp>
#include <codebase/memory/indexed_pool.hpp>

#include <bitset>
#include <mutex>

NS_LEMON_CORE_BEGIN

// Component is the raw data for one aspect of the entity
struct Component
{
    // bitset mask of components
    using Mask = std::bitset<kEntMaxComponents>;
    template<typename ... Args> static Mask calculate();

    // utilities of specialization of template
    template<typename T> using enable_if = typename std::enable_if<std::is_base_of<Component, T>::value, T>::type;
    template<typename T> using enable_true_if = typename std::enable_if<std::is_base_of<Component, T>::value>::type;

protected:
    template<typename T> static Mask calculate_recursive();
    template<typename T1, typename T2, typename ... Args> static Mask calculate_recursive();

    void operator delete(void*) = delete;
    void operator delete[](void*) = delete;
};

// an Entity is a convenience class wrapping all the references to component, which
// are allocated by the EntityComponentSystem. its associated with a unique versioned
// handle. its recommended to store Handle instead of Entity directly.

// sizeof(Entity) ~= sizeof(size_t) * kEntMaxComponents
// it consumes 1mb memory every 2k entities, if we have at most 64 different components.
struct EntityComponentSystem;
struct Entity
{
    // assign a component to object, passing through component constructor arguments
    template<typename T, typename ... Args> Component::enable_if<T>* add_component(Args&&...);

    // retrieve a component assigned to object
    template<typename T> Component::enable_if<T>* get_component();
    template<typename T> const Component::enable_if<T>* get_component() const;
    template<typename ... Args> std::tuple<Args*...> get_components();
    template<typename ... Args> std::tuple<const Args*...> get_components() const;

    // remove a component from object
    template<typename T, typename Enable = Component::enable_true_if<T>> void remove_component();

    // check if we have specified components
    template<typename T, typename Enable = Component::enable_true_if<T>> bool has_component() const;
    template<typename ... Args> bool has_components() const;

    // get bitmask representation of handled components
    Component::Mask get_components_mask() const;

    // returns handle to this entity
    const Handle handle;

protected:
    template<typename T> bool has_components_recursive() const;
    template<typename T1, typename T2, typename ... Args> bool has_components_recursive() const;

protected:
    friend class EntityComponentSystem;

    Entity(EntityComponentSystem& world, Handle handle);
    EntityComponentSystem& _world;

    Component::Mask _mask; // bitmask of components associated with each entity
    Component* _table[kEntMaxComponents]; // a sparse array which keeps all the references to component
};

// EntityComponentSystem provides lifetime management of entities
namespace details
{
    struct ComponentResolver
    {
        virtual ~ComponentResolver() {}
        virtual void free(void*) = 0;
    };

    template<typename T, size_t Growth=kEntPoolChunkSize>
    struct ComponentResolverT : public ComponentResolver
    {
        template<typename ... Args> T* create(Args&& ... args);
        void free(void* data) override;
        size_t size() const;
        size_t capacity() const;

    protected:
        std::mutex _mutex;
        MemoryPoolT<T, Growth> _allocator;
    };

}

struct EntityComponentSystem : public Subsystem
{
    // an iterator over a specified view with components of the entites in an HandleManager.
    struct iterator : public std::iterator<std::forward_iterator_tag, Entity*>
    {
        iterator(EntityComponentSystem& world, Component::Mask mask, ReuseableHandleSet::iterator current)
        : _world(world), _mask(mask), _current(current) {}

        iterator operator ++ (int);
        iterator& operator ++ ();

        bool operator == (const iterator&) const;
        bool operator != (const iterator&) const;

        Entity* operator*() const;

    protected:
        EntityComponentSystem& _world;
        Component::Mask _mask;
        ReuseableHandleSet::iterator _current;
    };

    struct view
    {
        view(EntityComponentSystem& world, Component::Mask mask)
        : _world(world), _mask(mask) {}

        iterator begin() const;
        iterator end() const;

    protected:
        EntityComponentSystem& _world;
        Component::Mask _mask;
    };

    template<typename ... Args> struct view_traits : public view
    {
        view_traits(EntityComponentSystem&);

        void visit(const std::function<void(Entity&, Args& ...)>&);
        void collect(std::vector<Entity*>&);
        template<typename ... ToArgs> void collect(std::vector<std::tuple<ToArgs*...>>&);
        size_t count() const;
    };

public:
    // initialize entity component system
    bool initialize() override;
    // dispose all the alive objects
    void dispose() override;

    // spawn a new entity and returns handle as identifier
    Entity* create();
    Entity* get(Handle);
    // recycle components and associated to this entity, and invalidate handle
    void free(Entity*);
    void free(Handle);
    // recycle all entities and components
    void free_all();
    // returns true if the entity handle is current alive
    bool is_valid(Handle) const;
    // returns the count of alive entities
    size_t size() const;

    // find entities that have all of the specified components, returns a incremental iterator
    view_traits<> find_entities();
    template<typename ... T> view_traits<T...> find_entities_with();

protected:
    friend class Entity;
    template<typename T> details::ComponentResolverT<T>* resolve();

protected:
    std::mutex _mutex;
    IndexedMemoryPoolT<Entity, kEntPoolChunkSize> _entities;
    std::vector<details::ComponentResolver*> _resolvers;
};

//
// IMPLEMENTATIONS of COMPONENT
template<> INLINE Component::Mask Component::calculate()
{
    return Component::Mask();
}

template<typename ... Args> Component::Mask Component::calculate()
{
    return calculate_recursive<Args...>();
}

template<typename T> Component::Mask Component::calculate_recursive()
{
    Component::Mask mask;
    mask.set(TypeInfo::id<Component, T>());
    return mask;
}

template<typename T1, typename T2, typename ... Args> Component::Mask Component::calculate_recursive()
{
    return calculate_recursive<T1>() | calculate_recursive<T2, Args...>();
}

//
// IMPLEMENTATIONS of COMPONENT RESOLVER
namespace details
{
    template<typename T, size_t Growth>
    template<typename ... Args> T* ComponentResolverT<T, Growth>::create(Args&& ... args)
    {
        T* data = nullptr;
        {
            std::unique_lock<std::mutex> L(_mutex);
            data = static_cast<T*>(_allocator.malloc());
        }

        ::new (data) T(std::forward<Args>(args)...);
        return data;
    }

    template<typename T, size_t Growth> void ComponentResolverT<T, Growth>::free(void* data)
    {
        static_cast<T*>(data)->~T();

        {
            std::unique_lock<std::mutex> L(_mutex);
            _allocator.free(data);
        }
    }

    template<typename T, size_t Growth> size_t ComponentResolverT<T, Growth>::size() const
    {
        return _allocator.size();
    }

    template<typename T, size_t Growth> size_t ComponentResolverT<T, Growth>::capacity() const
    {
        return _allocator.capacity();
    }
}

//
// IMPLEMENTATIONS of ENTITY
template<typename T, typename ... Args>
Component::enable_if<T>* Entity::add_component(Args && ... args)
{
    const auto index = TypeInfo::id<Component, T>();
    ASSERT(_table[index] == nullptr, "duplicated component %s.", typeid(T).name());

    _table[index] = _world.resolve<T>()->create(std::forward<Args>(args)...);
    _mask.set(index);
    return static_cast<T*>(_table[index]);
}

template<typename T> Component::enable_if<T>* Entity::get_component()
{
    const auto index = TypeInfo::id<Component, T>();
    return static_cast<T*>(_table[index]);
}

template<typename T> const Component::enable_if<T>* Entity::get_component() const
{
    const auto index = TypeInfo::id<Component, T>();
    return static_cast<const T*>(_table[index]);
}

template<typename ... Args> std::tuple<Args*...> Entity::get_components()
{
    return std::make_tuple(get_component<Args>()...);
}

template<typename ... Args> std::tuple<const Args*...> Entity::get_components() const
{
    return std::make_tuple(get_component<Args>()...);
}

template<typename T, typename Enable> void Entity::remove_component()
{
    const auto index = TypeInfo::id<Component, T>();
    ASSERT(_table[index] != nullptr, "remove undefined component %s.", typeid(T).name());

    _world.resolve<T>()->free(_table[index]);
    _mask.reset(index);
    _table[index] = nullptr;
}

template<typename T, typename Eanble> bool Entity::has_component() const
{
    const TypeInfo::index_t index = TypeInfo::id<Component, T>();
    return _mask.test(index);
}

template<> INLINE bool Entity::has_components() const
{
    return true;
}

template<typename ... Args> bool Entity::has_components() const
{
    return has_components_recursive<Args...>();
}

template<typename T> bool Entity::has_components_recursive() const
{
    return has_component<T>();
}

template<typename T1, typename T2, typename ... Args> bool Entity::has_components_recursive() const
{
    return has_component<T1>() && has_components_recursive<T2, Args...>();
}

INLINE Component::Mask Entity::get_components_mask() const
{
    return _mask;
}

//
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

INLINE Entity* EntityComponentSystem::iterator::operator * () const
{
    return _world.get(*_current);
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

template<typename ... Args> EntityComponentSystem::view_traits<Args...>::view_traits(EntityComponentSystem& world)
: view(world, Component::calculate<Args...>())
{}

template<typename ... Args>
void EntityComponentSystem::view_traits<Args...>::visit(const std::function<void(Entity&, Args&...)>& cb)
{
    for( auto entity : *this )
        cb(*entity, *entity->template get_component<Args>()...);
}

template<typename ... Args>
void EntityComponentSystem::view_traits<Args...>::collect(std::vector<Entity*>& ct)
{
    for( auto entity : *this )
        ct.push_back(entity);
}

template<typename ... Args> template<typename ... ToArgs>
void EntityComponentSystem::view_traits<Args...>::collect(std::vector<std::tuple<ToArgs*...>>& ct)
{
    static_assert(AllTrue<std::is_convertible<Args*, ToArgs*>::value...>::value, "");
    for( auto entity : *this )
    {
        auto tuple = std::make_tuple(static_cast<ToArgs*>(entity->template get_component<Args>())... );
        ct.push_back(tuple);
    }
}

template<typename ... Args>
size_t EntityComponentSystem::view_traits<Args...>::count() const
{
    size_t i = 0;
    for( auto entity : *this )
        i ++;
    return i;
}


INLINE Entity* EntityComponentSystem::get(Handle handle)
{
    return static_cast<Entity*>(_entities.get(handle));
}

INLINE bool EntityComponentSystem::is_valid(Handle handle) const
{
    return _entities.is_valid(handle);
}

INLINE size_t EntityComponentSystem::size() const
{
    return _entities.size();
}

INLINE EntityComponentSystem::view_traits<> EntityComponentSystem::find_entities()
{
    return view_traits<>(*this);
}

template<typename ... Args>
EntityComponentSystem::view_traits<Args...> EntityComponentSystem::find_entities_with()
{
    return view_traits<Args...>(*this);
}

template<typename T>
details::ComponentResolverT<T>* EntityComponentSystem::resolve()
{
    const TypeInfo::index_t index = TypeInfo::id<Component, T>();
    ASSERT(index < kEntMaxComponents,
        "too many components,"
        "please considering increase the constants \'kEntMaxComponents\'.");

    if( _resolvers.size() <= index )
        _resolvers.resize(index+1);

    if( _resolvers[index] == nullptr )
        _resolvers[index] = new (std::nothrow) details::ComponentResolverT<T>();

    return static_cast<details::ComponentResolverT<T>*>(_resolvers[index]);
}

NS_LEMON_CORE_END
