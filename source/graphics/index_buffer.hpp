// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/device.hpp>

NS_FLOW2D_GFX_BEGIN

enum class IndexElementFormat : uint8_t
{
    UBYTE = 0,
    USHORT
};

struct IndexBuffer : public GraphicsObject
{
    IndexBuffer(Device& device);
    virtual ~IndexBuffer() { release(); }

    bool restore(const void*, unsigned, IndexElementFormat, bool dynamic = false);
    bool restore() override;
    void release() override;

    // enable shadowing data in cpu memory
    void set_shadowed(bool);
    // set all data in the buffer
    bool update_data(const void*);
    // set a data range in the buffer, optionally discard data outside the range
    bool update_data_range(const void*, unsigned, unsigned, bool discard = false);

    unsigned get_index_count() const;
    unsigned get_stride() const;
    unsigned get_size() const;
    IndexElementFormat get_element_format() const;

protected:
    bool _dynamic;
    unsigned _index_count;
    IndexElementFormat _format;

    // shadow data stored in system memory
    bool _shadowed;
    std::unique_ptr<uint8_t[]> _shadowed_data;
};

NS_FLOW2D_GFX_END
