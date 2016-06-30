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

// represents an entity which is made up of components.
struct Entity
{
    using index_type = uint16_t; // supports 60k+ entities and versions.
    const static index_type invalid = std::numeric_limits<index_type>::max();

    // check if entity handle is invalid
    bool is_alive() const;
    operator bool () const;
    bool operator == (const Entity&) const;
    bool operator != (const Entity&) const;
    bool operator <  (const Entity&) const;

    index_type get_index() const;
    index_type get_version() const;

    // assign a component to an Entity::Uid, passing through component constructor arguments.
    template<typename T, typename ... Args> T* add_component(Args && ... args);
    template<typename T> T* add_component(const T&);
    // retrieve a component assigned to this collection
    template<typename T> T* get_component();
    template<typename ...T> std::tuple<T* ...> get_components();
    // check if an collection has a component.
    template<typename T> bool has_component() const;
    // remove a component from this collection
    template<typename T> void remove_component();
    // get bitmask representation of components
    ComponentMask   get_components_mask() const;

    // invalidate entity handle, disassociating it from entity manager.
    void invalidate();
    // destroy and invalidate this entity.
    void dispose();

private:
    friend class EntityManager;

    Entity(EntityManager& world, index_type index, index_type version)
    : _world(world), _index(index), _version(version)
    {}

    EntityManager&  _world;
    index_type      _index      = invalid;
    index_type      _version    = invalid;
};

struct Component
{
    // component memory is always managed by entity manager.
    void operator delete(void*);
    void operator delete[](void*);
};

template<typename T> struct ComponentTraitInfo
{
    static TypeID::index_type id() { return TypeID::value<Component, T>(); }
};

// entity life-circle management and component assignments
struct EntityManager
{
    // an iterator over a specified view with components of the entites in an EntityManager.
    struct Iterator : public std::iterator<std::forward_iterator_tag, Entity>
    {
        Iterator(EntityManager& manager, Entity::index_type index = Entity::invalid, ComponentMask mask = ComponentMask())
        : _manager(manager), _index(index), _mask(mask)
        {
            find_next_available();
        }

        Iterator&   operator ++ ();
        bool        operator == (const Iterator&) const;
        bool        operator != (const Iterator&) const;
        Entity      operator * () const;

    protected:
        void find_next_available();

        EntityManager&      _manager;
        ComponentMask       _mask;
        Entity::index_type  _index;
    };

    struct View
    {
        View(EntityManager& manager, ComponentMask mask)
        : _manager(manager), _mask(mask) {}

        Iterator begin() const;
        Iterator end() const;

    protected:
        EntityManager&  _manager;
        ComponentMask   _mask;
    };

    template<typename ...T> struct ViewTrait : public View
    {
        using callback = std::function<void(Entity, T& ...)>;

        ViewTrait(EntityManager& manager)
        : View(manager, manager.get_components_mask<T...>()) {}

        void each(callback);
    };

    using iterator = Iterator;
    using view = View;
    template<typename ...T> using view_trait = ViewTrait<T...>;

    EntityManager(EventManager& manager) : _dispatcher(manager) {}
    ~EntityManager();

    // non-copyable
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;

    size_t  size() const;
    size_t  capacity() const;
    void    reset();

    /// lifetime operations of Entity
    Entity  spawn();
    Entity  clone(Entity);
    void    dispose(Entity);
    bool    is_alive(Entity) const;

    // assign a component to an Entity::Uid, passing through component constructor arguments
    template<typename T, typename ... Args> T* add_component(Entity, Args && ... args);
    // retrieve a component assigned to an Entity
    template<typename T> T* get_component(Entity);
    template<typename ... T> std::tuple<T*...> get_components(Entity);
    // remove a component from an Entity
    template<typename T> void remove_component(Entity);
    // check if an entity has a component
    template<typename T> bool has_component(Entity) const;
    // get bitmask representation of components
    ComponentMask get_components_mask(Entity) const;

    // find entities that have all of the specified components, returns a incremental iterator
    template<typename ... T> view_trait<T...> find_entities_with();
    view find_entities();

    // utils of iterators
    template<typename T> ComponentMask get_components_mask() const;
    template<typename T1, typename T2, typename ...Args> ComponentMask get_components_mask() const;

private:
    using object_chunks = ObjectChunks<Entity::index_type>;
    template<typename T> using object_chunks_trait = ObjectChunksTrait<T, Entity::index_type>;

    void accomodate_entity(uint32_t);
    template<typename T> object_chunks_trait<T>* get_chunks();

    EventManager& _dispatcher;

    // incremented entity index for brand new and free slot
    Entity::index_type _incremental_index = 0;
    // each element in componets_pool corresponds to a Pool for a Component
    // the index into the vector is the Component::type();
    std::vector<object_chunks*> _components_pool;
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