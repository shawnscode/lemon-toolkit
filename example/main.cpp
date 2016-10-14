#include <iostream>
#include <lemon-toolkit.hpp>

using namespace lemon;
using namespace lemon::graphics;

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

        fs::set_current_directory("../../test");
        collection->add_search_path("resource");

        ////
        image = cache->get<res::Image>("view.jpeg");
        shader = cache->get<res::Shader>("shader/example-1.shader");

        texture = frontend->create<Texture>(image->get_data(),
            TextureFormat::RGB,
            static_cast<TexturePixelFormat>(image->get_element_format()),
            image->get_width(),
            image->get_height(),
            MemoryUsage::STATIC);

        auto t = frontend->get<Texture>(texture);
        t->set_mipmap(true);
        t->update_parameters();

        ////
        float depth = 0.f;
        float scale = 0.75f;
        float vertices[] =
        {
            -scale, -scale, depth,
            0.f, 0.f,
            scale, scale, depth,
            1.f, 1.f,
            scale, -scale, depth,
            1.0f, 0.f,
        };

        auto layout = VertexLayout::make(
             VertexAttribute(VertexAttribute::POSITION, VertexElementFormat::FLOAT, 3),
             VertexAttribute(VertexAttribute::TEXCOORD_0, VertexElementFormat::FLOAT, 2));

        vb = frontend->create<VertexBuffer>(vertices, 3, layout, MemoryUsage::STATIC);
        uniform = frontend->create<UniformBuffer>();

        auto p = frontend->get<Program>(shader->get_program_handle());
        p->set_attribute_name(VertexAttribute::POSITION, "position");
        p->set_attribute_name(VertexAttribute::TEXCOORD_0, "uv");

        auto u = frontend->get<UniformBuffer>(uniform);
        u->set_uniform_texture("sampler", texture);

        core::subscribe<EvtUpdate>(*this);
        core::subscribe<EvtRender>(*this);
    }

    void receive(const EvtUpdate& evt)
    {
        auto input = core::get_subsystem<Input>();

        if( input->get_key_press(KeyboardCode::D) )
            x += 0.1f;
        if( input->get_key_press(KeyboardCode::A) )
            x -= 0.1f;
        if( input->get_key_press(KeyboardCode::W) )
            y += 0.1f;
        if( input->get_key_press(KeyboardCode::S) )
            y -= 0.1f;

        auto window = core::get_subsystem<WindowDevice>();
        auto size = window->get_window_size();

        auto transform = math::look_at(math::Vector3f{0, 0, -10+x*10}, math::Vector3f {0, 0, 0}, math::Vector3f {0, 1, 0});
        transform *= math::perspective(45.f, (float)size[0]/(float)size[1], 0.1f, 100.f);

        auto frontend = core::get_subsystem<Renderer>();
        auto u = frontend->get<UniformBuffer>(uniform);
        u->set_uniform_4fm("MVP", transform);
    }

    void receive(const EvtRender& evt)
    {
        auto frontend = core::get_subsystem<Renderer>();
        frontend->clear(ClearOption::COLOR | ClearOption::DEPTH, {0.75, 0.75, 0.75}, 1.f);

        RenderDrawcall drawcall;
        drawcall.program = shader->get_program_handle();
        drawcall.uniform_buffer = uniform;
        drawcall.vertex_buffer = vb;
        drawcall.first = 0;
        drawcall.count = 3;
        frontend->submit(RenderLayer::BACKGROUND, 0, drawcall);
    }

    res::Image::ptr image;
    res::Shader::ptr shader;

    Handle texture;
    Handle vb;
    Handle uniform;

    float x, y;
};

DEFINE_APPLICATION_MAIN(Example);