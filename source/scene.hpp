// @date 2016/05/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include "flow2d.hpp"
#include "system.hpp"

NS_FLOW2D_BEGIN

struct SceneSystem : public SystemTrait<SceneSystem>
{
    SceneSystem();
    ~SceneSystem() override;
    void attach(SystemManager&) override;
    void detach(SystemManager&) override;
    void update(SystemManager&, float) override;

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