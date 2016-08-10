// @date 2016/08/03
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/device.hpp>
#include <resource/image.hpp>

NS_FLOW2D_GFX_BEGIN

enum class TextureUsage : uint8_t
{
    STATIC = 0,
    DYNAMIC,
    RENDERTARGET,
    DEPTHSTENCIL
};

struct Texture : public GPUObject
{
    Texture(Device& device) : GPUObject(device) {}
    virtual ~Texture() { release(); }

    void receive(const EvtDeviceRestore&) override { restore(); }
    void receive(const EvtDeviceLost&) override { release(); }

    bool restore(Image::ptr);
    bool restore();
    void release();

    // bind this vertex buffer to graphic device
    void bind_to_device();
    // enable shadowing data in cpu memory
    void set_shadowed(bool);

protected:
    bool            _shadowed;
    Image::ptr      _image;
    TextureUsage    _usage;
};

NS_FLOW2D_GFX_END
