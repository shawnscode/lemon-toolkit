// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/vertex_attributes.hpp>
#include <codebase/handle.hpp>

NS_LEMON_GRAPHICS_BEGIN

// the expected usage pattern of the data store
enum class BufferUsage : uint8_t
{
    // the data store contents will be modified once and used at most a few times
    STREAM_DRAW = 0,
    // the data store contents will be modified once and used many times
    STATIC_DRAW,
    // the data store contents will be modified repeatedly and used many times
    DYNAMIC_DRAW
};

enum class IndexElementFormat : uint8_t
{
    UBYTE = 0,
    USHORT
};

enum class RenderLayer : uint16_t
{
    BACKGROUND = 1000,
    GEOMETRY = 2000,
    ALPHATEST = 2500,
    TRANSPARENCY = 3000,
    OVERLAY = 4000
};

struct FrontendContext;
struct Frontend : public core::Subsystem
{
    SUBSYSTEM("lemon::graphics::Frontend")

    bool initialize() override;

    // create vertex buffer
    Handle create_vertex_buffer(const void*, size_t, const VertexAttributeLayout&, BufferUsage);
    // set all data in the vertex buffer, returns true if update successful
    bool update_vertex_buffer(Handle, const void*);
    // set a data range in the buffer, optionally discard data outside the range
    bool update_vertex_buffer(Handle, const void*, unsigned, unsigned, bool discard = false);
    // free vertex buffer handle
    void free_vertex_buffer(Handle);

    // create index buffer
    Handle create_index_buffer(const void*, size_t, IndexElementFormat, BufferUsage);
    // set all data in the index buffer, returns true if update successful
    bool update_index_buffer(Handle, const void*);
    // set a data range in the buffer, optionally discard data outside the range
    bool update_index_buffer(Handle, const void*, unsigned, unsigned, bool discard = false);
    // free index buffer handle
    void free_index_buffer(Handle);

    // create program
    // Handle create_program(const char*, const char*);
    // void free_program(Handle);

    // create uniform buffer associated with program
    // Handle create_uniform_buffer(Handle);
    // update uniform vector value
    // void update_uniform1f(Handle, const char*, const math::Vector<1, float>&);
    // void update_uniform2f(Handle, const char*, const math::Vector<2, float>&);
    // void update_uniform3f(Handle, const char*, const math::Vector<3, float>&);
    // void update_uniform4f(Handle, const char*, const math::Vector<4, float>&);
    // // update uniform matrix value
    // void update_uniform2fm(Handle, const char*, const math::Matrix<2, 2, float>&);
    // void update_uniform3fm(Handle, const char*, const math::Matrix<3, 3, float>&);
    // void update_uniform4fm(Handle, const char*, const math::Matrix<4, 4, float>&);
    // // update uniform texture value
    // // void update_uniform_texture(const char*, Handle);
    // void free_uniform_buffer(Handle);

    // // create texture

    bool begin_frame();
    void submit(RenderLayer, RenderState, Handle program, Handle uniform, Handle vb, Handle ib, uint32_t depth);
    void flush();
    void end_frame();

protected:
    std::unique_ptr<FrontendContext> _context;
};

NS_LEMON_GRAPHICS_END
