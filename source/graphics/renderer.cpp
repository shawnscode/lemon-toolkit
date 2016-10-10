// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/renderer.hpp>
#include <graphics/private/backend.hpp>

NS_LEMON_GRAPHICS_BEGIN

bool Renderer::initialize()
{
    _backend.reset(new (std::nothrow) RendererBackend());
    if ( _backend == nullptr )
        return false;

    _frame_began = false;
    return true;
}

bool Renderer::restore(SDL_Window* window)
{
    return _backend->initialize(window);
}

void Renderer::release()
{
    _backend->dispose();
}

bool Renderer::begin_frame()
{
    if( !_backend->begin_frame() )
        return false;

    _frame_began = true;
    return true;
}

void Renderer::clear(ClearOption option, const math::Color& color, float depth, unsigned stencil)
{
    ASSERT(_frame_began, "\'clear\' could only be called during rendering frame.");
    _backend->clear(option, color, depth, stencil);
}

void Renderer::submit(
    RenderLayer layer, RenderState state,
    Handle program, Handle vb, Handle ib,
    uint32_t depth, uint32_t start, uint32_t num)
{
    ASSERT(_frame_began, "\'submit\' could only be called during rendering frame.");

    // RenderDrawcall drawcall;
    // drawcall.state = state;
    // drawcall.program = program;
    // drawcall.vertex_buffer = vb;
    // drawcall.index_buffer = ib;
    // drawcall.start = start;
    // drawcall.vertex_count = num;
    // drawcall.sort_value = SortValue::encode(layer, program, depth);

    {
        // std::unique_lock<std::mutex> L(CTX(mutex));
        // CTX(drawcalls).push_back(drawcall);
    }
}

void Renderer::flush()
{
    ASSERT(_frame_began, "\'flush\' could only be called during rendering frame.");

    // std::unique_lock<std::mutex> L(_context->frame_mutex);
    // _context->frame.sort();
    // for( auto& drawcall : _context->frame )
    // {
        // auto program = _context->programs.get(drawcall.program);
        // auto ub = _context->uniform_buffers.get(drawcall.uniform_buffer);
        // auto vb = CTX(vertex_buffers).get(drawcall.vertex_buffer);
        // auto ib = CTX(index_buffers).get(drawcall.index_buffer);

        // auto hprogram = program->get_opengl_handle();

        // if( _supportVao )
        // {
        //     program->get_vao_handle();
        // }

        // _backend.set_shader(program->get_opengl_handle());
        // _backend.set_vertex_attributes(vb->get_attributes());
        // _backend.set_


        // ub->get_opengl_handle();
        // vb->get_opengl_handle();
        // ib->get_opengl_handle();
    // }
    // _context->frame.clear();
}

void Renderer::end_frame()
{
    ASSERT(_frame_began, "\'end_frame\' could only be called during rendering frame.");

    flush();
    _backend->end_frame();
    _frame_began = false;
}

NS_LEMON_GRAPHICS_END