// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <core/subsystem.hpp>
#include <scene/camera.hpp>
#include <scene/transform.hpp>
#include <engine/engine.hpp>

NS_LEMON_BEGIN

using namespace core;

struct Scene : public core::Subsystem
{
    bool initialize() override;
    void dispose() override;

    void receive(const EvtRenderUpdate&);
    void receive(const EvtRender&);

protected:
    void update_with_camera(Transform& transform, Camera& camera);
    void draw_with_camera(Transform& transform, Camera& camera);
};

NS_LEMON_END