// @date 2016/10/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <graphics/defer/graphics.hpp>
#include <codebase/handle.hpp>

NS_LEMON_GRAPHICS_BEGIN

struct RenderDrawCall
{
    void set_program(const Program&);
    // void set_render_state(const RenderState&);
    void set_data_source(const VertexBuffer&, uint16_t, uint16_t);
    void set_data_source(const VertexBuffer&, const IndexBuffer&, uint16_t, uint16_t);
    // void set_shared_uniforms(const UniformBufferView&);
    // void set_uniforms(const UniformBufferView&);
    // 

protected:
    friend class RenderFrontend;

    // the unique handle of shader program we are applying in drawing
    Handle _program;
    // the unique handle of render state instance
    Handle _state;
    // the unique handle of vertices and indices buffer
    Handle _buffer_vertex;
    Handle _buffer_index;
    // offsets to identifies the uniforms buffer used by this draw call
    uint16_t _const_offset, _const_size;
    // its common to have some shared uniforms such like view and projection matrix
    uint16_t _shared_const_offset, _shared_const_size;
    // specifies the offset and count of indices or vertices to be drawed
    uint16_t _first, _num;
};

INLINE void RenderDrawCall::set_program(const Program& program)
{
    _program = program;
}

// INLINE void RenderDrawCall::set_render_state(const RenderState& state)
// {
//     _state = state
// }

INLINE void RenderDrawCall::set_data_source(
    const VertexBuffer& vb, const IndexBuffer& ib, uint16_t first, uint16_t num)
{
    _buffer_index = ib;
    _buffer_vertex = vb;
    _first = first;
    _num = num;
}

INLINE void RenderDrawCall::set_data_source(
    const VertexBuffer& vb, uint16_t first, uint16_t num)
{
    _buffer_index.invalidate();
    _buffer_vertex = vb;
    _first = first;
    _num = num;
}

// INLINE void RenderDrawCall::set_shared_uniforms(const UniformBufferView& ubv)
// {
//     _shared_const_offset = ubv.get_offset();
//     _shared_const_size = ubv.get_size();
// }

// INLINE void RenderDrawCall::set_uniforms(const UniformBufferView& ubv)
// {
//     _const_offset = ubv.get_offset();
//     _const_size = ubv.get_size();
// }

NS_LEMON_GRAPHICS_END
