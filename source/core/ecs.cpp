// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#include <core/ecs.hpp>

NS_LEMON_CORE_BEGIN

Entity::Entity(EntityComponentSystem& world, Handle handle)
: _world(world), handle(handle)
{
    memset(_table, 0, sizeof(_table));
}

Entity::Entity(EntityComponentSystem& world)
: _world(world), handle()
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
    if( auto handle = _entities.create(*this) )
    {
        auto object = _entities.fetch(handle);
        *const_cast<Handle*>(&object->handle) = handle;
        return object;
    }

    return nullptr;
}

void EntityComponentSystem::free(Entity* object)
{
    if( object != nullptr )
        free(object->handle);
}

void EntityComponentSystem::free(Handle handle)
{
    if( auto object = _entities.fetch(handle) )
    {
        Entity tmp(*object);
        if( _entities.free(handle) )
        {
            for( size_t i = 0; i < _resolvers.size(); i++ )
            {
                if( _resolvers[i] != nullptr && tmp._mask.test(i) )
                    _resolvers[i]->free(tmp._table[i]);
            }
        }
    }
}

void EntityComponentSystem::free_all()
{
    for( auto handle : _entities )
    {
        auto entity = _entities.fetch(handle);
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
    _entities.clear();
}

NS_LEMON_CORE_END