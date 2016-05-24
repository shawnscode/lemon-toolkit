#pragma once

#include "flow2d.hpp"
#include "memory.hpp"

#include <bitset>
#include <vector>

// a fast, type-safe c++ entity component system based on [EntityX](https://github.com/alecthomas/entityx).

NS_FLOW2D_BEGIN

typedef std::bitset<kEntMaxComponents> ComponentMask;

// a convenience handle around an Entity::Uid.
// if an entity is destroyed, any copies will be invalidated. use is_valid() to
// check for validity before using.
struct Entity
{
    struct Uid
    {
        Uid() : m_id(0)
        {}
        Uid(uint32_t index, uint32_t version)
        : m_id(uint64_t(index) | (uint64_t(version) << 32)) {}

        uint64_t id() const { return m_id; }
        uint32_t index() const { return m_id & 0xffffffffUL; }
        uint32_t version() const { return m_id >> 32; }

        bool operator == (const Uid& rh) const { return m_id == rh.m_id; }
        bool operator != (const Uid& rh) const { return m_id != rh.m_id; }
        bool operator < (const Uid& rh) const { return m_id < rh.m_id; }

    private:
        friend class EntityManager;
        uint64_t m_id;
    };

    Entity() {}

    // check if entity handle is invalid
    bool is_valid() const;
    void assert_valid() const;
    operator bool() const;

    //
    bool operator == (const Entity&) const;
    bool operator != (const Entity&) const;
    bool operator <  (const Entity&) const;

    template<typename T, typename ... Args> ComponentHandle<T> add_component(Args && ... args);
    template<typename T> ComponentHandle<T> add_component(const T&);
    template<typename T> ComponentHandle<T> get_component();
    template<typename ... T> std::tuple<ComponentHandle<T>...> get_components();
    template<typename T> void remove_component();
    template<typename T> bool has_component() const;

    Entity::Uid     get_uid() const;
    ComponentMask   get_components_mask() const;

    // invalidate entity handle, disassociating it from entity manager.
    void invalidate();
    // destroy and invalidate this entity.
    void dispose();

private:
    friend class EntityManager;
    friend class EntityIterator;

    Entity(EntityManager*, Entity::Uid);

    EntityManager*  m_manager       = nullptr;
    Entity::Uid     m_identifier    = INVALID;

public:
    const static Uid INVALID;
};

// an iterator over a specified view with components of the entites in an EntityManager.
struct EntityIterator : public std::iterator<std::input_iterator_tag, Entity::Uid>
{
    EntityIterator(EntityManager* manager, uint32_t index);
    EntityIterator(EntityManager* manager, const ComponentMask mask, uint32_t index);

    EntityIterator& operator ++ ();
    bool            operator == (const EntityIterator&) const;
    bool            operator != (const EntityIterator&) const;
    Entity          operator * ();
    const Entity    operator * () const;

protected:
    void next();
    bool predicate() const;
    bool is_finished() const;

    EntityManager*  m_manager;
    ComponentMask   m_mask;
    uint32_t        m_current_cursor;
    size_t          m_capacity;
};

struct EntityView
{
    EntityIterator begin();
    EntityIterator end();
    const EntityIterator begin() const;
    const EntityIterator end() const;

protected:
    friend class EntityManager;
    explicit EntityView(EntityManager* manager, ComponentMask mask = ComponentMask());

    EntityManager*  m_manager;
    ComponentMask   m_mask;
};

template<typename ... T>
struct EntityViewTrait : public EntityView
{
    void each(std::function<void(Entity, T&...)>);

protected:
    friend class EntityManager;
    explicit EntityViewTrait(EntityManager*);
};

// a ComponentHandle is a wrapper around an instance of a component,
// it provides safe access to components. the handle will be invalidated
// under the following conditions:
// 1. if a component is removed from its host entity;
// 2. if its host entity is destroyed.
template<typename T> struct ComponentHandle
{
    typedef T type;

    ComponentHandle() {}

    // check if component handle is valid
    bool is_valid() const;
    void assert_valid() const;
    operator bool() const;

    // remove the component from its entity and destroy it.
    void dispose();
    // returns the entity associated with the component.
    Entity entity();

    // 
    T* operator -> ();
    const T* operator -> () const;
    T* get();
    const T* get() const;

    bool operator == (const ComponentHandle<T> &rh) const;
    bool operator != (const ComponentHandle<T> &rh) const;

private:
    friend class EntityManager;
    ComponentHandle(EntityManager*, Entity::Uid);

    EntityManager*  m_manager   = nullptr;
    Entity::Uid     m_ent       = Entity::INVALID;
};

struct Component
{
    typedef int32_t Class;

    // component memory is always managed by entity manager.
    void operator delete(void* p);
    void operator delete[](void *p);

protected:
    friend class EntityManager;
    static Class s_class_counter;
};

template<typename T> struct ComponentTrait : public Component
{
    typedef ComponentHandle<T> Handle;
    static Class get_class();
};

struct EntityManager
{
    // non-copyable
    EntityManager() = default;
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;

    ~EntityManager();

    size_t  size() const;
    size_t  capacity() const;
    void    reset();

    // check if the entity id is still valid
    bool    is_valid(Entity::Uid) const;
    bool    is_valid(Entity::Uid, const ComponentMask mask) const;
    void    assert_valid(Entity::Uid) const;
    Entity  create();
    Entity  create_from(Entity);
    Entity  get(Entity::Uid);
    void    erase(Entity::Uid);
    Entity::Uid make_uid(uint32_t);

    // assign a component to an Entity::Uid, passing through component constructor arguments.
    template<typename T, typename ... Args> ComponentHandle<T> add_component(Entity::Uid, Args && ... args);
    // retrieve a component assigned to an Entity::Uid.
    template<typename T> ComponentHandle<T> get_component(Entity::Uid);
    template<typename ... T> std::tuple<ComponentHandle<T> ...> get_components(Entity::Uid);
    template<typename T> T* get_component_ptr(Entity::Uid);
    template<typename T> const T* get_component_ptr(Entity::Uid) const;
    // remove a component from an Entity::Uid.
    template<typename T> void remove_component(Entity::Uid);
    // check if an entity has a component.
    template<typename T> bool has_component(Entity::Uid) const;
    ComponentMask get_components_mask(Entity::Uid) const;

    // find entities that have all of the specified components, returns a incremental iterator
    template<typename T> ComponentMask get_components_mask() const;
    template<typename T1, typename T2, typename ...Args> ComponentMask get_components_mask() const;
    template<typename ... T> EntityViewTrait<T...> find_entities_with();
    EntityView get_entities();

private:
    void accomodate_entity(uint32_t);
    template<typename T> ObjectChunksTrait<T>* get_chunks();

    uint32_t m_index_counter = 0;
    // each element in componets_pool corresponds to a Pool for a Component.
    // the index into the vector is the Component::get_class();
    std::vector<ObjectChunks*> m_components_pool;
    // bitmask of components associated with each entity.
    // the index into the vector is the Entity::Uid.
    std::vector<ComponentMask> m_components_mask;
    std::vector<bool> m_usages;
    // entity version numbers. incremented each time an entity is destroyed
    std::vector<uint32_t> m_versions;
    // list of available entity slots
    std::vector<uint32_t> m_freeslots;
};

#include "ecs.inl"
NS_FLOW2D_END

namespace std
{
    template<> struct hash<flow2d::Entity>
    {
        std::size_t operator() (const flow2d::Entity& entity) const
        {
            return static_cast<std::size_t>(entity.get_uid().index() ^ entity.get_uid().version());
        }
    };
}