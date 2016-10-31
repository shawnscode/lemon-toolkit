// @date 2016/10/10
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/graphics.hpp>

NS_LEMON_GRAPHICS_BEGIN

size_t FORMAT_COMPONENT_SIZE[] =
{
    1,
    3,
    4,
    1,
    2
};

size_t size_of_texture(
    TextureFormat format, TexturePixelFormat pixel_format,
    uint16_t width, uint16_t height)
{
    if( pixel_format == TexturePixelFormat::UBYTE )
    {
        return width * height * FORMAT_COMPONENT_SIZE[value(format)];
    }
    else
    {
        return width * height * 2;
    }
}

NS_LEMON_GRAPHICS_END
