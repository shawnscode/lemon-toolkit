// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/private/ecs.hpp>

NS_LEMON_CORE_BEGIN

// returns world instance
EntityComponentSystem& world();

// spawn a new entity and returns handle as identifier
INLINE Handle create()
{
    return world().create();
}

// recycle components and associated to this entity, and invalidate handle
INLINE void free(Handle handle)
{
    world().free(handle);
}

// returns Entity associated with handle
INLINE Entity* get(Handle handle)
{
    return world().get(handle);
}

// returns true if the entity handle is current alive
INLINE bool is_valid(Handle handle)
{
    return world().is_valid(handle);
}

// assign a component to object, passing through component constructor arguments
template<typename T, typename ... Args> T* add_component(Handle handle, Args&& ... args)
{
    Entity* object = world().get(handle);
    return object == nullptr ? nullptr : object->add_component<T>(std::forward<Args>(args)...);
}

template<typename T, typename ... Args> Handle create_with(Args&& ... args)
{
    auto handle = world().create();
    add_component<T>(handle, std::forward<Args>(args)...);
    return handle;
}

// retrieve a component assigned to object
template<typename T> T* get_component(Handle handle)
{
    Entity* object = world().get(handle);
    return object == nullptr ? nullptr : object->get_component<T>();
}

template<typename ... Args> std::tuple<Args*...> get_components(Handle handle)
{
    Entity* object = world().get(handle);
    return object == nullptr ? nullptr : object->get_components<Args...>();
}

// remove a component from object
template<typename T> void remove_component(Handle handle)
{
    Entity* object = world().get(handle);
    if( object != nullptr )
        object->remove_component<T>();
}

// check if we have specified components
template<typename T> bool has_component(Handle handle)
{
    Entity* object = world().get(handle);
    return object == nullptr ? false : object->has_component<T>();
}

template<typename ... Args> bool has_components(Handle handle)
{
    Entity* object = world().get(handle);
    return object == nullptr ? false : object->has_components<Args...>();
}

// get bitmask representation of handled components
INLINE ComponentMask get_components_mask(Handle handle)
{
    Entity* object = world().get(handle);
    return object == nullptr ? ComponentMask() : object->get_components_mask();
}

template<typename ... Args> ComponentMask get_components_mask()
{
    return Entity::get_components_mask<Args...>();
}

// find entities that have all of the specified components, returns a incremental iterator
INLINE EntityComponentSystem::view_t<> find_entities()
{
    return world().find_entities();
}

template<typename ... Args>
EntityComponentSystem::view_t<Args...> find_entities_with()
{
    return world().find_entities_with<Args...>();
}

NS_LEMON_CORE_END