// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/renderer.hpp>
#include <graphics/private/backend.hpp>
#include <graphics/private/state_cache.hpp>
#include <graphics/private/program.hpp>
#include <graphics/private/vertex_buffer.hpp>
#include <graphics/private/index_buffer.hpp>
#include <graphics/private/texture.hpp>
#include <graphics/private/uniform_buffer.hpp>

NS_LEMON_GRAPHICS_BEGIN

bool Renderer::initialize()
{
    _backend = new (std::nothrow) RendererBackend();
    _statecache = new (std::nothrow) RenderStateCache(*this);

    if( _backend == nullptr || _statecache == nullptr )
    {
        dispose();
        return false;
    }

    auto program_dtor = [=](Handle handle, void* p)
    {
        _statecache->free_program(handle);

        auto program = static_cast<ProgramGL*>(p);
        program->~ProgramGL();
    };

    auto vb_dtor = [=](Handle handle, void* vb)
    {
        _statecache->free_vertex_buffer(handle);

        auto vertex_buffer = static_cast<VertexBufferGL*>(vb);
        vertex_buffer->~VertexBufferGL();
    };

    if( !register_graphics_object<Program, ProgramGL, 8>(program_dtor) ||
        !register_graphics_object<Texture, TextureGL, 8>() ||
        !register_graphics_object<IndexBuffer, IndexBufferGL, 16>() ||
        !register_graphics_object<VertexBuffer, VertexBufferGL, 16>(vb_dtor) ||
        !register_graphics_object<UniformBuffer, UniformBufferGL, 16>() )
    {
        dispose();
        return false;
    }

    _frame_began = false;
    return true;
}

void Renderer::dispose()
{
    if( _backend != nullptr )
    {
        delete _backend;
        _backend = nullptr;
    }

    if( _statecache != nullptr )
    {
        delete _statecache;
        _statecache = nullptr;
    }

    for( size_t i = 0; i < _object_sets.size(); i++ )
    {
        auto& pool = _object_sets[i];
        if( pool == nullptr )
            continue;

        for( auto handle : *pool )
            _object_destructors[i](handle, pool->get(handle));
    }

    _object_sets.clear();
    _object_mutexs.clear();
    _object_destructors.clear();
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

    _frame_drawcall = 0;
    _frame_began = true;
    _statecache->begin_frame();
    return true;
}

void Renderer::clear(ClearOption option, const math::Color& color, float depth, unsigned stencil)
{
    ASSERT(_frame_began, "\'clear\' could only be called in render phase.");

    _backend->clear(option, color, depth, stencil);
}

void Renderer::submit(RenderLayer layer, uint32_t depth, RenderDrawcall& drawcall)
{
    ASSERT(_frame_began, "\'submit\' could only be called in render phase.");
    ASSERT(drawcall.program.is_valid(), "program is required to make drawcall.");
    ASSERT(drawcall.uniform_buffer.is_valid(), "uniform_buffer is required to make drawcall.");
    ASSERT(drawcall.vertex_buffer.is_valid(), "vertex_buffer is required to make drawcall.");

    drawcall.sort = SortValue::encode(layer, 1, depth);

    {
        std::unique_lock<std::mutex> L(_mutex);
        _drawcalls.push_back(drawcall);
    }
}

bool Renderer::drawcall_compare(const RenderDrawcall& lhs, const RenderDrawcall& rhs)
{
    return lhs.sort < rhs.sort;
}

void Renderer::flush()
{
    ASSERT(_frame_began, "\'flush\' could only be called in render phase.");

    std::unique_lock<std::mutex> L(_mutex);
    std::sort(_drawcalls.begin(), _drawcalls.end(), drawcall_compare);

    for( auto& drawcall : _drawcalls )
    {
        auto program = static_cast<ProgramGL*>(get<Program>(drawcall.program));
        auto uniform = static_cast<UniformBufferGL*>(get<UniformBuffer>(drawcall.uniform_buffer));
        auto vb = static_cast<VertexBufferGL*>(get<VertexBuffer>(drawcall.vertex_buffer));
        auto ib = static_cast<IndexBufferGL*>(get<IndexBuffer>(drawcall.index_buffer));

        if( program == nullptr || uniform == nullptr || vb == nullptr )
            continue;

        _statecache->bind_program(drawcall.program);
        _statecache->bind_uniform_buffer(drawcall.program, drawcall.uniform_buffer);
        _statecache->bind_vertex_buffer(drawcall.program, drawcall.vertex_buffer);

        auto& state = drawcall.state;
        _backend->set_scissor_test(state.scissor.enable, state.scissor.area);
        _backend->set_front_face(state.cull.winding);
        _backend->set_cull_face(state.cull.enable, state.cull.face);
        _backend->set_depth_test(state.depth.enable, state.depth.compare);
        _backend->set_depth_write(
            state.depth_write.enable,
            state.depth_write.bias_slope_scaled,
            state.depth_write.bias_constant);
        _backend->set_color_blend(
            state.blend.enable,
            state.blend.equation,
            state.blend.source_factor,
            state.blend.destination_factor);
        _backend->set_color_write(state.color_write);
        _backend->set_stencil_test(
            state.stencil.enable,
            state.stencil.compare,
            state.stencil.reference,
            state.stencil.mask);
        _backend->set_stencil_write(
            state.stencil_write.sfail,
            state.stencil_write.dpfail,
            state.stencil_write.dppass,
            state.stencil_write.mask);

        if( ib != nullptr )
        {
            ib->bind();
            _backend->draw_index(drawcall.primitive, ib->get_format(), drawcall.first, drawcall.count);
        }
        else
        {
            _backend->draw(drawcall.primitive, drawcall.first, drawcall.count);
        }
    }

    _frame_drawcall += _drawcalls.size();
    _drawcalls.clear();
}

unsigned Renderer::end_frame()
{
    ASSERT(_frame_began, "\'end_frame\' could only be called during rendering frame.");

    flush();
    _backend->end_frame();
    _frame_began = false;
    return _frame_drawcall;
}

NS_LEMON_GRAPHICS_END