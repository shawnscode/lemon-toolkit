// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

#include <flow2d/core/system.hpp>

NS_FLOW2D_BEGIN

System::Type System::s_type_counter = 0;

SystemManager::SystemManager(EntityManager& ent, EventManager& evt)
: m_world(ent), m_dispatcher(evt)
{}

SystemManager::~SystemManager()
{
    for( auto pair : m_systems )
        delete pair.second;
    m_systems.clear();
}

void SystemManager::update(float dt)
{
    for( auto pair : m_systems )
        pair.second->update(dt);
}

NS_FLOW2D_END