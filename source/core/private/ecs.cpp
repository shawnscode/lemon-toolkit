// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#include <core/ecs.hpp>

NS_LEMON_CORE_BEGIN

bool EntityComponentSystem::initialize()
{
    return true;
}

void EntityComponentSystem::dispose()
{
    free_all();
}

Handle EntityComponentSystem::create()
{
    auto handle = _entities.malloc();
    auto block = _entities.get(handle);

    if( block == nullptr )
        return handle;

    ::new (block) Entity(*this);
    return handle;
}

Entity* EntityComponentSystem::get(Handle handle)
{
    return static_cast<Entity*>(_entities.get(handle));
}

void EntityComponentSystem::free(Handle handle)
{
    auto entity = _entities.get_t(handle);
    if( entity == nullptr )
        return;

    for( size_t i = 0; i < _component_sets.size(); i++ )
    {
        if( entity->_mask.test(i) )
        {
            auto component = _component_sets[i]->get(entity->_components[i]);
            _destructors[i](handle, component);
            _component_sets[i]->free(entity->_components[i]);
        }
    }

    return _entities.free(handle);
}

void EntityComponentSystem::free_all()
{
    for( size_t i = 0; i < _component_sets.size(); i++ )
    {
        auto pool = _component_sets[i];
        if( pool == nullptr )
            continue;

        for( auto handle : _entities )
        {
            auto entity = _entities.get_t(handle);
            if( entity->_mask.test(i) )
                _destructors[i](handle, pool->get(entity->_components[i]));
        }

        delete pool;
    }

    _entities.free_all();
    _component_sets.clear();
}

NS_LEMON_CORE_END