#include <iostream>
#include <lemon-toolkit.hpp>

using namespace lemon;
using namespace lemon::graphics;

const char* vs =
"#version 330 core\n"
"layout(location = 0) in vec3 position;\n"
"layout(location = 1) in vec2 uv;\n"
"uniform mat4 MVP;\n"
"out vec2 v_uv;\n"
"void main(){\n"
"    gl_Position = vec4(position, 1.0)*MVP;\n"
"    v_uv = uv;\n"
"}\n";

const char* ps =
"#version 330 core\n"
"in vec2 v_uv;\n"
"out vec3 color;\n"
"uniform sampler2D sampler;\n"
"void main(){\n"
"  color = texture(sampler, v_uv).rgb;\n"
"}\n";

struct Example : public Application
{
    void start() override
    {
        auto& engine = get_subsystem<Engine>();
        auto& collection = get_subsystem<res::ArchiveCollection>();
        auto& cache = get_subsystem<res::ResourceCache>();
        auto& device = get_subsystem<graphics::Device>();

        engine.set_time_smoothing_step(10);
        engine.set_max_fps(25);

        fs::set_current_directory("../../test");
        collection.add_search_path("resource");

        ////
        image = cache.get<res::Image>("view.jpeg");
        texture = device.spawn<Texture2D>();
        texture->restore(image, TextureFormat::RGB);
        texture->set_mipmap(true);
        texture->update_parameters();

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
        vb = device.spawn<VertexBuffer>();
        vb->restore(vertices, 3, { {3, ElementFormat::FLOAT}, {2, ElementFormat::FLOAT} });

        shader = device.spawn<Shader>();
        shader->restore(vs, ps);
        shader->set_vertex_attribute("position", vb, 0);
        shader->set_vertex_attribute("uv", vb, 1);
        shader->set_texture("sampler", texture);

        ///
        subscribe<Example, EvtUpdate>();
        subscribe<Example, EvtRender>();
    }

    void receive(const EvtUpdate& evt)
    {
        auto& input = get_subsystem<Input>();

        if( input.get_key_press(KeyboardCode::D) )
            x += 0.1f;
        if( input.get_key_press(KeyboardCode::A) )
            x -= 0.1f;
        if( input.get_key_press(KeyboardCode::W) )
            y += 0.1f;
        if( input.get_key_press(KeyboardCode::S) )
            y -= 0.1f;
    }

    void receive(const EvtRender& evt)
    {
        auto& device = get_subsystem<graphics::Device>();
        auto size = device.get_window_size();

        auto transform = math::look_at(math::Vector3f{0, 0, -10+x*10}, math::Vector3f {0, 0, 0}, math::Vector3f {0, 1, 0});
        transform *= math::perspective(45.f, (float)size[0]/(float)size[1], 0.1f, 100.f);
        shader->set_uniform4fm("MVP", transform);
        shader->use();

        device.clear(graphics::ClearOption::COLOR | graphics::ClearOption::DEPTH, {0.75, 0.75, 0.75}, 1.f);
        device.draw(PrimitiveType::TRIANGLES, 0, 3);
    }

    res::Image::ptr     image;
    Texture2D::ptr      texture;
    VertexBuffer::ptr   vb;
    Shader::ptr         shader;

    float x, y;
};

DEFINE_APPLICATION_MAIN(Example);