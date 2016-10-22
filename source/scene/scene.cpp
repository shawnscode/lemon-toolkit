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
    auto projection_matrix = camera.get_projection_matrix();
    auto view_matrix = Camera::get_view_matrix(transform);
    auto view_pos = transform.get_position(TransformSpace::WORLD);

    auto ecs = core::get_subsystem<EntityComponentSystem>();
    ecs->find_entities_with<Transform, MeshRenderer>().visit(
        [=](Entity&, Transform& transform, MeshRenderer& mesh)
        {
            auto uniforms = mesh.material->get_uniform_buffer();
            auto name = graphics::BuildinUniforms::name(graphics::BuildinUniforms::PROJECTION);
            uniforms->set_uniform_4fm(name, projection_matrix);

            name = graphics::BuildinUniforms::name(graphics::BuildinUniforms::VIEW);
            uniforms->set_uniform_4fm(name, view_matrix);

            name = graphics::BuildinUniforms::name(graphics::BuildinUniforms::VIEW_POS);
            uniforms->set_uniform_3f(name, view_pos);
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
    auto frontend = core::get_subsystem<graphics::Renderer>();
    frontend->clear(graphics::ClearOption::COLOR | graphics::ClearOption::DEPTH, {0.75, 0.75, 0.75}, 1.f);

    auto view_pos = transform.get_position(TransformSpace::WORLD);
    auto ecs = core::get_subsystem<EntityComponentSystem>();
    ecs->find_entities_with<Transform, MeshRenderer>().visit(
        [=](Entity&, Transform& transform, MeshRenderer& mesh)
        {
            graphics::RenderDrawcall drawcall;

            // TODO: fill render state associated with materials
            drawcall.state.depth.enable = true;
            drawcall.buildin.model = transform.get_model_matrix(TransformSpace::WORLD);
            drawcall.buildin.normal = transform.get_normal_matrix(TransformSpace::WORLD);

            //
            drawcall.program = *mesh.material->get_program();
            drawcall.uniform_buffer = *mesh.material->get_uniform_buffer();
            drawcall.vertex_buffer = *mesh.primitive->get_vertex_buffer();
            drawcall.primitive = mesh.primitive->get_primitive_type();
            drawcall.first = 0;

            auto ib = mesh.primitive->get_index_buffer();
            if( ib != nullptr )
            {
                drawcall.index_buffer = *ib;
                drawcall.count = mesh.primitive->get_index_size();
            }
            else
            {
                drawcall.count = mesh.primitive->get_vertex_size();
            }

            //
            auto distance = math::distance_square(view_pos, transform.get_position(TransformSpace::WORLD));
            frontend->submit(graphics::RenderLayer::BACKGROUND, distance, drawcall);
        });
}

NS_LEMON_END