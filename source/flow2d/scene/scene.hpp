// @date 2016/05/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <flow2d/forward.hpp>
#include <flow2d/core/system.hpp>

NS_FLOW2D_BEGIN

struct SceneSystem : public SystemTrait<SceneSystem>
{
    SceneSystem();
    ~SceneSystem() override;
    void on_attach() override;
    void on_detach() override;
    void update(float) override;

    // event receivers
    void receive(const EvtComponentAdded<Transform>&);
    void receive(const EvtComponentRemoved<Transform>&);

    Transform* find_with_name(const char* n);
    Transform* root() { return m_transform; }

protected:
    // root transform of the whole scene
    Transform* m_transform;
};

NS_FLOW2D_END