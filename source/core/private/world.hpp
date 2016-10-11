// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/entity.hpp>
#include <codebase/memory/allocator.hpp>

NS_LEMON_CORE_BEGIN

const static unsigned kFallbackComponentSize = 8;

struct World;
struct IndexedMemoryChunks : public FixedSizeAllocator
{
    IndexedMemoryChunks(World&, size_t, size_t, const internal::destructor&);
    ~IndexedMemoryChunks();

    void* malloc_with_index(Entity::index_t);
    void free_with_index(Entity::index_t);
    void* get(Entity::index_t);

protected:
    World& _world;
    bool _fallback = false;
    internal::destructor _destructor;
    unsigned _redirect[kFallbackComponentSize];
    unsigned _top = kFallbackComponentSize;
    std::vector<void*> _objects;
};

struct World
{
    // initialize entity component system
    bool initialize();
    // dispose all the alive objects
    void dispose();

    // spawn a new entity identifier
    Entity spawn();
    // recycle entity identifier and its components
    void recycle(Entity);

    // initialize component storage and type informations
    bool register_component(TypeInfo::index_t, size_t, size_t, const internal::destructor&);
    // assign a component to object, passing through component constructor arguments
    void* add_component(TypeInfo::index_t, Entity);
    // retrieve a component assigned to object
    void* get_component(TypeInfo::index_t, Entity);
    // remove a component from object
    void remove_component(TypeInfo::index_t, Entity);

    // return next available entity identifier with specified mask
    Entity find_next_available(Entity::index_t index, ComponentMask mask, bool self);

    // return true if component has already registered
    INLINE bool has_component_registered(TypeInfo::index_t id)
    {
        return _components.size() > id && _components[id] != nullptr;
    }

    INLINE bool has_component(TypeInfo::index_t id, Entity object)
    {
        return alive(object) && _masks[object.get_index()].test(id);
    }

    INLINE ComponentMask get_components_mask(Entity object)
    {
        return alive(object) ? _masks[object.get_index()] : ComponentMask();
    }

    INLINE size_t size() const
    {
        return _incremental_index - _freeslots.size();
    }

    // returns true if the entity identifier is current alive
    INLINE bool alive(Entity object)
    {
        return
            object.get_index() < _versions.size() &&
            object.get_version() == _versions[object.get_index()];
    }

    // returns current alive entity identifier by index
    INLINE Entity get(Entity::index_t index)
    {
        return index < _versions.size() && (_versions[index] & 0x1) == 1 ?
            Entity(index, _versions[index]) : Entity();
    }

    INLINE size_t size(TypeInfo::index_t id)
    {
        return (_components.size() > id && _components[id] != nullptr) ?
            _components[id]->size() : 0;
    }

    INLINE size_t capacity(TypeInfo::index_t id)
    {
        return (_components.size() > id && _components[id] != nullptr) ?
            _components[id]->capacity() : 0;   
    }
    
protected:
    // incremented entity index for brand new and free slot
    Entity::index_t _incremental_index = 0;

    // each element in componets_pool corresponds to a Pool for a Component
    // the index into the vector is the Component::type();
    std::vector<IndexedMemoryChunks*> _components;

    // bitmask of components associated with each entity
    // the index into the vector is the Entity::Uid
    std::vector<ComponentMask> _masks;

    // entity version numbers. incremented each time an entity is destroyed
    std::vector<Entity::index_t> _versions;

    // list of available entity slots
    std::vector<Entity::index_t> _freeslots;
};

NS_LEMON_CORE_END