// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/renderer.hpp>
#include <graphics/private/backend.hpp>
#include <graphics/private/vertex_array_cache.hpp>
#include <graphics/private/program.hpp>
#include <graphics/private/vertex_buffer.hpp>
#include <graphics/private/index_buffer.hpp>
#include <graphics/private/texture.hpp>

NS_LEMON_GRAPHICS_BEGIN

bool Renderer::initialize()
{
    _backend = new (std::nothrow) RendererBackend();
    if ( _backend == nullptr )
        return false;

    _vaocache = new (std::nothrow) VertexArrayObjectCache();
    if( _vaocache == nullptr )
    {
        delete _backend;
        _backend = nullptr;
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

    if( _vaocache != nullptr )
    {
        delete _vaocache;
        _vaocache = nullptr;
    }
}

bool Renderer::restore(SDL_Window* window)
{
    return _backend->initialize(window);
}

void Renderer::release()
{
    _backend->dispose();
}

Program::ptr Renderer::create_program(const char* vs, const char* ps)
{
    auto object = new (std::nothrow) ProgramGL(*this);
    if( object && object->initialize(vs, ps) )
        return Program::ptr(object);

    if( object ) delete object;
    return nullptr;
}

Texture::ptr Renderer::create_texture(
    const void* pixels,
    TextureFormat format,
    TexturePixelFormat pixel_format,
    unsigned width,
    unsigned height,
    MemoryUsage usage)
{
    auto object = new (std::nothrow) TextureGL(*this);
    if( object && object->initialize(pixels, format, pixel_format, width, height, usage) )
        return Texture::ptr(object);

    if( object ) delete object;
    return nullptr;
}

VertexBuffer::ptr Renderer::create_vertex_buffer(
    const void* data,
    unsigned size,
    const VertexLayout& layer,
    MemoryUsage usage)
{
    auto object = new (std::nothrow) VertexBufferGL(*this);
    if( object && object->initialize(data, size, layer, usage) )
        return VertexBuffer::ptr(object);

    if( object ) delete object;
    return nullptr;
}

IndexBuffer::ptr Renderer::create_index_buffer(
    const void* data,
    unsigned size,
    IndexElementFormat format,
    MemoryUsage usage)
{
    auto object = new (std::nothrow) IndexBufferGL(*this);
    if( object && object->initialize(data, size, format, usage) )
        return IndexBuffer::ptr(object);

    if( object ) delete object;
    return nullptr;
}

bool Renderer::begin_frame()
{
    if( !_backend->begin_frame() )
        return false;

    _frame_drawcall = 0;
    _frame_began = true;
    _vaocache->unbind();
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
    ASSERT(drawcall.program != nullptr, "program is required to make drawcall.");
    ASSERT(drawcall.vertex_buffer != nullptr, "vertex_buffer is required to make drawcall.");

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
        std::static_pointer_cast<ProgramGL>(drawcall.program)->bind();
        _vaocache->bind(drawcall.program, drawcall.vertex_buffer);

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

        if( drawcall.index_buffer != nullptr )
        {
            auto index_buffer = std::static_pointer_cast<IndexBufferGL>(drawcall.index_buffer);
            index_buffer->bind();
            _backend->draw_index(drawcall.primitive, index_buffer->get_format(), drawcall.first, drawcall.count);
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