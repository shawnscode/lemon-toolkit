// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/private/ecs.hpp>

NS_LEMON_CORE_BEGIN

// returns world instance
EntityComponentSystem& world();

// spawn a new entity and returns handle as identifier
Handle create();

// recycle components and associated to this entity, and invalidate handle
void free(Handle handle);

// returns Entity associated with handle
Entity* get(Handle handle);

// returns true if the entity handle is current alive
bool is_valid(Handle handle);

// assign a component to object, passing through component constructor arguments
template<typename T, typename ... Args> T* add_component(Handle handle, Args&& ... args);
template<typename T, typename ... Args> Handle create_with(Args&& ... args);

// retrieve a component assigned to object
template<typename T> T* get_component(Handle handle);
template<typename ... Args> std::tuple<Args*...> get_components(Handle handle);

// remove a component from object
template<typename T> void remove_component(Handle handle);

// check if we have specified components
template<typename T> bool has_component(Handle handle);
template<typename ... Args> bool has_components(Handle handle);

// get bitmask representation of handled components
ComponentMask get_components_mask(Handle handle);
template<typename ... Args> ComponentMask get_components_mask();

// find entities that have all of the specified components, returns a incremental iterator
EntityComponentSystem::view_t<> find_entities();
template<typename ... Args> EntityComponentSystem::view_t<Args...> find_entities_with();

//
// implementation of entity component system
INLINE Handle create()
{
    return world().create();
}

INLINE void free(Handle handle)
{
    world().free(handle);
}

INLINE Entity* get(Handle handle)
{
    return world().get(handle);
}

INLINE bool is_valid(Handle handle)
{
    return world().is_valid(handle);
}

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

template<typename T> void remove_component(Handle handle)
{
    Entity* object = world().get(handle);
    if( object != nullptr )
        object->remove_component<T>();
}

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

INLINE ComponentMask get_components_mask(Handle handle)
{
    Entity* object = world().get(handle);
    return object == nullptr ? ComponentMask() : object->get_components_mask();
}

template<typename ... Args> ComponentMask get_components_mask()
{
    return Entity::get_components_mask<Args...>();
}

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