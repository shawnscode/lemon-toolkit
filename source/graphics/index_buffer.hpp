// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/device.hpp>

NS_FLOW2D_GFX_BEGIN

struct IndexBuffer : public GPUObject
{
    IndexBuffer(Device& device) : GPUObject(device) {}
    virtual ~IndexBuffer() { release(); }

    void receive(const EvtDeviceRestore&) override { restore(); }
    void receive(const EvtDeviceLost&) override { release(); }

    bool restore(unsigned count, unsigned size, bool dynamic);
    bool restore();
    void release();

    // bind this vertex buffer to graphic device
    void bind_to_device();
    // enable shadowing data in cpu memory
    void set_shadowed(bool);
    // set all data in the buffer
    bool set_data(const void*);
    // set a data range in the buffer, optionally discard data outside the range
    bool set_data_range(const void*, unsigned, unsigned, bool discard = false);

    // return number of vertices
    unsigned get_vertex_count() const { return _count; }
    // return vertex size in bytes
    unsigned get_vertex_size() const { return _size; }

protected:
    bool        _dynamic    = false;
    unsigned    _count      = 0;
    unsigned    _size       = 0;

    // shadow data stored in system memory
    std::unique_ptr<uint8_t[]> _shadowed_data;
    // do we need to sync shadow data to gpu
    bool _data_pending = false;
};

NS_FLOW2D_GFX_END
