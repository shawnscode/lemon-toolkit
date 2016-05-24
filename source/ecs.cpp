#include "ecs.hpp"

NS_FLOW2D_BEGIN

const Entity::Uid Entity::INVALID;

Entity::Entity(EntityManager* manager, Entity::Uid id)
: m_manager(manager), m_identifier(id) {}

EntityIterator::EntityIterator(EntityManager* manager, uint32_t index)
: m_manager(manager), m_capacity(manager->capacity()), m_current_cursor(index)
{ next(); }

EntityIterator::EntityIterator(EntityManager* manager, const ComponentMask mask, uint32_t index)
: m_manager(manager), m_capacity(manager->capacity()), m_current_cursor(index), m_mask(mask)
{ next(); }

EntityView::EntityView(EntityManager* manager, ComponentMask mask)
: m_manager(manager), m_mask(mask)
{}

Component::Class Component::s_class_counter = 0;

void Component::operator delete(void *)
{
    assert(false && "[ECS] component memory is always managed by the EntityManager.");
}

void Component::operator delete[](void *)
{
    assert(false && "[ECS] component memory is always managed by the EntityManager.");
}

EntityManager::~EntityManager()
{
    reset();
}

void EntityManager::reset()
{
    // free components with destructor
    for( auto i=0; i<m_components_mask.size(); i++ )
    {
        const auto mask = m_components_mask[i];
        for( auto j=0; j<m_components_pool.size(); j++ )
        {
            auto p = m_components_pool[j];
            if( p != nullptr && mask.test(j) ) p->erase(i);
        }
    }

    // free memory usages
    for( auto p : m_components_pool )
        if(p != nullptr) delete p;

    m_index_counter = 0;
    m_components_pool.clear();
    m_components_mask.clear();
    m_versions.clear();
    m_freeslots.clear();
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

Entity EntityManager::create_from(Entity rh)
{
    rh.assert_valid();

    auto clone  = create();
    auto mask   = rh.get_components_mask();
    // for( auto i=0; i<m_components_pool.size(); i++ )
    // {
    //     auto p = m_components_pool[i];
    //     if( p && mask.test(i) )
    //         clone.add_component(rh.get_component().get());
    // }

    return clone;
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