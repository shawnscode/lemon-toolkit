// @date 2016/10/09
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <core/ecs.hpp>
#include <resource/material.hpp>
#include <resource/primitive.hpp>

NS_LEMON_BEGIN

struct MeshRenderer : core::Component
{
    MeshRenderer(res::Material::ptr material, res::Primitive::ptr primitive)
    : material(material), primitive(primitive)
    {}

    int8_t layer = 0;
    bool visible = true;
    res::Material::ptr material;
    res::Primitive::ptr primitive;
};

NS_LEMON_END