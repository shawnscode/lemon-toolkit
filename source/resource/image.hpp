// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <resource/resource.hpp>

NS_FLOW2D_RES_BEGIN

enum class ImageFormat : uint8_t
{
    ALPHA = 0,
    RGB,
    RGBA,
    LUMINANCE,
    LUMINANCE_ALPHA
};

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
    bool set_size(unsigned, unsigned, unsigned);
    // set new image data
    void set_data(const uint8_t* data);

    const void* get_data() const { return _data.get(); }
    unsigned get_width() const { return _width; }
    unsigned get_height() const { return _height; }
    ImageFormat get_format() const { return _format; }
    ImageElementFormat get_element_format() const { return _element_format; }

protected:
    // with/height
    unsigned _width = 0, _height = 0;
    // number of color components
    unsigned _components = 0;
    // pixel data
    std::unique_ptr<uint8_t[]> _data;
    ImageFormat _format = ImageFormat::RGB;
    ImageElementFormat _element_format = ImageElementFormat::UBYTE;
};

NS_FLOW2D_RES_END