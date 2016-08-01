// @date 2016/07/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/device.hpp>

NS_FLOW2D_GFX_BEGIN

struct VertexElement
{
    VertexElement(
        VertexElementType t = VertexElementType::VECTOR3,
        VertexElementSemantic s = VertexElementSemantic::POSITION,
        uint8_t i = 0)
    : type(t), semantic(s), semantic_index(i) {}

    // test  for equality with other vertex element
    bool operator == (const VertexElement& rhs) const;

    VertexElementType       type;
    VertexElementSemantic   semantic;
    // semantic index of elements, for example multi-texcoords
    uint8_t                 semantic_index;
    // offset of element from vertex state. filled by VertexBuffer once the vertex declaration
    unsigned                offset;
};

INLINE bool VertexElement::operator == (const VertexElement& rhs) const
{
    return type == rhs.type && semantic == rhs.semantic && semantic_index == rhs.semantic_index;
}

struct VertexBuffer : public GPUObject
{
    // enable shadowing in cpu memory, and its forced on if the GraphicsEngine does not exist
    void set_shadowed(bool);
    // set size, vertex elements and dynamic mode, previous data will be lost
    bool set_size(unsigned, const vector<VertexElement>&, bool dynamic = false);
    // set all data in the buffer
    bool set_data(const void*);
    // set a data range in the buffer. optionally discard data outside the range
    bool set_data_range(const void*, unsigned, unsigned, bool discard = false);
    // lock the buffer for write-only editing. return data pointer if successful. optionally discard data outside the range
    void* lock(unsigned, unsigned, bool discard = false);
    // unlock the buffer and apply changes to the graphic buffer
    void unlock();
    // return whether is currently locked
    bool is_locked() const;
    // return number of vertices
    unsigned get_vertex_count() const;
    // return vertex size in bytes
    unsigned get_vertex_size() const;

protected:
    using stream = std::unique_ptr<uint8_t[]>;

    bool        _shadowed;
    bool        _dynamic;
    unsigned    _vertex_count, _vertex_size;

    // shadow data stored in memory
    stream      _shadow_data;
    // scratch buffer for fallback locking
    void*       _scratch_data;
};


NS_FLOW2D_GFX_END