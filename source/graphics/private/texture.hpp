// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <graphics/graphics.hpp>
#include <graphics/private/opengl.hpp>
// #include <resource/image.hpp>

NS_LEMON_GRAPHICS_BEGIN

struct TextureGL : public Texture
{
    TextureGL(Renderer& renderer, Handle handle);
    virtual ~TextureGL() { dispose(); }

    // set data from an image and restore graphics state, return true if success
    bool initialize(const void*, TextureFormat, TexturePixelFormat, unsigned, unsigned, MemoryUsage) override;
    // release graphics resource and state
    void dispose() override;
    // set number of requested mip levels
    void set_mipmap(bool) override;
    // set filtering mode
    void set_filter_mode(TextureFilterMode) override;
    // set adddress mode
    void set_address_mode(TextureCoordinate, TextureAddressMode) override;
    // update the changed parameters to device
    void update_parameters(bool force = false) override;

    //
    GLuint get_handle() const { return _object; }

protected:
    GLuint _object;

    // settings of texture
    bool _override_parameters;
    bool _mipmap;
    MemoryUsage _usage;
    TextureFilterMode _filter;
    TextureAddressMode _address[3];

    unsigned _width;
    unsigned _height;
    TextureFormat _format;
    TexturePixelFormat _pixel_format;
};

NS_LEMON_GRAPHICS_END
