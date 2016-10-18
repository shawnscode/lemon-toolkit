// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#include <core/ecs.hpp>

NS_LEMON_CORE_BEGIN

Entity::Entity(EntityComponentSystem& world, Handle handle)
: _world(world), handle(handle)
{
    memset(_table, 0, sizeof(_table));
}

bool EntityComponentSystem::initialize()
{
    return true;
}

void EntityComponentSystem::dispose()
{
    free_all();
}

Entity* EntityComponentSystem::create()
{
    Handle handle;
    {
        std::unique_lock<std::mutex> L(_mutex);
        handle = _entities.malloc();
    }

    if( !handle.is_valid() )
        return nullptr;

    return ::new (_entities.get(handle)) Entity(*this, handle);
}

void EntityComponentSystem::free(Entity* object)
{
    if( object != nullptr )
        free(object->handle);
}

void EntityComponentSystem::free(Handle handle)
{
    auto entity = _entities.get_t(handle);
    if( entity == nullptr )
        return;

    for( size_t i = 0; i < _resolvers.size(); i++ )
    {
        if( _resolvers[i] != nullptr && entity->_mask.test(i) )
            _resolvers[i]->free(entity->_table[i]);
    }

    {
        std::unique_lock<std::mutex> L(_mutex);
        _entities.free(handle);
    }
}

void EntityComponentSystem::free_all()
{
    for( auto handle : _entities )
    {
        auto entity = _entities.get_t(handle);
        for( size_t i = 0; i < _resolvers.size(); i++ )
        {
            if( _resolvers[i] == nullptr || !entity->_mask.test(i) )
                continue;
            _resolvers[i]->free(entity->_table[i]);
        }
    }

    for( size_t i = 0; i < _resolvers.size(); i++ )
    {
        if( _resolvers[i] == nullptr )
            continue;

        delete _resolvers[i];
    }

    _resolvers.clear();
    _entities.free_all();
}

NS_LEMON_CORE_END