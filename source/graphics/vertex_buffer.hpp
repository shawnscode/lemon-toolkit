// @date 2016/07/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/device.hpp>

NS_FLOW2D_GFX_BEGIN

struct VertexBuffer : public GPUObject
{
    VertexBuffer(Device& device, unsigned count, unsigned size, bool dynamic = false)
    : GPUObject(device), _count(count), _size(size), _dynamic(dynamic)
    {}

    bool initialize() override;
    void dispose() override;
    void on_device_restore() override;

    // enable shadowing in cpu memory, and its forced on if the GraphicsEngine does not exist
    void set_shadowed(bool);
    // set all data in the buffer
    bool set_data(const void*);
    // set a data range in the buffer. optionally discard data outside the range
    bool set_data_range(const void*, unsigned, unsigned, bool discard = false);

    // lock the buffer for write-only editing. return data pointer if successful. optionally discard data outside the range
    void* lock(unsigned, unsigned, bool discard = false);
    // unlock the buffer and apply changes to the graphic buffer
    void unlock();

    // return number of vertices
    unsigned get_vertex_count() const { _count; }
    // return vertex size in bytes
    unsigned get_vertex_size() const { _size; }

protected:
    bool        _dynamic;
    unsigned    _count;
    unsigned    _size;

    // shadow data stored in system memory
    std::unique_ptr<uint8_t[]> _shadowed_data;
    // do we need to sync shadow data to gpu
    bool _data_pending = false;
};

NS_FLOW2D_GFX_END