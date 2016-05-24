#include "ecs.hpp"

NS_FLOW2D_BEGIN

const Entity::Uid Entity::INVALID;

Entity::Entity(EntityManager* manager, Entity::Uid id)
: m_manager(manager), m_identifier(id) {}

ComponentTraits::Class ComponentTraits::s_class_counter = 0;

void ComponentTraits::operator delete(void *)
{
    assert(false && "[ECS] component memory is always managed by the EntityManager.");
}

void ComponentTraits::operator delete[](void *)
{
    assert(false && "[ECS] component memory is always managed by the EntityManager.");
}

Entity EntityManager::create()
{
    uint32_t index, version;
    if( m_freeslots.empty() )
    {
        index = m_index_counter++;
        accomodate_entity(index);
        version = m_versions[index] = 1;
    }
    else
    {
        index = m_freeslots.back();
        m_freeslots.pop_back();
        version = m_versions[index];
    }

    auto entity = Entity(this, Entity::Uid(index, version));
    return entity;
}

void EntityManager::erase(Entity::Uid id)
{
    assert_valid(id);

    const uint32_t index = id.index();
    const auto mask = m_components_mask[index];

    for( auto i=0; i<m_components_pool.size(); i++ )
    {
        auto p = m_components_pool[i];
        if( p && mask.test(i) ) p->erase(index);
    }

    m_components_mask[index].reset();
    m_versions[index] ++;
    m_freeslots.push_back(index);
}

NS_FLOW2D_END