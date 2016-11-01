// @date 2016/10/26
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/graphics.hpp>
#include <graphics/backend/frame.hpp>
#include <graphics/backend/backend.hpp>

NS_LEMON_GRAPHICS_BEGIN

struct CreateVertexBuffer : public FrameTask
{
    Handle handle;
    void* data;
    size_t size;
    VertexLayout layout;
    BufferUsage usage;

    void dispatch(RenderBackend& backend) override
    {
        backend.create_vertex_buffer(handle, data, size, layout, usage);
    }
};

struct UpdateVertexBuffer : public FrameTask
{
    Handle handle;
    uint16_t start;
    void* data;
    size_t size;

    void dispatch(RenderBackend& backend) override
    {
        backend.update_vertex_buffer(handle, start, data, size);
    }
};

struct FreeVertexBuffer : public FrameTask
{
    Handle handle;

    void dispatch(RenderBackend& backend) override
    {
        backend.free_vertex_buffer(handle);
    }
};

struct CreateIndexBuffer : public FrameTask
{
    Handle handle;
    void* data;
    size_t size;
    IndexElementFormat format;
    BufferUsage usage;

    void dispatch(RenderBackend& backend) override
    {
        backend.create_index_buffer(handle, data, size, format, usage);
    }
};

struct UpdateIndexBuffer : public FrameTask
{
    Handle handle;
    uint16_t start;
    void* data;
    size_t size;

    virtual void dispatch(RenderBackend& backend) override
    {
        backend.update_index_buffer(handle, start, data, size);
    }
};

struct FreeIndexBuffer : public FrameTask
{
    Handle handle;

    virtual void dispatch(RenderBackend& backend) override
    {
        backend.free_index_buffer(handle);
    }
};

struct CreateProgram : public FrameTask
{
    Handle handle;
    char* vs;
    char* fs;

    void dispatch(RenderBackend& backend) override
    {
        backend.create_program(handle, vs, fs);
    }
};

struct CreateProgramUniform : public FrameTask
{
    Handle handle;
    char* name;

    void dispatch(RenderBackend& backend) override
    {
        backend.create_program_uniform(handle, name);
    }
};

struct CreateProgramAttribute : public FrameTask
{
    Handle handle;
    VertexAttribute::Enum attribute;
    char* name;

    void dispatch(RenderBackend& backend) override
    {
        backend.create_program_attribute(handle, attribute, name);
    }
};

struct FreeProgram : public FrameTask
{
    Handle handle;

    void dispatch(RenderBackend& backend) override
    {
        backend.free_program(handle);
    }
};

struct CreateTexture : public FrameTask
{
    Handle handle;
    void* data;
    TextureFormat format;
    TexturePixelFormat pixel_format;
    uint16_t width;
    uint16_t height;
    BufferUsage usage;

    void dispatch(RenderBackend& backend) override
    {
        backend.create_texture(
            handle, data, format, pixel_format, width, height, usage);
    }
};

struct UpdateTextureMipmap : public FrameTask
{
    Handle handle;
    bool mipmap;

    void dispatch(RenderBackend& backend) override
    {
        backend.update_texture_mipmap(handle, mipmap);
    }
};

struct UpdateTextureAddress : public FrameTask
{
    Handle handle;
    TextureCoordinate coordinate;
    TextureAddressMode wrap;

    void dispatch(RenderBackend& backend) override
    {
        backend.update_texture_address_mode(handle, coordinate, wrap);
    }
};

struct UpdateTextureFilter : public FrameTask
{
    Handle handle;
    TextureFilterMode filter;

    void dispatch(RenderBackend& backend) override
    {
        backend.update_texture_filter_mode(handle, filter);
    }
};

struct FreeTexture : public FrameTask
{
    Handle handle;

    void dispatch(RenderBackend& backend) override
    {
        backend.free_texture(handle);
    }
};

struct ClearView : public FrameTask
{
    ClearOption option;
    math::Color color;
    float depth;
    uint32_t stencil;

    void dispatch(RenderBackend& backend)
    {
        backend.clear(option, color, depth, stencil);
    }
};

NS_LEMON_GRAPHICS_END
