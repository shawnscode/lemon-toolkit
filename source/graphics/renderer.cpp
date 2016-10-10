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

    _frame_began = true;
    return true;
}

void Renderer::clear(ClearOption option, const math::Color& color, float depth, unsigned stencil)
{
    ASSERT(_frame_began, "\'clear\' could only be called in render phase.");

    _backend->clear(option, color, depth, stencil);
}

void Renderer::submit(
    RenderLayer layer, RenderState state,
    Program::ptr program, VertexBuffer::ptr vb, IndexBuffer::ptr ib,
    uint32_t depth, uint32_t start, uint32_t num)
{
    ASSERT(_frame_began, "\'submit\' could only be called in render phase.");

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

void Renderer::submit(RenderLayer layer, uint32_t depth, RenderDrawcall& drawcall)
{
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
        _backend->set_front_face(state.cull.winding);
        _backend->set_cull_face(state.cull.enable, state.cull.face);
        // _backend->set_
        // ...

        _backend->draw(state.primitive, drawcall.first, drawcall.count);
    }

    printf("draw %ld", _drawcalls.size());
    _drawcalls.clear();
}

void Renderer::end_frame()
{
    ASSERT(_frame_began, "\'end_frame\' could only be called during rendering frame.");

    flush();
    _backend->end_frame();
    _frame_began = false;
}

NS_LEMON_GRAPHICS_END