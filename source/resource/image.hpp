// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <resource/resource.hpp>
#include <graphics/graphics.hpp>
#include <math/color.hpp>

NS_LEMON_RESOURCE_BEGIN

enum class ImageElementFormat : uint8_t
{
    UBYTE = 0,
    USHORT_565,
    USHORT_4444,
    USHORT_5551
};

struct Image : public Resource
{
    using ptr = std::shared_ptr<Image>;
    using weak_ptr = std::weak_ptr<Image>;

    virtual ~Image();

    bool read(std::istream&) override;
    bool save(std::ostream&) override;
    size_t get_memory_usage() const override;

    // set size and number of color components, old pixels will be discarded.
    bool initialize(unsigned, unsigned, unsigned, ImageElementFormat element = ImageElementFormat::UBYTE);
    // set new image data
    void set_data(const uint8_t* data);
    void set_data_raw(const uint8_t* data, unsigned offset, unsigned size);
    // clear the image with a color
    void clear(const math::Color&);
    // set a 2d pixel
    void set_pixel(unsigned, unsigned, const math::Color&);
    // return a 2D pixel color
    math::Color get_pixel(unsigned, unsigned) const;
    // return a linearly sampled 2d pixel color, x and y have the range 0-1
    math::Color get_pixel_linear(float, float) const;
    // return the raw memory pointer of image
    const void* get_data() const { return _data.get(); }
    // return the width/height/components of image
    unsigned get_width() const { return _width; }
    unsigned get_height() const { return _height; }
    unsigned get_components() const { return _components; }
    // return the internal format of image
    ImageElementFormat get_element_format() const { return _element_format; }

    // create a texture object if no exists and update data to buffer
    bool update_texture(graphics::MemoryUsage);
    // returns graphics object of texture
    graphics::Texture* get_texture() const;
    // returns handle of texture
    Handle get_texture_handle() const;

protected:
    // texture
    graphics::Texture* _texture = nullptr;
    // with/height
    unsigned _width = 0, _height = 0, _components = 1;
    // pixel data
    std::unique_ptr<uint8_t[]> _data;
    ImageElementFormat _element_format = ImageElementFormat::UBYTE;
};

INLINE graphics::Texture* Image::get_texture() const
{
    return _texture;
}

INLINE Handle Image::get_texture_handle() const
{
    return _texture == nullptr ? Handle() : _texture->handle;
}

NS_LEMON_RESOURCE_END