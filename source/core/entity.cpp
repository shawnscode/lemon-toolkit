// @date 2016/05/22
// @author Mao Jingkai(oammix@gmail.com)

#include <core/entity.hpp>

NS_FLOW2D_BEGIN

void Component::operator delete(void *)
{
    ASSERT(false, "[ECS] component memory is always managed by the EntityManager.");
}

void Component::operator delete[](void *)
{
    ASSERT(false, "[ECS] component memory is always managed by the EntityManager.");
}

EntityManager::~EntityManager()
{
    reset();
}

void EntityManager::reset()
{
    // free memory usages
    for( auto p : _components_pool )
        if( p != nullptr )
            delete p;

    _incremental_index = 0;
    _components_pool.clear();
    _components_mask.clear();
    _versions.clear();
    _freeslots.clear();
}

Entity EntityManager::spawn()
{
    Entity::index_type index, version;
    if( _freeslots.empty() )
    {
        index = _incremental_index++;
        accomodate_entity(index);
        ASSERT( _incremental_index != Entity::invalid,
            "too much entities,"
            "please considering change the representation of Entity::index_type." );
        _versions[index] = 1;
    }
    else
    {
        index = _freeslots.back();
        _freeslots.pop_back();
        _versions[index]++;
    }

    version = _versions[index];
    auto object = Entity(index, version);
    _dispatcher.emit<EvtEntityCreated>(object);
    return object;
}

Entity EntityManager::clone(Entity source)
{
    if( !is_alive(source) )
    {
        LOGW("cloned from a non-alive entity.");
        return spawn();
    }

    auto dest   = spawn();
    auto mask   = get_components_mask(source);

    for( auto i=0; i<_components_pool.size(); i++ )
    {
        auto p = _components_pool[i];
        if( p != nullptr && mask.test(i) )
            p->clone(*this, dest, source);
    }

    return dest;
}

void EntityManager::dispose(Entity object)
{
    if( !is_alive(object) )
    {
        LOGW("trying to dispose a nonexistent entity.");
        return;
    }

    _dispatcher.emit<EvtEntityDisposed>(object);

    const auto mask = _components_mask[object._index];
    for( auto i=0; i<_components_pool.size(); i++ )
    {
        auto p = _components_pool[i];
        if( p != nullptr && mask.test(i) )
            p->dispose(*this, object);
    }

    _components_mask[object._index].reset();
    _versions[object._index]++;

    ASSERT( _versions[object._index] != Entity::invalid,
        "too much reusages of this entity,"
        "please considering change the representation of Entity::index_type." );
    _freeslots.push_back(object._index);
}

NS_FLOW2D_END