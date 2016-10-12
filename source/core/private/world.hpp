// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/entity.hpp>
#include <codebase/memory/allocator.hpp>
#include <codebase/handle_set.hpp>

NS_LEMON_CORE_BEGIN

const static unsigned kFallbackComponentSize = 8;

struct World;
struct IndexedMemoryChunks : public FixedSizeAllocator
{
    IndexedMemoryChunks(World&, size_t, size_t, const internal::destructor&);
    ~IndexedMemoryChunks();

    void* malloc_with_index(Handle::index_t);
    void free_with_index(Handle::index_t);
    void* get(Handle::index_t);

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
    Handle spawn();
    // recycle entity identifier and its components
    void recycle(Handle);

    // initialize component storage and type informations
    bool register_component(TypeInfo::index_t, size_t, size_t, const internal::destructor&);
    // assign a component to object, passing through component constructor arguments
    void* add_component(TypeInfo::index_t, Handle);
    // retrieve a component assigned to object
    void* get_component(TypeInfo::index_t, Handle);
    // remove a component from object
    void remove_component(TypeInfo::index_t, Handle);

    // returns first available entity identifier with specified mask
    Handle find_first_available(ComponentMask);
    // returns next available entity identifier with specified mask
    Handle find_next_available(Handle, ComponentMask);

    // return true if component has already registered
    INLINE bool has_component_registered(TypeInfo::index_t id)
    {
        return _components.size() > id && _components[id] != nullptr;
    }

    INLINE bool has_component(TypeInfo::index_t id, Handle object)
    {
        return alive(object) && _masks[object.get_index()].test(id);
    }

    INLINE ComponentMask get_components_mask(Handle object)
    {
        return alive(object) ? _masks[object.get_index()] : ComponentMask();
    }

    INLINE size_t size() const
    {
        return _handles.size();
    }

    // returns true if the entity identifier is current alive
    INLINE bool alive(Handle object)
    {
        return _handles.is_valid(object);
    }

    // returns current alive entity identifier by index
    INLINE Handle get(Handle::index_t index)
    {
        return Handle(index, _handles.get_version(index));
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
    // incremented handle set for brand new and free slot
    ReuseableHandleSet _handles;

    // each element in componets_pool corresponds to a Pool for a Component
    // the index into the vector is the Component::type();
    std::vector<IndexedMemoryChunks*> _components;

    // bitmask of components associated with each entity
    // the index into the vector is the Handle::Uid
    std::vector<ComponentMask> _masks;
};

NS_LEMON_CORE_END