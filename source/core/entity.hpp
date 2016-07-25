// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <core/memory.hpp>
#include <core/event.hpp>
#include <core/traits.hpp>

// a fast, type-safe c++ entity component system based on [EntityX](https://github.com/alecthomas/entityx).

NS_FLOW2D_BEGIN

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

struct ComponentBase
{
    // component memory is always managed by entity manager.
    void operator delete(void*);
    void operator delete[](void*);

    virtual void on_dispose(EntityManager&, Entity) {}
    virtual void on_spawn(EntityManager&, Entity) {}
};

template<size_t s = kEntPoolChunkSize> struct Component : ComponentBase
{
    static const size_t chunk_size = s;
};

// entity life-circle management and component assignments
struct EntityManager
{
    // an iterator over a specified view with components of the entites in an EntityManager.
    struct iterator : public std::iterator<std::forward_iterator_tag, Entity>
    {
        iterator(EntityManager& manager, Entity::index_type index = Entity::invalid, ComponentMask mask = ComponentMask())
        : _world(manager), _index(index), _mask(mask)
        {
            find_next_available();
        }

        iterator&   operator ++ ();
        bool        operator == (const iterator&) const;
        bool        operator != (const iterator&) const;
        Entity      operator * () const;

    protected:
        void find_next_available();

        EntityManager&      _world;
        ComponentMask       _mask;
        Entity::index_type  _index;
    };

    struct view
    {
        view(EntityManager& manager, ComponentMask mask)
        : _world(manager), _mask(mask) {}

        iterator begin() const;
        iterator end() const;

    protected:
        EntityManager&  _world;
        ComponentMask   _mask;
    };

    template<typename ...T> struct view_trait : public view
    {
        view_trait(EntityManager& manager)
        : view(manager, manager.get_components_mask<T...>()) {}

        using visitor = std::function<void(Entity, T& ...)>;
        void visit(const visitor&);
    };

    EntityManager();
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
    // retrieve a component assigned to an Entity
    template<typename T> T* get_component(Entity);
    template<typename ... T> std::tuple<T*...> get_components(Entity);
    // remove a component from an Entity
    template<typename T> void remove_component(Entity);
    // check if an entity has a component
    template<typename T> bool has_component(Entity) const;
    template<typename T> bool has_components(Entity) const;
    template<typename T1, typename T2, typename ...Args> bool has_components(Entity) const;
    // get bitmask representation of components
    ComponentMask get_components_mask(Entity) const;

    // find entities that have all of the specified components, returns a incremental iterator
    template<typename ... T> view_trait<T...> find_entities_with();
    view find_entities();

    // utils of iterators
    template<typename T> ComponentMask get_components_mask() const;
    template<typename T1, typename T2, typename ...Args> ComponentMask get_components_mask() const;

    // event dispatchers
    EventManager& get_dispatcher();
    

protected:
    template<typename T> using object_chunks = IndexedObjectChunks<T, Entity::index_type, 8>;
    template<typename T> object_chunks<T>* get_chunks();
    void accomodate_entity(uint32_t);

    // event dispatcher
    std::unique_ptr<EventManager> _dispatcher;
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
    explicit EvtEntityCreated(Entity entity) : entity(entity) {}
    Entity entity;
};

struct EvtEntityDisposed
{
    explicit EvtEntityDisposed(Entity entity) : entity(entity) {}
    Entity entity;
};

template<typename T> struct EvtComponentAdded
{
    EvtComponentAdded(Entity entity, T& component)
    : entity(entity), component(component) {}

    Entity  entity;
    T&      component;
};

template<typename T> struct EvtComponentRemoved
{
    EvtComponentRemoved(Entity entity, T& component)
    : entity(entity), component(component) {}

    Entity  entity;
    T&      component;
};

#include <core/entity.inl>
NS_FLOW2D_END

namespace std
{
    template<> struct hash<flow2d::Entity>
    {
        std::size_t operator() (const flow2d::Entity& entity) const
        {
            return static_cast<std::size_t>((size_t)entity.get_index() ^ (size_t)entity.get_version());
        }
    };
}