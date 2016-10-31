// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#include <scene/scene.hpp>
#include <core/event.hpp>
#include <scene/mesh.hpp>
#include <math/vector.hpp>
#include <graphics/frontend.hpp>

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
            graphics::UniformVariable v;
            v.set<math::Matrix4f>(projection_matrix);
            mesh.material->set_uniform_variable("lm_ProjectionMatrix", v);

            v.set<math::Matrix4f>(view_matrix);
            mesh.material->set_uniform_variable("lm_ViewMatrix", v);

            v.set<math::Vector3f>(view_pos);
            mesh.material->set_uniform_variable("lm_ViewPos", v);
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
    auto frontend = core::get_subsystem<graphics::RenderFrontend>();
    frontend->clear(graphics::ClearOption::COLOR | graphics::ClearOption::DEPTH, {0.75, 0.75, 0.75}, 1.f);

    auto ecs = core::get_subsystem<EntityComponentSystem>();
    ecs->find_entities_with<Transform, MeshRenderer>().visit(
        [=](Entity&, Transform& transform, MeshRenderer& mesh)
        {
            graphics::RenderDrawCall drawcall;

            drawcall.program = mesh.material->get_shader()->get_video_uid();
            drawcall.state = mesh.material->get_video_state();
            drawcall.buffer_vertex = mesh.primitive->get_video_vertex_buffer();
            drawcall.buffer_index = mesh.primitive->get_video_index_buffer();
            drawcall.shared_uniforms = mesh.material->get_video_uniforms();

            graphics::UniformVariable v;
            auto uniforms = frontend->allocate_uniform_buffer(2);
            v.set<math::Matrix4f>(transform.get_model_matrix(TransformSpace::WORLD));
            frontend->update_uniform_buffer(uniforms, "lm_ModelMatrix", v);
            v.set<math::Matrix3f>(transform.get_normal_matrix(TransformSpace::WORLD));
            frontend->update_uniform_buffer(uniforms, "lm_NormalMatrix", v);
            drawcall.uniforms = uniforms;

            drawcall.first = 0;
            if( mesh.primitive->get_video_index_buffer().is_valid() )
                drawcall.num = mesh.primitive->get_index_size();
            else
                drawcall.num = mesh.primitive->get_vertex_size();

            frontend->submit(drawcall);
        });
}

NS_LEMON_END