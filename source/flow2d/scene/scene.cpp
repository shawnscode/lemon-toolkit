// @date 2016/05/29
// @author Mao Jingkai(oammix@gmail.com)

#include <flow2d/core/event.hpp>
#include <flow2d/scene/scene.hpp>
#include <flow2d/scene/transform.hpp>

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

void SceneSystem::on_attach()
{
    dispatcher().subscribe<EvtComponentAdded<Transform>>(*this);
    dispatcher().subscribe<EvtComponentRemoved<Transform>>(*this);
}

void SceneSystem::on_detach()
{
    dispatcher().unsubscribe<EvtComponentAdded<Transform>>(*this);
    dispatcher().unsubscribe<EvtComponentRemoved<Transform>>(*this);
}

void SceneSystem::update(float)
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