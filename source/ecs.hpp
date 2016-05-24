#pragma once

#include "flow2d.hpp"
#include "pool.hpp"

#include <bitset>
#include <vector>

// a fast, type-safe c++ entity component system based on [EntityX](https://github.com/alecthomas/entityx).

NS_FLOW2D_BEGIN

typedef std::bitset<kEntMaxComponents> ComponentMask;

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

    Entity(EntityManager*, Entity::Uid);

    EntityManager*  m_manager       = nullptr;
    Entity::Uid     m_identifier    = INVALID;

public:
    const static Uid INVALID;
};

// a ComponentHandle is a wrapper around an instance of a component,
// it provides safe access to components. the handle will be invalidated
// under the following conditions:
// 1. if a component is removed from its host entity;
// 2. if its host entity is destroyed.
template<typename T> struct ComponentHandle
{
    typedef T ComponentType;

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

    bool operator == (const ComponentHandle<T> &rh) const;
    bool operator != (const ComponentHandle<T> &rh) const;

private:
    friend class EntityManager;
    ComponentHandle(EntityManager*, Entity::Uid);

    EntityManager*  m_manager   = nullptr;
    Entity::Uid     m_ent       = Entity::INVALID;
};

struct ComponentTraits
{
    typedef int32_t Class;

    // component memory is always managed by entity manager.
    void operator delete(void* p);
    void operator delete[](void *p);

    static Class s_class_counter;
};

template<typename T> struct Component : public ComponentTraits
{
    typedef ComponentHandle<T> Handle;

protected:
    friend class EntityManager;
    static Class get_class();
};

struct EntityManager
{
    bool    is_valid(Entity::Uid) const;
    void    assert_valid(Entity::Uid) const;
    Entity  create();
    Entity  get(Entity::Uid);
    void    erase(Entity::Uid);
    size_t  size();
    // void    reset();

    template<typename T, typename ... Args> ComponentHandle<T> add_component(Entity::Uid, Args && ... args);
    template<typename T> ComponentHandle<T> get_component(Entity::Uid);
    template<typename T> T* get_component_ptr(Entity::Uid);
    template<typename T> const T* get_component_ptr(Entity::Uid) const;
    template<typename T> void remove_component(Entity::Uid);
    template<typename T> bool has_component(Entity::Uid);
    ComponentMask get_components_mask(Entity::Uid);

private:
    void accomodate_entity(uint32_t);
    template<typename T> Pool<T>* accomodate_component();

    uint32_t m_index_counter = 0;
    // each element in componets_pool corresponds to a Pool for a Component.
    // the index into the vector is the Component::get_class();
    std::vector<GenericPool*> m_components_pool;
    // bitmask of components associated with each entity.
    // the index into the vector is the Entity::Uid.
    std::vector<ComponentMask> m_components_mask;
    // entity version numbers. incremented each time an entity is destroyed
    std::vector<uint32_t> m_versions;
    // list of available entity slots
    std::vector<uint32_t> m_freeslots;
};

#include "ecs.inl"
NS_FLOW2D_END