#include <iostream>
#include <lemon-toolkit.hpp>

using namespace lemon;
using namespace lemon::graphics;

struct Example : public Application
{
    math::Vector3f lamp_scale = {0.15f, 0.15f, 0.15f};
    math::Vector3f lamp_pos = {-3.0f, 3.f, 10.0f};
    math::Vector3f lamp_color = {0.34f, 0.14f, 1.0f};
    math::Vector3f view_pos = {0, 0, 0};

    core::Entity* lamp = nullptr;
    core::Entity* camera = nullptr;

    std::vector<core::Entity*> cubes;

    math::Vector3f rotations[10] = {
        { 0.2f, 1.0f, 0.3f },
        { 0.4f, 0.3f, 0.7f },
        { 0.3f, 0.7f, 0.2f },
        { 0.7f, 0.6f, 0.6f },
        { 0.1f, 1.0f, 0.3f },
        { 0.7f, 0.6f, 0.6f },
        { 0.4f, 0.3f, 0.7f },
        { 0.7f, 0.6f, 0.6f },
        { 0.3f, 0.7f, 0.2f },
        { 0.2f, 1.0f, 0.3f },
    };

    void setup() override
    {
        parse("../../example/resource/engine/defaults.json");
    }

    void start() override
    {
        auto cache = core::get_subsystem<res::ResourceCache>();
        auto ecs = core::get_subsystem<core::EntityComponentSystem>();

        ////
        {
            auto state = graphics::RenderState();
            state.depth.enable = true;

            auto shader = cache->fetch<res::Shader>("shader/phong.shader");
            auto material = cache->create<res::Material>("_buildin_/material/phong", shader);
            material->set_render_state(state);

            for( size_t i = 0; i < 10; i++ )
            {
                auto position = math::Vector3f{
                    (float)(std::rand() % 40) / 10.f - 2.f,
                    (float)(std::rand() % 40) / 10.f - 2.f,
                    (float)(std::rand() % 80) / 10.f + 4.f};
                auto scale = Vector3f { 0.5f, 0.5f, 0.5f };
                auto rotation = math::from_euler_angles({
                    (float)(std::rand() % 180),
                    (float)(std::rand() % 180),
                    (float)(std::rand() % 180)});

                auto cube = ecs->create();
                cube->add_component<Transform>(*cube, position, scale, rotation);
                cube->add_component<MeshRenderer>(material, res::Primitive::cube());
                cubes.push_back(cube);
            }
        }

        {
            auto material = cache->create<res::Material>("_buildin_/material/color", res::Shader::color());
            lamp = ecs->create();
            lamp->add_component<Transform>(*lamp, lamp_pos, lamp_scale);
            lamp->add_component<MeshRenderer>(material, res::Primitive::cube());
        }

        {
            camera = ecs->create();
            camera->add_component<Transform>(*camera, view_pos);
            auto c = camera->add_component<PerspectiveCamera>();
            auto size = core::get_subsystem<WindowDevice>()->get_window_size();
            c->set_aspect((float)size[0]/(float)size[1]);
        }

        ////
        auto shader = res::Shader::color();
        core::get_subsystem<core::EventSystem>()->subscribe<EvtRenderUpdate>(*this);
        std::cout << *cache << std::endl;
    }

    void receive(const EvtRenderUpdate& evt)
    {
        auto frontend = core::get_subsystem<graphics::RenderFrontend>();
        frontend->clear(ClearOption::COLOR, {0.25f, 0.25f, 0.25f});

        auto now = core::get_subsystem<Engine>()->get_time_since_launch() / std::chrono::milliseconds(1);

        for( size_t i = 0; i < cubes.size(); i++ )
        {
            auto rotation = math::from_euler_angles((float)now/1000.f*90.f*rotations[i]);
            cubes[i]->get_component<Transform>()->set_rotation(rotation);
        }

        {
            for( auto cube : cubes )
            {
                auto material = cube->get_component<MeshRenderer>()->material;

                graphics::UniformVariable v;
                v.set<math::Vector3f>(lamp_pos);
                material->set_uniform_variable("LightPos", v);
                v.set<math::Vector3f>(lamp_color);
                material->set_uniform_variable("LightColor", v);
                v.set<math::Vector3f>(math::Vector3f {1.0f, 1.0f, 1.0f});
                material->set_uniform_variable("ObjectColor", v);
            }
        }

        {
            graphics::UniformVariable v;
            v.set<math::Vector3f>(lamp_color);

            auto material = lamp->get_component<MeshRenderer>()->material;
            material->set_uniform_variable("ObjectColor", v);
        }
    }
};

DEFINE_APPLICATION_MAIN(Example);
