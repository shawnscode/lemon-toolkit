// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/defines.hpp>
#include <core/memory.hpp>
#include <core/typeinfo.hpp>
#include <core/event.hpp>

// a fast, type-safe c++ entity component system based on [EntityX](https://github.com/alecthomas/entityx).

NS_FLOW2D_CORE_BEGIN

typedef std::bitset<kEntMaxComponents> ComponentMask;

// represents an entity handle which is made up of components.
struct Entity
{
    using index_type = uint16_t; // supports 60k+ entities and versions.
    const static index_type invalid = std::numeric_limits<index_type>::max();

    Entity() = default;
    Entity(const Entity&) = default;
    Entity& operator = (const Entity&) = default;

    bool operator == (const Entity&) const;
    bool operator != (const Entity&) const;
    bool operator <  (const Entity&) const;

    index_type get_index() const;
    index_type get_version() const;

    // invalidate entity handle, disassociating it from entity manager.
    void invalidate();

private:
    friend class EntityManager;

    Entity(index_type index, index_type version)
    : _index(index), _version(version)
    {}

    index_type      _index      = invalid;
    index_type      _version    = invalid;
};

#define SET_CHUNK_SIZE(size) static const size_t chunk_size = size;

struct Component
{
    static const size_t chunk_size = kEntPoolChunkSize;

    // component memory is always managed by entity manager.
    void operator delete(void*) = delete;
    void operator delete[](void*) = delete;

    virtual void on_dispose() {}
    virtual void on_spawn() {}

    // assign a component to object, passing through component constructor arguments
    template<typename T, typename ... Args> T* add_component(Args && ... args);
    // retrieve a component assigned to object
    template<typename T> T* get_component();
    // remove a component from object
    template<typename T> void remove_component();
    // check if we has a component
    template<typename T> bool has_component() const;

private:
    friend class EntityManager;
    EntityManager* _world = nullptr;
    Entity         _object;
};

// entity life-circle management and component assignments
struct EntityManager
{
    // an iterator over a specified view with components of the entites in an EntityManager.
    template<typename T>
    struct t_iterator : public std::iterator<std::forward_iterator_tag, Entity>
    {
        t_iterator(T& manager, Entity::index_type index = Entity::invalid, ComponentMask mask = ComponentMask())
        : _world(manager), _index(index), _mask(mask)
        {
            find_next_available();
        }

        t_iterator& operator ++ ();
        bool        operator == (const t_iterator&) const;
        bool        operator != (const t_iterator&) const;
        Entity      operator * () const;

    protected:
        void find_next_available();
        T&                  _world;
        ComponentMask       _mask;
        Entity::index_type  _index;
    };

    using iterator = t_iterator<EntityManager>;
    using const_iterator = t_iterator<const EntityManager>;

    template<typename T> struct t_view
    {
        t_view(T& manager, ComponentMask mask)
        : _world(manager), _mask(mask) {}

        t_iterator<T> begin() const;
        t_iterator<T> end() const;

    protected:
        T&              _world;
        ComponentMask   _mask;
    };

    using view = t_view<EntityManager>;
    using const_view = t_view<const EntityManager>;

    template<typename T, typename ...Args> struct t_view_trait : public t_view<T>
    {
        t_view_trait(EntityManager& manager)
        : view(manager, manager.get_components_mask<Args...>()) {}

        using visitor = std::function<void(Entity, Args& ...)>;
        void visit(const visitor&);
    };

    template<typename ... Args> using view_trait = t_view_trait<EntityManager, Args...>;
    template<typename ... Args> using const_view_trait = t_view_trait<const EntityManager, Args...>;

    EntityManager(EventManager&);
    ~EntityManager();

    // non-copyable
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;

    size_t  size() const;
    size_t  capacity() const;
    void    reset();

    /// lifetime operations of Entity
    Entity  spawn();
    void    dispose(Entity);
    bool    is_alive(Entity) const;
    template<typename T, typename ... Args> Entity spawn_with(Args && ...args);

    // assign a component to an Entity::Uid, passing through component constructor arguments
    template<typename T, typename ... Args> T* add_component(Entity, Args && ... args);
    // remove a component from an Entity
    template<typename T> void remove_component(Entity);
    // check if an entity has a component
    template<typename T> bool has_component(Entity) const;
    template<typename T> bool has_components(Entity) const;
    template<typename T1, typename T2, typename ...Args> bool has_components(Entity) const;
    // retrieve a component assigned to an Entity
    template<typename T> T* get_component(Entity);
    template<typename T> const T* get_component(Entity) const;
    template<typename ... T> std::tuple<T*...> get_components(Entity);
    template<typename ... T> std::tuple<const T*...> get_components(Entity) const;

    // find entities that have all of the specified components, returns a incremental iterator
    view find_entities();
    const_view find_entities() const;
    template<typename ... T> view_trait<T...> find_entities_with();
    template<typename ... T> const_view_trait<T...> find_entities_with() const;

    // get bitmask representation of components
    ComponentMask get_components_mask(Entity) const;
    // utils of iterators
    template<typename T> ComponentMask get_components_mask() const;
    template<typename T1, typename T2, typename ...Args> ComponentMask get_components_mask() const;

protected:
    template<typename T> using object_chunks = IndexedObjectChunks<T, Entity::index_type, 8>;
    template<typename T> object_chunks<T>* get_chunks();
    void accomodate_entity(uint32_t);

    // reference of event dispatcher
    EventManager& _dispatcher;
    // incremented entity index for brand new and free slot
    Entity::index_type _incremental_index = 0;
    // each element in componets_pool corresponds to a Pool for a Component
    // the index into the vector is the Component::type();
    std::vector<MemoryChunks*> _components_pool;
    std::vector<std::function<void(Entity)>> _components_dispose;
    // bitmask of components associated with each entity
    // the index into the vector is the Entity::Uid
    std::vector<ComponentMask> _components_mask;
    // entity version numbers. incremented each time an entity is destroyed
    std::vector<Entity::index_type> _versions;
    // list of available entity slots
    std::vector<Entity::index_type> _freeslots;
};

/// dispatched events during the whole life of event manager
struct EvtEntityCreated
{
    explicit EvtEntityCreated(Entity object) : object(object) {}
    Entity object;
};

struct EvtEntityDisposed
{
    explicit EvtEntityDisposed(Entity object) : object(object) {}
    Entity object;
};

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

#include <core/entity.inl>
NS_FLOW2D_CORE_END

namespace std
{
    template<> struct hash<flow2d::core::Entity>
    {
        std::size_t operator() (const flow2d::core::Entity& entity) const
        {
            return static_cast<std::size_t>((size_t)entity.get_index() ^ (size_t)entity.get_version());
        }
    };
}