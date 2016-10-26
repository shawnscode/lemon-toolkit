// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#include <graphics/private/texture.hpp>
#include <graphics/private/backend.hpp>

NS_LEMON_GRAPHICS_BEGIN

static GLenum GL_WRAP_MODE[] =
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

static GLenum GL_TEXTURE_FORMAT[] =
{
    GL_ALPHA,
    GL_RGB,
    GL_RGBA,
    GL_LUMINANCE,
    GL_LUMINANCE_ALPHA
};

static GLenum GL_TEXTURE_PIXEL_FORMAT[] =
{
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_SHORT_5_6_5,
    GL_UNSIGNED_SHORT_4_4_4_4,
    GL_UNSIGNED_SHORT_5_5_5_1
};

TextureGL::TextureGL(Handle handle) : Texture(handle)
{
    _override_parameters = true;
    _filter = TextureFilterMode::LINEAR;
    _address[0] = _address[1] = _address[2] = TextureAddressMode::WRAP;
    _mipmap = false;
    _width = _height = 0;
    _format = TextureFormat::RGBA;
    _pixel_format = TexturePixelFormat::USHORT4444;
}

bool TextureGL::initialize(
    const void* pixels,
    TextureFormat format,
    TexturePixelFormat pixel_format,
    unsigned width,
    unsigned height,
    MemoryUsage usage)
{
    if( !pixels || width == 0 || height == 0 )
    {
        LOGW("failed to set data of texture with null image.");
        return false;
    }

    dispose();

    glGenTextures(1, &_object);
    if( _object == 0 )
    {
        LOGW("failed to create texture object.");
        return false;
    }

    _pixel_format = pixel_format;
    _format = format;
    _width  = width;
    _height = height;
    _usage = usage;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _object);

    update_parameters(true);
    glTexImage2D(
        /*target*/ GL_TEXTURE_2D,
        /*level*/ 0,
        /*internalFormat*/ GL_TEXTURE_FORMAT[value(_format)],
        /*width*/ _width,
        /*height*/ _height,
        /*border*/ 0,
        /*format*/ GL_TEXTURE_FORMAT[value(_format)],
        /*type*/ GL_TEXTURE_PIXEL_FORMAT[value(_pixel_format)],
        /*data*/ pixels);

    if( _mipmap )
        glGenerateMipmap(GL_TEXTURE_2D);

    CHECK_GL_ERROR();
    return true;
}

void TextureGL::dispose()
{
    if( _object != 0 )
    {
        glDeleteBuffers(1, &_object);
        _object = 0;
    }
}

void TextureGL::set_mipmap(bool mipmap)
{
    if( _object != 0 && _mipmap != mipmap && mipmap )
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _object);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    _mipmap = mipmap;
    _override_parameters = true;
}

void TextureGL::set_filter_mode(TextureFilterMode mode)
{
    _filter = mode;
    _override_parameters = true;
}

void TextureGL::set_address_mode(TextureCoordinate coord, TextureAddressMode mode)
{
    _address[value(coord)] = mode;
    _override_parameters = true;
}

void TextureGL::update_parameters(bool force)
{
    if( !force && !_override_parameters )
        return;

    if( !_object )
        return;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_WRAP_MODE[value(_address[0])]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_WRAP_MODE[value(_address[1])]);
#ifndef GL_ES_VERSION_2_0
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_WRAP_MODE[value(_address[2])]);
#endif

    switch(_filter)
    {
        case TextureFilterMode::NEAREST:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;

        case TextureFilterMode::LINEAR:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _mipmap ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;

        case TextureFilterMode::TRILINEAR:
        case TextureFilterMode::ANISOTROPIC:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        default:
            break;
    }

    _override_parameters = false;
}

NS_LEMON_GRAPHICS_END
