// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/device.hpp>
#include <resource/image.hpp>

NS_FLOW2D_GFX_BEGIN

enum class TextureUsage : uint8_t
{
    STATIC = 0,
    DYNAMIC
};

enum class TextureFilterMode : uint8_t
{
    NEAREST = 0,
    LINEAR,
    TRILINEAR,
    ANISOTROPIC
};

enum class TextureAddressMode : uint8_t
{
    WRAP = 0,
    MIRROR,
    CLAMP,
    BORDER
};

enum class TextureCoordinate : uint8_t
{
    U = 0,
    V,
    W
};

enum class TextureFormat : uint8_t
{
    ALPHA = 0,
    RGB,
    RGBA,
    LUMINANCE,
    LUMINANCE_ALPHA
};

struct Texture : public GPUObject
{
    Texture(Device& device, unsigned target);

    bool restore(TextureFormat);
    void release();

    // bind texture to specified unit
    void bind_to_device(unsigned);
    // keep the shared-reference of image
    void set_shadowed(bool);
    // set number of requested mip levels
    void set_mipmap(bool);
    // set filtering mode
    void set_filter_mode(TextureFilterMode);
    // set adddress mode
    void set_address_mode(TextureCoordinate, TextureAddressMode);
    // update the changed parameters to device
    void update_parameters(bool force = false);
    // return memory representation if we shadowed this texture
    res::Image::ptr get_image() { return _image; }
    // set data from an image, return true if success
    virtual bool set_data(res::Image::ptr) = 0;

protected:
    const unsigned  _target;
    bool            _shadowed;
    bool            _mipmap;
    TextureFormat   _format;
    res::Image::ptr _image;

    // settings of texture
    bool                _parameter_dirty;
    TextureUsage        _usage;
    TextureFilterMode   _filter;
    TextureAddressMode  _address[3];
};

struct Texture2D : public Texture
{
    Texture2D(Device& device);
    bool set_data(res::Image::ptr) override;
};

NS_FLOW2D_GFX_END
