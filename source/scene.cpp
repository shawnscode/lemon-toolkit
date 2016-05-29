// @date 2016/05/29
// @author Mao Jingkai(oammix@gmail.com)

#include "scene.hpp"
#include "event.hpp"
#include "transform.hpp"

NS_FLOW2D_BEGIN

SceneSystem::SceneSystem()
{
    m_transform = new Transform();
}

SceneSystem::~SceneSystem()
{
    delete m_transform;
    m_transform = nullptr;
}

void SceneSystem::attach(SystemManager& sys)
{
    // sys.ensure<RequireComponents<Transform, SpatialNode>>();
    auto& evt = sys.get_event_manager();
    evt.subscribe<EvtComponentAdded<Transform>>(*this);
    evt.subscribe<EvtComponentRemoved<Transform>>(*this);
}

void SceneSystem::detach(SystemManager& sys)
{
    auto& evt = sys.get_event_manager();
    evt.unsubscribe<EvtComponentAdded<Transform>>(*this);
    evt.unsubscribe<EvtComponentRemoved<Transform>>(*this);
}

void SceneSystem::update(SystemManager& sys, float)
{
}

void SceneSystem::receive(const EvtComponentAdded<Transform>& evt)
{
    evt.component->set_scene(this);
    if( evt.component->get_parent() == nullptr )
        evt.component->set_parent(m_transform);
}

void SceneSystem::receive(const EvtComponentRemoved<Transform>& evt)
{
    evt.component->detach_from_parent();
}

Transform* SceneSystem::find_with_name(const char* n)
{
    return m_transform->find_with_name(n);
}

NS_FLOW2D_END