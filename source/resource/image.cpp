// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#include <resource/image.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

NS_FLOW2D_RES_BEGIN

bool Image::read(std::istream& in)
{
    auto start_pos = in.tellg();
    in.seekg(0, std::ios::end);
    auto size = in.tellg() - start_pos;
    in.seekg(start_pos);

    std::unique_ptr<uint8_t[]> tmp(new uint8_t[size]);
    in.read((char*)tmp.get(), size);

    unsigned width, height, components;
    auto pixels = stbi_load_from_memory(
        tmp.get(), size, (int*)&width, (int*)&height, (int*)&components, 0);
    if( !pixels )
    {
        LOGW("failed to load image %s,\n\t%s", _name.c_str(), stbi_failure_reason());
        return false;
    }
    tmp.reset();

    if( set_size(width, height, components) )
    {
        stbi_image_free(pixels);
        return false;
    }

    set_data(pixels);
    stbi_image_free(pixels);
    return true;
}

bool Image::save(std::ostream& out)
{
    return false;
}

bool Image::set_size(unsigned width, unsigned height, unsigned components)
{
    if( width == _width && height == _width && components == _components )
        return true;

    if( width <= 0 || height <= 0 || components <= 0 || components > 4 )
    {
        LOGW("invalid image size/components when set_size.");
        return false;
    }

    _data.reset(new uint8_t[width*height*components]);
    if( !_data )
    {
        LOGW("failed to allocate memory for image data.");
        return false;
    }

    _width = width;
    _height = height;
    _components = components;
    _memusage = width * height * components;
    return true;
}

void Image::set_data(const uint8_t* data)
{
    if( !data )
        return;

    memcpy(_data.get(), data, _width*_height*_components);
}

NS_FLOW2D_RES_END