// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/texture.hpp>
#include <graphics/opengl.hpp>

NS_FLOW2D_GFX_BEGIN

static GLenum GL_WRAP_MODES[] =
{
    GL_REPEAT,
    GL_MIRRORED_REPEAT,
    GL_CLAMP_TO_EDGE,
#ifndef GL_ES_VERSION_2_0
    GL_CLAMP,
#else
    GL_CLAMP_TO_EDGE
#endif
};

static GLenum GL_TEXTURE_FORMATS[] =
{
    GL_ALPHA,
    GL_RGB,
    GL_RGBA,
    GL_LUMINANCE,
    GL_LUMINANCE_ALPHA
};

static GLenum GL_IMAGE_ELEMENT_FORMATS[] =
{
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_SHORT_5_6_5,
    GL_UNSIGNED_SHORT_4_4_4_4,
    GL_UNSIGNED_SHORT_5_5_5_1
};

Texture::Texture(Device& device, unsigned target)
: GraphicsObject(device), _target(target)
{
    _parameter_dirty = true;
    _filter = TextureFilterMode::LINEAR;
    _address[0] = _address[1] = _address[2] = TextureAddressMode::WRAP;
    _mipmap = false;
}

bool Texture::restore(TextureFormat format)
{
    if( _device.is_device_lost() )
    {
        LOGW("trying to restore texture while device is lost.");
        return false;
    }

    release();

    glGenTextures(1, &_object);
    if( !_object )
    {
        LOGW("failed to create texture object.");
        return false;
    }

    _format = format;
    if( _image != nullptr && !set_data(_image) )
        return false;

    return true;
}

void Texture::release()
{
    if( _object != 0 && !_device.is_device_lost() )
        glDeleteBuffers(1, &_object);

    _object = 0;
}

void Texture::bind_to_device(unsigned unit)
{
    if( !_object || _device.is_device_lost() )
    {
        LOGW("failed to bind texture while device is lost.");
        return;
    }

    _device.set_texture(unit, _target, _object);
}

void Texture::set_shadowed(bool shadowed)
{
    if( _shadowed != shadowed )
    {
        if( !shadowed )
            _image.reset();
        _shadowed = shadowed;
    }
}

void Texture::set_mipmap(bool mipmap)
{
    _mipmap = mipmap;
    if( _object && !_device.is_device_lost() && _image )
    {
        _device.set_texture(0, _target, _object);
        if( mipmap ) glGenerateMipmap(_target);
    }
}

void Texture::set_filter_mode(TextureFilterMode mode)
{
    _filter = mode;
    _parameter_dirty = true;
}

void Texture::set_address_mode(TextureCoordinate coord, TextureAddressMode mode)
{
    _address[to_value(coord)] = mode;
    _parameter_dirty = true;
}

void Texture::update_parameters(bool force)
{
    if( !force && !_parameter_dirty )
        return;

    if( !_object || _device.is_device_lost() )
        return;

    glTexParameteri(_target, GL_TEXTURE_WRAP_S, GL_WRAP_MODES[to_value(_address[0])]);
    glTexParameteri(_target, GL_TEXTURE_WRAP_T, GL_WRAP_MODES[to_value(_address[1])]);
#ifndef GL_ES_VERSION_2_0
    glTexParameteri(_target, GL_TEXTURE_WRAP_R, GL_WRAP_MODES[to_value(_address[2])]);
#endif

    switch(_filter)
    {
        case TextureFilterMode::NEAREST:
            glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;

        case TextureFilterMode::LINEAR:
            glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, _mipmap ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR);
            glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;

        case TextureFilterMode::TRILINEAR:
        case TextureFilterMode::ANISOTROPIC:
            glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, _mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        default:
            break;
    }

    _parameter_dirty = false;
}

Texture2D::Texture2D(Device& device)
: Texture(device, GL_TEXTURE_2D)
{}

bool Texture2D::set_data(res::Image::ptr image)
{
    if( !image )
    {
        LOGW("failed to set data of texture with null image.");
        return false;
    }

    if( _shadowed && _image != image )
        _image = image;

    if( _object && !_device.is_device_lost() )
    {
        _device.set_texture(0, _target, _object);
        update_parameters(true);
        glTexImage2D(
            _target,
            0,
            GL_TEXTURE_FORMATS[to_value(_format)],
            image->get_width(),
            image->get_height(),
            0,
            GL_TEXTURE_FORMATS[to_value(_format)],
            GL_IMAGE_ELEMENT_FORMATS[to_value(image->get_element_format())],
            image->get_data());

        if( _mipmap ) glGenerateMipmap(_target);
    }

    CHECK_GL_ERROR();
    return true;
}

NS_FLOW2D_GFX_END