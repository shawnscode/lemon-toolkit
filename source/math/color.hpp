// @date 2016/06/01
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forward.hpp>
#include <initializer_list>

NS_FLOW2D_BEGIN

struct Color
{
    Color() : r(1.f), g(1.f), b(1.f), a(1.f) {};
    Color(const Color&, float);
    Color(std::initializer_list<float>);

    Color(const Color&) = default;
    Color& operator = (const Color&) = default;

    // test for equality and equality with another color without epsilon.
    bool operator == (const Color&) const;
    bool operator != (const Color&) const;
    Color operator + (const Color&) const;
    Color operator - (const Color&) const;
    Color operator * (const Color&) const;
    Color operator * (float) const;
    float operator [] (size_t) const;

    // return the 'grayscale' representation of RGB values
    float grayscale() const;
    // return color packed to a 32-bit integer
    uint32_t to_uint32() const;

    // clip to [0, 1.0] range
    Color& clip();
    // inverts the RGB channels and optionally the alpha channel as well
    Color& invert(bool alpha = false);

    float r, g, b, a;

    static const Color WHITE;
    static const Color GRAY;
    static const Color BLACK;
    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color CYAN;
    static const Color MAGENTA;
    static const Color YELLOW;
    static const Color TRANSPARENT;
};

// test for equality with epsilon
bool equals(const Color&, const Color&, float epsilon = std::numeric_limits<float>::epsilon());
// return linear interpolation o this color with another color
Color lerp(const Color&, const Color&, float t);

#include <math/color.inl>
NS_FLOW2D_END