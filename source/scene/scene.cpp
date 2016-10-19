// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/scene.hpp>
#include <core/event.hpp>
#include <graphics/renderer.hpp>
#include <scene/mesh.hpp>
#include <math/vector.hpp>

NS_LEMON_BEGIN

using namespace core;

bool Scene::initialize()
{
    core::get_subsystem<EventSystem>()->subscribe<EvtRenderUpdate>(this);
    core::get_subsystem<EventSystem>()->subscribe<EvtRender>(this);
    return true;
}

void Scene::dispose()
{
    core::get_subsystem<EventSystem>()->unsubscribe<EvtRenderUpdate>(this);
    core::get_subsystem<EventSystem>()->unsubscribe<EvtRender>(this);
}

void Scene::receive(const EvtRenderUpdate& evt)
{
    auto ecs = core::get_subsystem<EntityComponentSystem>();

    std::vector<std::tuple<Transform*, Camera*>> cameras;
    ecs->find_entities_with<Transform, PerspectiveCamera>().collect(cameras);
    ecs->find_entities_with<Transform, OrthoCamera>().collect(cameras);

    for( auto camera : cameras )
        update_with_camera(*std::get<0>(camera), *std::get<1>(camera));
}

void Scene::update_with_camera(Transform& transform, Camera& camera)
{
    auto projection = camera.get_projection_matrix();
    auto view = Camera::get_view_matrix(transform);
    auto view_pos = transform.get_position(TransformSpace::WORLD);
    auto ecs = core::get_subsystem<EntityComponentSystem>();

    ecs->find_entities_with<Transform, MeshRenderer>().visit(
        [=](Entity&, Transform& transform, MeshRenderer& mesh)
        {
            auto uniform = res::Material::name(res::Material::BuildinUniform::PROJECTION);
            if( mesh.material->has_uniform(uniform) )
                mesh.material->set_uniform_4fm(uniform, projection);

            uniform = res::Material::name(res::Material::BuildinUniform::VIEW);
            if( mesh.material->has_uniform(uniform) )
                mesh.material->set_uniform_4fm(uniform, view);

            uniform = res::Material::name(res::Material::BuildinUniform::VIEW_POS);
            if( mesh.material->has_uniform(uniform) )
                mesh.material->set_uniform_3f(uniform, view_pos);

            // if( lights.size() <= 0 )
            //     continue;

            // if( mesh.material->has_uniform(Material::BuildinUniform::LIGHT_POS) )
            // {
            //     auto t = lights[0]->get_component<Transform>();
            //     auto light_pos = t->get_position(TransformSpace::WORLD);
            //     uniform->set_uniform_3f(Material::name(Material::BuildinUniform::LIGHT_POS), light_pos);
            // }

            // if( mesh.material->has_uniform(Material::BuildinUniform::LIGHT_COLOR) )
            //     uniform->set_uniform_3f(Material::name(Material::BuildinUniform::LIGHT_POS), lights[0]->color);
        });
}

void Scene::receive(const EvtRender& evt)
{
    auto ecs = core::get_subsystem<EntityComponentSystem>();

    std::vector<std::tuple<Transform*, Camera*>> cameras;
    ecs->find_entities_with<Transform, PerspectiveCamera>().collect(cameras);
    ecs->find_entities_with<Transform, OrthoCamera>().collect(cameras);

    for( auto camera : cameras )
        draw_with_camera(*std::get<0>(camera), *std::get<1>(camera));
}

void Scene::draw_with_camera(Transform& transform, Camera& camera)
{
    auto view_pos = transform.get_position(TransformSpace::WORLD);

    auto frontend = core::get_subsystem<graphics::Renderer>();
    frontend->clear(graphics::ClearOption::COLOR | graphics::ClearOption::DEPTH, {0.75, 0.75, 0.75}, 1.f);

    auto ecs = core::get_subsystem<EntityComponentSystem>();
    ecs->find_entities_with<Transform, MeshRenderer>().visit(
        [=](Entity&, Transform& transform, MeshRenderer& mesh)
        {
            auto distance = math::distance_square(view_pos, transform.get_position(TransformSpace::WORLD));
            graphics::RenderDrawcall drawcall;
            drawcall.state.depth.enable = true;
            drawcall.model = transform.to_matrix(TransformSpace::WORLD);
            drawcall.program = mesh.material->get_program_handle();
            drawcall.uniform_buffer = mesh.material->get_uniform_handle();
            drawcall.vertex_buffer = mesh.primitive->get_vertex_handle();
            drawcall.index_buffer = mesh.primitive->get_index_handle();
            drawcall.primitive = mesh.primitive->get_type();
            drawcall.first = 0;
            drawcall.count = mesh.primitive->get_vertex_count();
            frontend->submit(graphics::RenderLayer::BACKGROUND, distance, drawcall);
        });
}

NS_LEMON_END