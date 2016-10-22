// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#include <resource/image.hpp>
#include <graphics/renderer.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

NS_LEMON_RESOURCE_BEGIN

using namespace graphics;

TextureFormat FORMAT[] =
{
    TextureFormat::ALPHA,
    TextureFormat::ALPHA,
    TextureFormat::LUMINANCE_ALPHA,
    TextureFormat::RGB,
    TextureFormat::RGBA,
};

Image::~Image()
{
    graphics::Renderer::checked_free(_texture);
}

bool Image::read(std::istream& in)
{
    auto start_pos = in.tellg();
    in.seekg(0, std::ios::end);
    auto size = in.tellg() - start_pos;
    in.seekg(start_pos);

    std::unique_ptr<uint8_t[]> tmp(new uint8_t[size]);
    in.read((char*)tmp.get(), size);

    unsigned width, height, components;

    // make sure we have same texture coordinations with OpenGL
    stbi_set_flip_vertically_on_load(true);
    auto pixels = stbi_load_from_memory(
        tmp.get(), size, (int*)&width, (int*)&height, (int*)&components, 0);
    if( !pixels )
    {
        LOGW("failed to load image %s,\n\t%s", _name.c_str(), stbi_failure_reason());
        return false;
    }
    tmp.reset();

    if( !initialize(width, height, components) )
    {
        stbi_image_free(pixels);
        return false;
    }

    set_data(pixels);
    stbi_image_free(pixels);
    return update_texture(MemoryUsage::STATIC);
}

bool Image::save(std::ostream& out)
{
    if( !_data )
    {
        LOGW("can't save zero-sized image %s.", _name.c_str());
        return false;
    }

    int len;
    uint8_t* png = stbi_write_png_to_mem(_data.get(), 0, _width, _height, _components, &len);
    out.write((const char*)png, len);
    free(png);

    if( (out.rdstate() & std::ifstream::failbit ) != 0 )
    {
        LOGW("failed to write ot ostream when save image %s", _name.c_str());
        return false;
    }

    return true;
}

size_t Image::get_memory_usage() const
{
    return _width * _height * _components;
}

bool Image::update_texture(graphics::MemoryUsage usage)
{
    auto format = FORMAT[_components];
    auto element_format = static_cast<TexturePixelFormat>(_element_format);
    if( _texture == nullptr )
    {
        _texture = core::get_subsystem<graphics::Renderer>()->create<graphics::Texture>(
            _data.get(), format, element_format, _width, _height, usage);
        return _texture != nullptr;
    }
    else
    {
        return _texture->initialize(_data.get(), format, element_format, _width, _height, usage);
    }
}

bool Image::initialize(unsigned width, unsigned height, unsigned components, ImageElementFormat element)
{
    if( width == _width && height == _width && element == _element_format )
        return true;

    if( width <= 0 || height <= 0 || components <=0 || components > 4 )
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
    _element_format = element;
    return true;
}

void Image::set_data(const uint8_t* data)
{
    if( !data )
        return;

    memcpy(_data.get(), data, _width*_height*_components);
}

void Image::set_data_raw(const uint8_t* data, unsigned offset, unsigned size)
{
    memcpy(_data.get()+offset, data, size);
}

void Image::clear(const math::Color& color)
{
    if( !_data )
        return;

    for( unsigned y = 0; y < _height; y++ )
        for( unsigned x = 0; x < _width; x++ )
            set_pixel(x, y, color);
}

void Image::set_pixel(unsigned x, unsigned y, const math::Color& color)
{
    if( !_data || x >= _width || y >= _height )
        return;

    uint8_t* dest = _data.get() + (y*_width+x) * _components;
    uint32_t ic = color.to_uint32();
    switch(_components)
    {
        case 4: // RGBA
            dest[3] = (uint8_t)((ic & 0x000000FF) >> 0);
        case 3: // RGB
        {
            dest[0] = (uint8_t)((ic & 0xFF000000) >> 24);
            dest[1] = (uint8_t)((ic & 0x00FF0000) >> 16);
            dest[2] = (uint8_t)((ic & 0x0000FF00) >> 8);
            break;
        }
        case 2: // LUMINANCE_ALPHA
        {
            dest[0] = (uint8_t)math::clamp(255.f * color.grayscale(), 0.f, 255.f);
            dest[1] = (uint8_t)((ic & 0x000000FF) >> 0);
            break;
        }
        case 1: // ALPHA
        {
            dest[0] = (uint8_t)((ic & 0x000000FF) >> 0);
            break;
        }
        default: FATAL("invalid components");
    }
}

math::Color Image::get_pixel(unsigned x, unsigned y) const
{
    if( !_data )
        return 0xFF000000;

    x = x >= _width ? _width - 1 : x;
    y = y >= _height ? _height - 1 : y;
    uint8_t* src = _data.get() + (y*_width+x) * _components;

    math::Color ret = { 0.f, 0.f, 0.f, 1.f };
    switch(_components)
    {
        case 4: // RGBA
            ret.a = (float)src[3] / 255.0f;
        case 3: // RGB
        {
            ret.r = (float)src[0] / 255.0f;
            ret.g = (float)src[1] / 255.0f;
            ret.b = (float)src[2] / 255.0f;
            break;
        }
        case 2: // LUMINANCE_ALPHA
        {
            ret.r = ret.g = ret.b = (float)src[0] / 255.0f;
            ret.a = (float)src[1] / 255.f;
            break;
        }
        case 1:
        {
            ret.a = (float)src[0] / 255.f;
            break;
        }
        default: FATAL("invalid components");
    }
    return ret;
}

math::Color Image::get_pixel_linear(float x, float y) const
{
    x = math::clamp(x * _width - 0.5f, 0.f, (float)(_width-1));
    y = math::clamp(y * _height - 0.5f, 0.f, (float)(_height-1));

    int xi = (int)x;
    int yi = (int)y;

    float xd = x - std::floor(x);
    float yd = y - std::floor(y);

    auto top    = lerp(get_pixel(xi, yi+1), get_pixel(xi+1, yi+1), xd);
    auto bottom = lerp(get_pixel(xi, yi), get_pixel(xi+1, yi), xd);
    return lerp(top, bottom, yd);
}

NS_LEMON_RESOURCE_END