// @date 2016/10/14
// @author Mao Jingkai(oammix@gmail.com)

#include <resource/primitive.hpp>
#include <graphics/renderer.hpp>

NS_LEMON_RESOURCE_BEGIN

using namespace graphics;

float s_cube_vertices[] =
{
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

Primitive::ptr Primitive::cube()
{
    static const fs::Path s_path = {"_buildin_/primitive/cube"};
    if( auto cache = core::get_subsystem<ResourceCache>() )
    {
        auto layout = VertexLayout::make(
            VertexAttribute(VertexAttribute::POSITION, VertexElementFormat::FLOAT, 3),
            VertexAttribute(VertexAttribute::NORMAL, VertexElementFormat::FLOAT, 3)
        );
        return cache->create<Primitive>(s_path, s_cube_vertices, layout, 36);
    }
    return nullptr;
}

Primitive::~Primitive()
{
    graphics::resource::free(_vertex_buffer);
    graphics::resource::free(_index_buffer);
}

bool Primitive::update_video_object()
{
    if( (_vertex_buffer = graphics::resource::create<VertexBuffer>(
        _vertices.get(), _vertex_size, _layout, _usage)) )
    {
        if( _indices != nullptr )
        {
            if( (_index_buffer = graphics::resource::create<IndexBuffer>(
                _indices.get(), _index_size, _index_format, _usage)) )
            {
                return true;
            }

            graphics::resource::free<VertexBuffer>(_vertex_buffer);
            _vertex_buffer = nullptr;
        }
        return true;
    }
    return false;
}

bool Primitive::initialize(const void* data, const graphics::VertexLayout& layout, size_t size)
{
    if( data == nullptr )
    {
        LOGW("failed to create primitive with empty data.");
        return false;
    }

    _vertices.reset( new (std::nothrow) uint8_t[layout.get_stride()*size] );
    if( _vertices == nullptr )
    {
        LOGW("failed to allocate memory(%d bytes) for primitive.", layout.get_stride()*size);
        return false;
    }

    memcpy(_vertices.get(), data, layout.get_stride()*size);
    _layout = layout;
    _vertex_size = size;
    _indices.reset();
    _index_size = 0;
    return true;
}

unsigned INDEX_ELEMENT_SIZES[] =
{
    1,
    2
};

bool Primitive::initialize(
    const void* vdata,
    const graphics::VertexLayout& layout,
    size_t vsize,
    const void* idata,
    graphics::IndexElementFormat format,
    size_t isize)
{
    if( vdata == nullptr || idata == nullptr )
    {
        LOGW("failed to create primitive with empty data.");
        return false;
    }

    size_t vdata_size = layout.get_stride()*vsize;
    _vertices.reset( new (std::nothrow) uint8_t[vdata_size] );
    if( _vertices == nullptr )
    {
        LOGW("failed to allocate memory(%d bytes) for primitive.", vdata_size);
        return false;
    }

    size_t idata_size = INDEX_ELEMENT_SIZES[value(format)]*isize;
    _indices.reset( new (std::nothrow) uint8_t[idata_size] );
    if( _indices == nullptr )
    {
        _vertices.reset();
        LOGW("failed to allocate memory(%d bytes) for primitive.", idata_size);
        return false;
    }

    memcpy(_vertices.get(), vdata, vdata_size);
    memcpy(_indices.get(), idata, idata_size);

    _layout = layout;
    _vertex_size = vsize;
    _index_size = isize;
    _index_format = format;
    return true;
}

size_t Primitive::get_memory_usage() const
{
    return
        sizeof(Primitive) +
        _layout.get_stride() * _vertex_size +
        INDEX_ELEMENT_SIZES[value(_index_format)] *_index_size;
}

size_t Primitive::get_video_memory_usage() const
{
    return 0;
}

NS_LEMON_RESOURCE_END
