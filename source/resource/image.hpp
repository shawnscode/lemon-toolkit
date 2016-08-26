// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <resource/resource.hpp>
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

    bool read(std::istream&) override;
    bool save(std::ostream&) override;

    // set size and number of color components, old pixels will be discarded.
    bool set_size(unsigned, unsigned, unsigned, ImageElementFormat element = ImageElementFormat::UBYTE);
    // set new image data
    void set_data(const uint8_t* data);
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
    // return the width/height of image
    unsigned get_width() const { return _width; }
    unsigned get_height() const { return _height; }
    // return the internal format of image
    ImageElementFormat get_element_format() const { return _element_format; }
    // return number of color components
    unsigned get_components() const { return _components; }

protected:
    // with/height
    unsigned _width = 0, _height = 0, _components = 1;
    // pixel data
    std::unique_ptr<uint8_t[]> _data;
    ImageElementFormat _element_format = ImageElementFormat::UBYTE;
};

NS_LEMON_RESOURCE_END