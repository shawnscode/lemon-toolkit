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

enum class PixelFormat : uint8_t
{
    UBYTE = 0,
    USHORT565,
    USHORT4444,
    USHORT5551
};

struct Texture : public GraphicsObject
{
    using ptr = std::shared_ptr<Texture>;
    using weak_ptr = std::weak_ptr<Texture>;

    Texture(Device& device, unsigned target);
    ~Texture() { release(); }

    bool restore(const void*, TextureFormat, PixelFormat, unsigned, unsigned, unsigned);

    // set data from an image and restore graphics state, return true if success
    bool restore() override;
    // release graphics resource and state
    void release() override;

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

    unsigned get_target() const { return _target; }

protected:
    virtual bool set_texture_data(const void*) = 0;

    const unsigned  _target;
    bool            _shadowed;
    bool            _mipmap;

    // settings of texture
    bool                _parameter_dirty;
    TextureUsage        _usage;
    TextureFilterMode   _filter;
    TextureAddressMode  _address[3];

    //
    std::unique_ptr<uint8_t[]> _shadowed_pixels;
    unsigned _width, _height, _depth;
    TextureFormat _format;
    PixelFormat _pixel_format;
};

struct Texture2D : public Texture
{
    Texture2D(Device& device);

    bool restore(TextureFormat, res::Image::ptr);
    bool set_texture_data(const void*) override;
};

NS_FLOW2D_GFX_END
