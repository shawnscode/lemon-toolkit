#include <iostream>
#include <lemon-toolkit.hpp>

using namespace lemon;
using namespace lemon::graphics;

struct Example : public Application
{
    math::Vector3f lamp_scale = {0.35f, 0.35f, 0.35f};
    math::Vector3f lamp_pos = {-3.0f, 3.f, -3.0f};
    math::Vector3f lamp_color = {1.0f, 1.0f, 1.0f};
    math::Vector3f view_pos = {0, 0, -5};

    core::Entity* cube = nullptr;
    core::Entity* lamp = nullptr;
    core::Entity* camera = nullptr;

    void start() override
    {
        fs::set_current_directory("../../example");

        auto engine = core::get_subsystem<Engine>();
        auto collection = core::get_subsystem<res::ArchiveCollection>();
        auto cache = core::get_subsystem<res::ResourceCache>();
        auto frontend = core::get_subsystem<graphics::Renderer>();
        auto ecs = core::get_subsystem<core::EntityComponentSystem>();

        engine->set_time_smoothing_step(10);
        engine->set_max_fps(30);
        collection->add_search_path("resource");

        ////
        {
            auto shader = cache->get<res::Shader>("shader/phong.shader");
            auto material = res::Material::create("_buildin_/material/phong", shader);
            material->set_attribute_name(VertexAttribute::POSITION, "position");
            material->set_attribute_name(VertexAttribute::NORMAL, "normal");

            cube = ecs->create();
            cube->add_component<Transform>(*cube);
            cube->add_component<MeshRenderer>(material, res::Primitive::cube());
        }

        {
            auto shader = cache->get<res::Shader>("shader/color.shader");
            auto material = res::Material::create("_buildin_/material/color", shader);
            material->set_attribute_name(VertexAttribute::POSITION, "position");

            lamp = ecs->create();
            lamp->add_component<Transform>(*cube, lamp_pos, lamp_scale);
            lamp->add_component<MeshRenderer>(material, res::Primitive::cube());
        }

        {
            camera = ecs->create();
            auto t = camera->add_component<Transform>(*camera, view_pos);
            auto c = camera->add_component<PerspectiveCamera>();
            auto size = core::get_subsystem<WindowDevice>()->get_window_size();
            c->set_aspect((float)size[0]/(float)size[1]);
        }

        ////
        core::get_subsystem<core::EventSystem>()->subscribe<EvtUpdate>(*this);
        core::get_subsystem<core::EventSystem>()->subscribe<EvtRender>(*this);
        std::cout << *cache << std::endl;
    }

    void receive(const EvtUpdate& evt)
    {
        auto now = core::get_subsystem<Engine>()->get_time_since_launch() / std::chrono::milliseconds(1);
        lamp_color[0] = sin((float)now/1000.f * 2.0f);
        lamp_color[1] = sin((float)now/1000.f * 0.7f);
        lamp_color[2] = sin((float)now/1000.f * 1.3f);

        auto input = core::get_subsystem<Input>();
        if( input->get_mouse_button_down(MouseCode::LEFT) )
        {
            auto transform = cube->get_component<Transform>();
            auto rotation = transform->get_rotation(TransformSpace::WORLD);
            auto delta = input->get_mouse_delta();

            rotation *= math::from_euler_angles({-(float)delta[1], (float)delta[0], 0});
            transform->set_rotation(rotation, TransformSpace::WORLD);
        }

        {
            auto material = cube->get_component<MeshRenderer>()->material;
            material->set_uniform_3f("LightPos", lamp_pos);
            material->set_uniform_3f("LightColor", lamp_color);
            material->set_uniform_3f("ObjectColor", {1.0f, 1.0f, 1.0f});
        }

        {
            auto material = lamp->get_component<MeshRenderer>()->material;
            material->set_uniform_3f("ObjectColor", lamp_color);
        }
    }

    void receive(const EvtRender& evt) {}
};

DEFINE_APPLICATION_MAIN(Example);