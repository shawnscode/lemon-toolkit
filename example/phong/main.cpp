#include <iostream>
#include <lemon-toolkit.hpp>

using namespace lemon;
using namespace lemon::graphics;

float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};


struct Example : public Application
{
    void start() override
    {
        auto engine = core::get_subsystem<Engine>();
        auto collection = core::get_subsystem<res::ArchiveCollection>();
        auto cache = core::get_subsystem<res::ResourceCache>();
        auto frontend = core::get_subsystem<Renderer>();

        engine->set_time_smoothing_step(10);
        engine->set_max_fps(30);

        fs::set_current_directory("../../example");
        collection->add_search_path("resource");

        ////
        phong = cache->get<res::Shader>("shader/phong.shader");
        lamp = cache->get<res::Shader>("shader/color.shader");

        ////
        auto layout = VertexLayout::make(
            VertexAttribute(VertexAttribute::POSITION, VertexElementFormat::FLOAT, 3),
            VertexAttribute(VertexAttribute::NORMAL, VertexElementFormat::FLOAT, 3)
        );

        vb = frontend->create<VertexBuffer>(vertices, 36, layout, MemoryUsage::STATIC);
        uniform = frontend->create<UniformBuffer>();
        lamp_uniform = frontend->create<UniformBuffer>();

        auto p = frontend->get<Program>(phong->get_program_handle());
        p->set_attribute_name(VertexAttribute::POSITION, "position");
        p->set_attribute_name(VertexAttribute::NORMAL, "normal");

        p = frontend->get<Program>(lamp->get_program_handle());
        p->set_attribute_name(VertexAttribute::POSITION, "position");

        auto u = frontend->get<UniformBuffer>(uniform);
        u->set_uniform_3f("light_pos", lamp_pos);

        core::subscribe<EvtUpdate>(*this);
        core::subscribe<EvtRender>(*this);
    }

    void receive(const EvtUpdate& evt)
    {
        auto now = core::get_subsystem<Engine>()->get_time_since_launch() / std::chrono::milliseconds(1);
        lamp_color[0] = sin((float)now/1000.f * 2.0f);
        lamp_color[1] = sin((float)now/1000.f * 0.7f);
        lamp_color[2] = sin((float)now/1000.f * 1.3f);

        auto input = core::get_subsystem<Input>();

        if( input->get_key_press(KeyboardCode::D) )
            x += 0.1f;
        if( input->get_key_press(KeyboardCode::A) )
            x -= 0.1f;
        auto view_pos = math::Vector3f{-1, 0, -5+x*10};

        if( input->get_mouse_button_down(MouseCode::LEFT) )
        {
            auto delta = input->get_mouse_delta();
            rotation *= from_euler_angles({-(float)delta[1], (float)delta[0], 0});
        }

        auto window = core::get_subsystem<WindowDevice>();
        auto frontend = core::get_subsystem<Renderer>();
        auto size = window->get_window_size();

        auto projection = math::perspective(45.f, (float)size[0]/(float)size[1], 0.1f, 100.f);
        auto view = math::look_at(view_pos, math::Vector3f {0, 0, 0}, math::Vector3f {0, 1, 0});

        auto u = frontend->get<UniformBuffer>(uniform);
        u->set_uniform_4fm("projection", projection);
        u->set_uniform_4fm("view", view);
        u->set_uniform_4fm("model", hlift(to_rotation_matrix(rotation)));
        u->set_uniform_3f("light_color", lamp_color);
        u->set_uniform_3f("object_color", {1.0f, 1.0f, 1.0f});
        u->set_uniform_3f("view_pos", view_pos);

        u = frontend->get<UniformBuffer>(lamp_uniform);
        u->set_uniform_4fm("projection", projection);
        u->set_uniform_4fm("view", view);
        u->set_uniform_4fm("model", hlift(scale(lamp_scale))*translation(lamp_pos));
        u->set_uniform_3f("object_color", lamp_color);
    }

    void receive(const EvtRender& evt)
    {
        auto frontend = core::get_subsystem<Renderer>();
        frontend->clear(ClearOption::COLOR | ClearOption::DEPTH, {0.75, 0.75, 0.75}, 1.f);

        RenderDrawcall drawcall;
        drawcall.state.depth.enable = true;

        drawcall.program = phong->get_program_handle();
        drawcall.uniform_buffer = uniform;
        drawcall.vertex_buffer = vb;
        drawcall.first = 0;
        drawcall.count = 36;
        frontend->submit(RenderLayer::BACKGROUND, 0, drawcall);

        drawcall.program = lamp->get_program_handle();
        drawcall.uniform_buffer = lamp_uniform;
        drawcall.first = 0;
        drawcall.count = 36;
        frontend->submit(RenderLayer::BACKGROUND, 0, drawcall);
    }

    res::Shader::ptr phong;
    Handle uniform;
    Handle vb;

    res::Shader::ptr lamp;
    Handle lamp_uniform;
    math::Vector3f lamp_scale = {0.15f, 0.15f, 0.15f};
    math::Vector3f lamp_pos = {-1.0f, 0.5f, -1.0f};
    math::Vector3f lamp_color = {1.0f, 1.0f, 1.0f};

    math::Quaternion rotation;
    float x, y;
};

DEFINE_APPLICATION_MAIN(Example);