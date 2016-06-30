// @date 2016/05/28
// @author Mao Jingkai(oammix@gmail.com)

#include <core/system.hpp>

NS_FLOW2D_BEGIN

System::Type System::s_type_counter = 0;

SystemManager::SystemManager(EntityManager& ent, EventManager& evt)
: _world(ent), _dispatcher(evt)
{}

SystemManager::~SystemManager()
{
    for( auto pair : _systems )
        delete pair.second;
    _systems.clear();
}

void SystemManager::update(float dt)
{
    for( auto pair : _systems )
        pair.second->update(dt);
}

NS_FLOW2D_END