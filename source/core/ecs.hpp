// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/defines.hpp>
#include <core/event.hpp>
#include <core/typeinfo.hpp>

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

    template<typename T> bool has_components_t(Entity object);
    template<typename T1, typename T2, typename ... Args> bool has_components_t(Entity);

    template<typename T> ComponentMask get_components_mask_t();
    template<typename T1, typename T2, typename ... Args> ComponentMask get_components_mask_t();
}

// spawn a new entity identifier
Entity spawn();
template<typename T, typename ... Args> Entity spawn_with(Args&&...);
// recycle entity identifier and its components
void recycle(Entity);
// returns true if the entity identifier is current alive
bool alive(Entity);

// assign a component to object, passing through component constructor arguments
template<typename T, typename ... Args> T* add_component(Entity, Args && ... args);
// retrieve a component assigned to object
template<typename T> T* get_component(Entity);
template<typename ...Args> std::tuple<Args*...> get_components(Entity);
// remove a component from object
template<typename T> void remove_component(Entity);
// check if we have specified components
template<typename ...Args> bool has_components(Entity);
// get bitmask representation of components
ComponentMask get_components_mask(Entity);
template<typename ...Args> ComponentMask get_components_mask();

// find entities that have all of the specified components, returns a incremental iterator
ecs::view<> find_entities();
template<typename ... T> ecs::view<T...> find_entities_with();

namespace ecs
{
    // initialize/dispose entity component system
    bool initialize();
    void dispose();
    // returns the count of current alive entities
    size_t size();
    // dispose all the alive objects
    void reset();

    // initialize component storage and type informations
    template<typename T> bool register_component();
    bool has_component_registered(TypeInfo::index_type);

    // private ecs specialization to avoid ambiguous resolve
    using destructor = std::function<void(Entity, void*)>;
    bool register_component(TypeInfo::index_type, size_t, size_t, const destructor&);
    void* add_component(TypeInfo::index_type, Entity);
    void* get_component(TypeInfo::index_type, Entity);
    void remove_component(TypeInfo::index_type, Entity);
    bool has_component(TypeInfo::index_type, Entity);

    Entity::index_type find_next_available(Entity::index_type, ComponentMask, bool self = false);
    Entity get(Entity::index_type);

    namespace test_mem
    {
        size_t size(TypeInfo::index_type);
        size_t capacity(TypeInfo::index_type);
    }
}

/// dispatched events during the whole life of event manager
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

#include <core/ecs.inl>
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
