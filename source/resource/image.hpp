// @date 2016/08/10
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <resource/resource.hpp>

NS_FLOW2D_RES_BEGIN

struct Image : public Resource
{
    virtual ~Image() {}

    bool read(std::istream&) override;
    bool save(std::ostream&) override;

    // set size and number of color components, old pixels will be discarded.
    bool set_size(unsigned, unsigned, unsigned);
    // set new image data
    void set_data(const uint8_t* data);

protected:
    // with/height
    unsigned _width = 0, _height = 0;
    // number of color components
    unsigned _components = 0;
    // pixel data
    std::unique_ptr<uint8_t[]> _data;
};

NS_FLOW2D_RES_END